# Build

## Estado del Paso 0

El entorno queda validado para compilar la app Zephyr en `native_sim` desde este PC.

Resultado validado:

```bash
cd /mnt/c/Workspaces
export PATH=/root/miniconda/bin:$PATH
export ZEPHYR_TOOLCHAIN_VARIANT=host
west build -p always -b native_sim Specialized_Test/app
```

Resultado: build correcto con Zephyr `v4.4.0`.

## Restriccion del PC

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

## Entorno usado

Como workaround, se registro Ubuntu 20.04 en WSL1 desde una PowerShell elevada.

Datos verificados:

```text
Ubuntu 20.04.3 LTS
WSL version: 1
kernel: 4.4.0-18362-Microsoft
```

Nota importante: la distro quedo registrada en el contexto elevado usado para instalarla. Los comandos de Zephyr se ejecutan lanzando `wsl.exe -d Ubuntu` desde PowerShell elevada.

## Dependencias instaladas

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

## Workspace west

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

## Comandos reproducibles

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
west build -p always -b native_sim Specialized_Test/app
```

## Por que no se usa Zephyr SDK completo todavia

Para `native_sim` basta la toolchain host con:

```bash
export ZEPHYR_TOOLCHAIN_VARIANT=host
```

Instalar Zephyr SDK completo queda pospuesto hasta que haga falta compilar para una placa real o un target que no sea `native_sim`/`unit_testing`.

## Pendiente recomendado

Cuando se pueda tocar BIOS/UEFI:

1. Activar Intel VT-x / Virtualization Technology.
2. Convertir o reinstalar Ubuntu como WSL2.
3. Repetir:

```powershell
wsl --set-default-version 2
wsl -l -v
```

La solucion puede continuar mientras tanto sobre WSL1 porque `native_sim` ya compila.
