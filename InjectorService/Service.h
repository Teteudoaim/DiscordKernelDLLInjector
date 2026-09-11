#pragma once

#include <windows.h>
#include <winsvc.h>
#include <stdio.h>
#include <string>

class InjectorService
{
public:
    InjectorService();
    ~InjectorService();

    BOOL StartService();
    BOOL StopService();
    BOOL InstallDriver();
    BOOL UninstallDriver();
    BOOL MonitorAndInject();

private:
    HANDLE m_DriverHandle;
    BOOL m_IsRunning;

    static DWORD WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
    static VOID WINAPI ServiceCtrlHandler(DWORD Opcode);

    HANDLE OpenDriver();
    BOOL FindDiscordCanaryPID(ULONG& PID);
    BOOL InjectDLLIntoDiscord(ULONG PID, const std::wstring& DLLPath);
    BOOL GetSystemPath(std::wstring& Path);
};

// Estruturas compartilhadas com o driver
#define IOCTL_INJECT_DLL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_DISCORD_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _INJECT_DLL_REQUEST {
    ULONG DiscordPID;
    WCHAR DLLPath[260];
    ULONG DLLPathLength;
} INJECT_DLL_REQUEST, * PINJECT_DLL_REQUEST;

typedef struct _DISCORD_PID_RESPONSE {
    ULONG PID;
    BOOLEAN Found;
} DISCORD_PID_RESPONSE, * PDISCORD_PID_RESPONSE;