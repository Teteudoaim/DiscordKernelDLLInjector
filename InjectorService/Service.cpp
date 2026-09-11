#include "Service.h"
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "winsvc.lib")

static InjectorService* g_Service = NULL;
static SERVICE_STATUS_HANDLE g_ServiceStatusHandle = NULL;

InjectorService::InjectorService() : m_DriverHandle(INVALID_HANDLE_VALUE), m_IsRunning(FALSE)
{
    g_Service = this;
}

InjectorService::~InjectorService()
{
    if (m_DriverHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_DriverHandle);
    }
}

BOOL InjectorService::StartService()
{
    printf("[*] Iniciando Injector Service\n");

    m_DriverHandle = OpenDriver();
    if (m_DriverHandle == INVALID_HANDLE_VALUE)
    {
        printf("[!] Falha ao abrir driver\n");
        return FALSE;
    }

    m_IsRunning = TRUE;
    printf("[+] Serviço iniciado com sucesso\n");

    return MonitorAndInject();
}

BOOL InjectorService::StopService()
{
    printf("[*] Parando Injector Service\n");
    m_IsRunning = FALSE;

    if (m_DriverHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_DriverHandle);
        m_DriverHandle = INVALID_HANDLE_VALUE;
    }

    printf("[+] Serviço parado\n");
    return TRUE;
}

HANDLE InjectorService::OpenDriver()
{
    HANDLE DriverHandle = CreateFileW(
        L"\\\\.\\DiscordInjector",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (DriverHandle == INVALID_HANDLE_VALUE)
    {
        printf("[!] Erro ao abrir driver: %d\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    printf("[+] Driver aberto com sucesso\n");
    return DriverHandle;
}

BOOL InjectorService::FindDiscordCanaryPID(ULONG& PID)
{
    DISCORD_PID_RESPONSE Response = { 0 };
    DWORD BytesReturned = 0;

    if (m_DriverHandle == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL Result = DeviceIoControl(
        m_DriverHandle,
        IOCTL_GET_DISCORD_PID,
        NULL,
        0,
        &Response,
        sizeof(Response),
        &BytesReturned,
        NULL
    );

    if (Result && Response.Found)
    {
        PID = Response.PID;
        printf("[+] Discord Canary encontrado - PID: %u\n", PID);
        return TRUE;
    }

    return FALSE;
}

BOOL InjectorService::InjectDLLIntoDiscord(ULONG PID, const std::wstring& DLLPath)
{
    INJECT_DLL_REQUEST Request = { 0 };
    DWORD BytesReturned = 0;

    if (m_DriverHandle == INVALID_HANDLE_VALUE)
        return FALSE;

    Request.DiscordPID = PID;
    Request.DLLPathLength = (ULONG)wcslen(DLLPath.c_str()) * sizeof(WCHAR);
    wcscpy_s(Request.DLLPath, 260, DLLPath.c_str());

    BOOL Result = DeviceIoControl(
        m_DriverHandle,
        IOCTL_INJECT_DLL,
        &Request,
        sizeof(Request),
        NULL,
        0,
        &BytesReturned,
        NULL
    );

    if (Result)
    {
        printf("[+] DLL injetada com sucesso em PID: %u\n", PID);
        return TRUE;
    }
    else
    {
        printf("[!] Falha ao injetar DLL: %d\n", GetLastError());
        return FALSE;
    }
}

BOOL InjectorService::GetSystemPath(std::wstring& Path)
{
    WCHAR SystemPath[MAX_PATH] = { 0 };
    if (!GetSystemDirectoryW(SystemPath, MAX_PATH))
        return FALSE;

    Path = SystemPath;
    return TRUE;
}

BOOL InjectorService::MonitorAndInject()
{
    ULONG DiscordPID = 0;
    std::wstring DLLPath;
    std::wstring SystemPath;

    printf("[*] Monitorando processos...\n");

    if (!GetSystemPath(SystemPath))
    {
        printf("[!] Falha ao obter caminho do sistema\n");
        return FALSE;
    }

    DLLPath = SystemPath + L"\\TargetDLL.dll";

    while (m_IsRunning)
    {
        if (FindDiscordCanaryPID(DiscordPID))
        {
            printf("[*] Tentando injetar DLL em PID: %u\n", DiscordPID);
            if (InjectDLLIntoDiscord(DiscordPID, DLLPath))
            {
                printf("[+] Injeção bem-sucedida!\n");
                break;
            }
            else
            {
                printf("[!] Falha na injeção, tentando novamente em 5 segundos...\n");
            }
        }

        Sleep(5000);
    }

    return TRUE;
}

DWORD WINAPI InjectorService::ServiceMain(DWORD argc, LPTSTR* argv)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    if (g_Service)
    {
        g_Service->StartService();
    }

    return 0;
}

VOID WINAPI InjectorService::ServiceCtrlHandler(DWORD Opcode)
{
    switch (Opcode)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
        {
            if (g_Service)
            {
                g_Service->StopService();
            }
            break;
        }

        default:
            break;
    }
}