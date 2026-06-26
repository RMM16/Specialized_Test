# Build

This document is the detailed build reference: the full rationale, the
original-PC workaround history and troubleshooting. For the short version
(dependencies, `west init`, build and run), see the
["Set up on another PC"](../README.md#set-up-on-another-pc) section in the
README -- the commands here are deliberately not repeated so the two docs
cannot drift apart.

Sections below are ordered from "applies to any modern setup" to "only
happened on the original development PC and should not be reproduced":

- **Clean setup on another PC** and **Update an existing workspace** apply
  to any clean setup, including the one validated on a second PC with WSL2 +
  Ubuntu 26.04 (see [docs/progress.md](progress.md)).
- Everything from **Legacy: validated original-PC environment** onward
  documents the WSL1 + Miniconda workaround used on the original development
  PC, where WSL2 could not be installed because virtualization was disabled
  in the BIOS/UEFI. It is kept for traceability only -- do not copy it on a
  modern PC.

## Clean setup on another PC

Follow the ["Set up on another PC"](../README.md#set-up-on-another-pc)
section in the README: dependencies, `west init`/`west update`, and the
`native_sim` build/run commands are the same regardless of host OS, as long
as WSL2 (or a native Linux install) is available.

For a future physical board, install the Zephyr SDK:

```bash
cd ~/specialized-workspace/zephyr
west sdk install
```

The SDK is not required for the current host-only validation.

## Update an existing workspace

```bash
cd ~/specialized-workspace
source .venv/bin/activate
git -C Specialized_Test switch main
git -C Specialized_Test pull --ff-only
west update
west build -p always -b native_sim/native/64 Specialized_Test/app
```

## Legacy: validated original-PC environment

El entorno queda validado para compilar la app Zephyr en `native_sim` desde este PC.

Resultado validado:

```bash
cd /mnt/c/Workspaces
export PATH=/root/miniconda/bin:$PATH
export ZEPHYR_TOOLCHAIN_VARIANT=host
west build -p always -b native_sim Specialized_Test/app
```

Resultado: build correcto con Zephyr `v4.4.0`.

El target por defecto genera un ejecutable ELF de 32 bits. Para compilar y
ejecutar la aplicacion en este PC se usa la variante oficial de 64 bits:

```bash
west build -p always -b native_sim/native/64 Specialized_Test/app
west build -t run
```

Salida validada:

```text
*** Booting Zephyr OS build v4.4.0 ***
Specialized Test booted on native_sim
```

El proceso del simulador permanece activo despues de que `main` retorna porque
Zephyr continua ejecutando el kernel. Esto es comportamiento esperado.

## Legacy: original-PC restriction

WSL2 no puede usarse ahora mismo porque la virtualizacion esta desactivada en BIOS/UEFI:

```text
VirtualizationFirmwareEnabled: False
```

Windows si tiene activadas las features necesarias:

- `Microsoft-Windows-Subsystem-Linux`: enabled
- `VirtualMachinePlatform`: enabled
- `hypervisorlaunchtype`: `Auto`

Tambien se aplico manualmente el kernel de WSL2 porque el MSI no lo instalaba correctamente:

- archivo: `C:\Windows\System32\lxss\tools\kernel`
- version registrada: `5.10.16`

Aunque eso desbloquea el aviso de kernel, WSL2 sigue sin poder importar distros hasta activar Intel VT-x/virtualizacion en BIOS.

## Legacy: original-PC workaround

Como workaround, se registro Ubuntu 20.04 en WSL1 desde una PowerShell elevada.

Datos verificados:

```text
Ubuntu 20.04.3 LTS
WSL version: 1
kernel: 4.4.0-18362-Microsoft
```

Nota importante: la distro quedo registrada en el contexto elevado usado para instalarla. Los comandos de Zephyr se ejecutan lanzando `wsl.exe -d Ubuntu` desde PowerShell elevada.

## Legacy: packages used on the original PC

Paquetes Linux instalados:

```bash
apt-get install -y --no-install-recommends \
  git gcc g++ make gperf ccache dfu-util device-tree-compiler wget xz-utils \
  file python3 python3-pip python3-venv python3-setuptools python3-wheel \
  libsdl2-dev gcc-multilib g++-multilib libc6-dev-i386
```

Python del sistema Ubuntu 20.04 no sirve para Zephyr `v4.4.0`:

- Python 3.8 falla con `patool>=2.0.0`
- Python 3.9 tambien falla porque `patool>=2.0.0` exige Python >= 3.10

Solucion aplicada:

- Miniconda instalado en `/root/miniconda`
- Python usado por Zephyr: Python 3.13.13
- herramientas instaladas con `pip`: `west`, `cmake`, `ninja`
- requisitos Zephyr instalados con `pip install -r zephyr/scripts/requirements.txt`

Versiones verificadas:

```text
west 1.5.0
cmake 4.3.4
ninja 1.13.0
```

## Legacy: original workspace layout

El workspace se inicializo en:

```text
C:\Workspaces
```

Estructura relevante:

```text
C:\Workspaces\.west
C:\Workspaces\zephyr
C:\Workspaces\modules
C:\Workspaces\Specialized_Test
```

El manifest local es:

```text
C:\Workspaces\Specialized_Test\west.yml
```

El tag `v4.4.0` de Zephyr fue comprobado antes de ejecutar `west update`.

## Legacy: original-PC commands

Desde PowerShell elevada:

```powershell
Start-Process powershell.exe -Verb RunAs
```

Dentro de esa PowerShell elevada:

```powershell
wsl.exe -d Ubuntu
```

Dentro de Ubuntu:

```bash
export PATH=/root/miniconda/bin:$PATH
export ZEPHYR_TOOLCHAIN_VARIANT=host
cd /mnt/c/Workspaces
west build -p always -b native_sim/native/64 Specialized_Test/app
```

## Legacy: why the full SDK was not used

Para `native_sim` basta la toolchain host con:

```bash
export ZEPHYR_TOOLCHAIN_VARIANT=host
```

Instalar Zephyr SDK completo queda pospuesto hasta que haga falta compilar para una placa real o un target que no sea `native_sim`/`unit_testing`.

## Legacy: recommended original-PC fix

Cuando se pueda tocar BIOS/UEFI:

1. Activar Intel VT-x / Virtualization Technology.
2. Convertir o reinstalar Ubuntu como WSL2.
3. Repetir:

```powershell
wsl --set-default-version 2
wsl -l -v
```

La solucion puede continuar mientras tanto sobre WSL1 porque `native_sim` ya compila.
