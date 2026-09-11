#include <windows.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")

// Função que será executada quando a DLL for injetada
void ExecutePayload()
{
    // Este é o código original da sua DLL
    // Será executado dentro do processo Discord Canary
    
    MessageBoxW(
        NULL,
        L"DLL injetada com sucesso no Discord Canary!",
        L"Discord Kernel Injector",
        MB_OK | MB_ICONINFORMATION
    );

    // Você pode adicionar seu código aqui
    // Por exemplo:
    // - Hooks de API
    // - Modificação de funcionalidades
    // - Monitoramento
    // - Etc.
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            // Criar thread para executar o payload
            // Isso evita travar a inicialização do processo
            CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE)ExecutePayload,
                NULL,
                0,
                NULL
            );
            break;
        }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        {
            break;
        }
    }

    return TRUE;
}