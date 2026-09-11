#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <ntstatus.h>
#include <windef.h>

// IOCTL Codes
#define IOCTL_INJECT_DLL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_DISCORD_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Estrutura de dados para injeção
typedef struct _INJECT_DLL_REQUEST {
    ULONG DiscordPID;
    WCHAR DLLPath[260];
    ULONG DLLPathLength;
} INJECT_DLL_REQUEST, * PINJECT_DLL_REQUEST;

typedef struct _DISCORD_PID_RESPONSE {
    ULONG PID;
    BOOLEAN Found;
} DISCORD_PID_RESPONSE, * PDISCORD_PID_RESPONSE;

// Protótipos de funções
NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
);

NTSTATUS DispatchCreateClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS DispatchDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

NTSTATUS FindDiscordCanaryPID(
    _Out_ PULONG DiscordPID
);

NTSTATUS InjectDLLIntoProcess(
    _In_ ULONG TargetPID,
    _In_ PUNICODE_STRING DLLPath
);

NTSTATUS ExecuteDLLInProcess(
    _In_ HANDLE ProcessHandle,
    _In_ PVOID DLLBase
);