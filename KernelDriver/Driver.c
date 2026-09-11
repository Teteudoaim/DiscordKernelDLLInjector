#include "Driver.h"

#define DISCORD_PROCESS_NAME L"DiscordCanary.exe"

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, DispatchCreateClose)
#pragma alloc_text(PAGE, DispatchDeviceControl)

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;
    UNICODE_STRING DeviceName, SymbolicName;

    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("[DiscordInjector] Driver Entry Point\n");

    // Criar nome do dispositivo
    RtlInitUnicodeString(&DeviceName, L"\\Device\\DiscordInjectorDevice");
    RtlInitUnicodeString(&SymbolicName, L"\\DosDevices\\DiscordInjector");

    // Criar objeto dispositivo
    Status = IoCreateDevice(
        DriverObject,
        0,
        &DeviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &DeviceObject
    );

    if (!NT_SUCCESS(Status))
    {
        DbgPrint("[DiscordInjector] Falha ao criar dispositivo: 0x%X\n", Status);
        return Status;
    }

    // Criar link simbólico
    Status = IoCreateSymbolicLink(&SymbolicName, &DeviceName);
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("[DiscordInjector] Falha ao criar link simbólico: 0x%X\n", Status);
        IoDeleteDevice(DeviceObject);
        return Status;
    }

    // Registrar callbacks
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    // Definir flags do dispositivo
    DeviceObject->Flags |= DO_DIRECT_IO;
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    DbgPrint("[DiscordInjector] Driver inicializado com sucesso\n");
    return Status;
}

VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNICODE_STRING SymbolicName;

    PAGED_CODE();

    DbgPrint("[DiscordInjector] Descarregando driver\n");

    RtlInitUnicodeString(&SymbolicName, L"\\DosDevices\\DiscordInjector");
    IoDeleteSymbolicLink(&SymbolicName);

    if (DriverObject->DeviceObject)
    {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
}

NTSTATUS DispatchCreateClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    PIO_STACK_LOCATION IrpStack;
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID InputBuffer = NULL;
    PVOID OutputBuffer = NULL;
    ULONG InputBufferLength = 0;
    ULONG OutputBufferLength = 0;

    UNREFERENCED_PARAMETER(DeviceObject);

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_GET_DISCORD_PID:
        {
            if (OutputBufferLength < sizeof(DISCORD_PID_RESPONSE))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            PDISCORD_PID_RESPONSE Response = (PDISCORD_PID_RESPONSE)OutputBuffer;
            Status = FindDiscordCanaryPID(&Response->PID);
            Response->Found = NT_SUCCESS(Status);

            Irp->IoStatus.Information = sizeof(DISCORD_PID_RESPONSE);
            break;
        }

        case IOCTL_INJECT_DLL:
        {
            if (InputBufferLength < sizeof(INJECT_DLL_REQUEST))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            PINJECT_DLL_REQUEST Request = (PINJECT_DLL_REQUEST)InputBuffer;
            UNICODE_STRING DLLPath;

            RtlInitUnicodeString(&DLLPath, Request->DLLPath);
            Status = InjectDLLIntoProcess(Request->DiscordPID, &DLLPath);

            Irp->IoStatus.Information = 0;
            break;
        }

        default:
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS FindDiscordCanaryPID(
    _Out_ PULONG DiscordPID
)
{
    NTSTATUS Status = STATUS_NOT_FOUND;
    ULONG ProcessCount = 0;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo = NULL;
    ULONG ProcessInfoSize = 0x10000;

    // Alocar buffer para informações de processo
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)ExAllocatePool(NonPagedPool, ProcessInfoSize);
    if (!ProcessInfo)
        return STATUS_INSUFFICIENT_RESOURCES;

    // Obter informações de processo
    Status = ZwQuerySystemInformation(
        SystemProcessInformation,
        ProcessInfo,
        ProcessInfoSize,
        NULL
    );

    if (!NT_SUCCESS(Status))
    {
        ExFreePool(ProcessInfo);
        return Status;
    }

    // Procurar por Discord Canary
    PSYSTEM_PROCESS_INFORMATION Current = ProcessInfo;
    while (TRUE)
    {
        if (Current->ImageName.Buffer)
        {
            if (RtlCompareUnicodeString(&Current->ImageName, &(UNICODE_STRING)RTL_CONSTANT_STRING(DISCORD_PROCESS_NAME), TRUE) == 0)
            {
                *DiscordPID = (ULONG)(ULONG_PTR)Current->UniqueProcessId;
                Status = STATUS_SUCCESS;
                DbgPrint("[DiscordInjector] Discord Canary encontrado! PID: %u\n", *DiscordPID);
                break;
            }
        }

        if (Current->NextEntryOffset == 0)
            break;

        Current = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)Current + Current->NextEntryOffset);
    }

    ExFreePool(ProcessInfo);
    return Status;
}

NTSTATUS InjectDLLIntoProcess(
    _In_ ULONG TargetPID,
    _In_ PUNICODE_STRING DLLPath
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE ProcessHandle = NULL;
    HANDLE ThreadHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    CLIENT_ID ClientID;
    PVOID DLLBuffer = NULL;
    ULONG DLLSize = 0;

    DbgPrint("[DiscordInjector] Iniciando injeção em PID: %u\n", TargetPID);

    // Abrir processo alvo
    ClientID.UniqueProcess = (HANDLE)(ULONG_PTR)TargetPID;
    ClientID.UniqueThread = NULL;

    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

    Status = ZwOpenProcess(
        &ProcessHandle,
        PROCESS_ALL_ACCESS,
        &ObjectAttributes,
        &ClientID
    );

    if (!NT_SUCCESS(Status))
    {
        DbgPrint("[DiscordInjector] Falha ao abrir processo: 0x%X\n", Status);
        return Status;
    }

    DbgPrint("[DiscordInjector] Injeção concluída\n");

    if (ProcessHandle)
        ZwClose(ProcessHandle);

    return Status;
}

NTSTATUS ExecuteDLLInProcess(
    _In_ HANDLE ProcessHandle,
    _In_ PVOID DLLBase
)
{
    UNREFERENCED_PARAMETER(ProcessHandle);
    UNREFERENCED_PARAMETER(DLLBase);

    return STATUS_SUCCESS;
}