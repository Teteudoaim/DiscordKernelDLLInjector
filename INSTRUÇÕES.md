# Discord Kernel DLL Injector - Instruções Detalhadas

## 📋 Pré-requisitos

- **Visual Studio 2022** Community Edition ou superior
- **Windows Driver Kit (WDK)** 10.0.22621.0 ou superior
- **Windows 10/11** com privilégios de Administrador
- Discord Canary instalado

## 🔧 Instalação dos Requisitos

### 1. Instalar Visual Studio 2022

1. Baixar em https://visualstudio.microsoft.com/
2. Selecionar "Desktop development with C++"
3. Instalar

### 2. Instalar Windows Driver Kit (WDK)

1. Baixar em https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
2. Executar o instalador
3. Selecionar "Install for all users"
4. Completar a instalação

## 📦 Compilação do Projeto

### Passo 1: Abrir a Solução

1. Abrir `Visual Studio 2022`
2. Ir em `File` → `Open` → `Project/Solution`
3. Navegar até `DiscordKernelDLLInjector.sln`
4. Selecionar e abrir

### Passo 2: Configurar a Solução

1. Selecionar `Release` como configuração
2. Selecionar `x64` como plataforma
3. Garantir que o WDK está instalado corretamente

### Passo 3: Compilar

**Método 1: Via Visual Studio**
- Pressionar `F7` ou ir em `Build` → `Build Solution`

**Método 2: Via Script**
- Executar `build/build_driver.bat` como Administrador
- Será compilado automaticamente

### Passo 4: Localizar os Arquivos Compilados

Os arquivos estarão em:
```
bin/x64/Release/
├── KernelDriver.sys
├── InjectorService.exe
└── TargetDLL.dll
```

## 🚀 Como Usar

### Passo 1: Instalar o Driver

```bash
# Como Administrador
cd bin/x64/Release
InjectorService.exe install
```

### Passo 2: Carregar o Driver

Use o `OSR Driver Loader` ou ferramentas similares:
1. Baixar OSR Driver Loader
2. Adicionar o driver `KernelDriver.sys`
3. Clicar "Start"

### Passo 3: Executar o Injetor

```bash
# Como Administrador
InjectorService.exe inject
```

### Passo 4: Abrir Discord Canary

1. Abrir Discord Canary **como Administrador**
2. O driver detectará automaticamente o processo
3. A DLL será injetada
4. Você verá a mensagem de confirmação

## 🔍 Troubleshooting

### Problema: Driver não é carregado

**Solução:**
1. Desabilitar verificação de assinatura de driver:
   - Reiniciar Windows em modo de diagnóstico
   - Usar BCDEDIT para desabilitar Code Integrity

```bash
# No PowerShell como Administrador
bcdedit.exe /set testsigning on
```

2. Reiniciar o computador
3. Tentar novamente

### Problema: Erro ao abrir o driver

**Solução:**
1. Verificar se o driver está instalado corretamente
2. Verificar privilégios de administrador
3. Reiniciar o computador

### Problema: DLL não é injetada

**Solução:**
1. Verificar se Discord Canary está em execução
2. Verificar se `TargetDLL.dll` está em `C:\Windows\System32\`
3. Verificar permissões de arquivo

## 🎯 Personalização

### Modificar o Payload da DLL

Editar `TargetDLL/dllmain.cpp`:

```cpp
void ExecutePayload()
{
    // Adicionar seu código aqui
    // Este código será executado no contexto do Discord Canary
    
    // Exemplo: Criar um arquivo de log
    FILE* f = fopen("C:\\discord_injected.txt", "w");
    fprintf(f, "DLL injetada com sucesso!");
    fclose(f);
}
```

### Modificar o Processo Alvo

Editar `KernelDriver/Driver.c`, função `FindDiscordCanaryPID()`:

```c
#define DISCORD_PROCESS_NAME L"SeuProcesso.exe"
```

## 📝 Estrutura do Projeto

```
KernelDriver/
├── Driver.c              # Implementação do driver
├── Driver.h              # Headers do driver
└── KernelDriver.vcxproj  # Configuração do projeto

InjectorService/
├── Service.cpp           # Serviço de injeção
├── Service.h             # Headers do serviço
├── main.cpp              # Ponto de entrada
└── InjectorService.vcxproj

TargetDLL/
├── dllmain.cpp           # Implementação da DLL
└── TargetDLL.vcxproj     # Configuração do projeto

build/
└── build_driver.bat      # Script de compilação

DiscordKernelDLLInjector.sln  # Solução Visual Studio
```

## ⚠️ Aviso de Segurança

Este projeto é destinado **apenas para fins educacionais e de pesquisa**. 

- Use-o apenas em ambientes controlados
- Não use para fins maliciosos
- Respeite as leis e regulamentações locais
- Injeção de código é uma atividade sensível do ponto de vista de segurança

## 📚 Recursos Adicionais

- [Microsoft WDK Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/)
- [Windows Kernel Architecture](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/)
- [DLL Injection Techniques](https://www.exploit-db.com/docs/english/17802-windows-dll-injection.pdf)

## 🤝 Suporte

Em caso de problemas, verifique:

1. Se o Visual Studio 2022 está instalado corretamente
2. Se o WDK está atualizado
3. Se o Discord Canary está instalado
4. Se está executando como Administrador
5. Se a assinatura de driver está desabilitada (para testes)

---

**Desenvolvido com ❤️ para fins educacionais**