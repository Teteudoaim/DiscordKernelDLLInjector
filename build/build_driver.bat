@echo off
REM Script para compilar o driver usando Visual Studio

echo ======================================
echo Discord Kernel DLL Injector - Build Script
echo ======================================
echo.

REM Definir caminho do Visual Studio
set VS2022PATH=C:\Program Files\Microsoft Visual Studio\2022\Community

if not exist "%VS2022PATH%" (
    echo [!] Visual Studio 2022 nao encontrado em %VS2022PATH%
    echo [!] Por favor, ajuste o caminho manualmente
    pause
    exit /b 1
)

echo [*] Compilando projeto...
echo.

REM Compilar em modo Release x64
call "%VS2022PATH%\Common7\Tools\VsDevCmd.bat" -arch=x64

cd /d %~dp0..
msbuild DiscordKernelDLLInjector.sln /p:Configuration=Release /p:Platform=x64 /m

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [+] Compilacao concluida com sucesso!
    echo [+] Arquivos gerados em bin\Release\
) else (
    echo.
    echo [!] Falha na compilacao!
    pause
    exit /b 1
)

echo.
echo [*] Navegue para bin\Release\ para encontrar os executaveis
pause