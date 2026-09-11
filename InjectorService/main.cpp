#include "Service.h"
#include <conio.h>

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 2)
    {
        printf("Discord Kernel DLL Injector v1.0\n");
        printf("=====================================\n\n");
        printf("Uso: InjectorService.exe <opcao>\n\n");
        printf("Opcoes:\n");
        printf("  inject      - Executar injetor\n");
        printf("  install     - Instalar driver\n");
        printf("  uninstall   - Desinstalar driver\n");
        printf("\n");
        printf("Exemplo: InjectorService.exe inject\n");
        return 1;
    }

    InjectorService Service;
    std::wstring Command = argv[1];

    if (Command == L"inject")
    {
        printf("[*] Iniciando injetor...\n");
        printf("[*] Execute Discord Canary como Administrador\n\n");

        if (!Service.StartService())
        {
            printf("[!] Falha ao iniciar serviço\n");
            return 1;
        }

        printf("\nPressione qualquer tecla para parar...\n");
        _getch();

        Service.StopService();
    }
    else if (Command == L"install")
    {
        printf("[*] Instalando driver...\n");
        if (Service.InstallDriver())
        {
            printf("[+] Driver instalado com sucesso\n");
        }
        else
        {
            printf("[!] Falha ao instalar driver\n");
            return 1;
        }
    }
    else if (Command == L"uninstall")
    {
        printf("[*] Desinstalando driver...\n");
        if (Service.UninstallDriver())
        {
            printf("[+] Driver desinstalado com sucesso\n");
        }
        else
        {
            printf("[!] Falha ao desinstalar driver\n");
            return 1;
        }
    }
    else
    {
        printf("[!] Comando desconhecido: %ws\n", argv[1]);
        return 1;
    }

    return 0;
}