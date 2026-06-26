# Approach

## Resumen

La estrategia del proyecto es separar la solucion en dos niveles:

- logica portable y testeable sin Zephyr
- capa de integracion con Zephyr para CAN, tiempo y consola

## Por que este enfoque

No hay microcontrolador fisico disponible, asi que la solucion tiene que permitir:

- validar comportamiento con `unit_testing`
- validar integracion minima con `native_sim`
- mantener la logica aislada de drivers y del kernel

## Regla principal

Estos modulos no deben depender de Zephyr:

- `app_config_model.*`
- `app_logic.*`
- `can_formatter.*`

Estos modulos si dependen de Zephyr:

- `main.c`
- `runtime.c`
- `zephyr_app_config.c`

## Estado actual

El entorno, scaffold y arranque minimo estan completados:

- Zephyr `v4.4.0` descargado con `west update`
- app scaffold compilada con `west build -p always -b native_sim/native/64 Specialized_Test/app`
- build hecho en Ubuntu 20.04 sobre WSL1, ejecutado desde PowerShell elevada
- toolchain usada: `ZEPHYR_TOOLCHAIN_VARIANT=host`
- ejecucion validada con `native_sim/native/64`
- salida visible mediante `printk`

La limitacion pendiente del PC es que WSL2 no puede arrancar porque la virtualizacion esta desactivada en BIOS/UEFI. Esto no bloquea continuar con `native_sim`, pero si conviene corregirlo cuando sea posible.

## Validacion de arranque

La `Rama 2` valido que la app arranca y que la consola funciona mediante un
`printk` minimo:

```text
*** Booting Zephyr OS build v4.4.0 ***
Specialized Test booted on native_sim
```

Se usa el qualifier oficial `native_sim/native/64` porque el target
`native_sim` por defecto genera un ELF de 32 bits que compila correctamente,
pero no puede ejecutarse sobre el WSL1 disponible en este PC.

Validacion:

```bash
export PATH=/root/miniconda/bin:$PATH
export ZEPHYR_TOOLCHAIN_VARIANT=host
cd /mnt/c/Workspaces
west build -p always -b native_sim/native/64 Specialized_Test/app
west build -t run
```

## Rama 3 completada

`feat/build-time-config` introdujo Kconfig (CAN bus, TX periodico, triggers
opcionales) y el modelo de configuracion tipado `app_config_model`, validado
en `main.c` al boot. El build sigue verde en CI sobre `native_sim`.

## Rama 4 completada

`feat/portable-logic` anadio `app_logic` (estado de impresion, triggers
`start`/`stop`/`hello`) y `can_formatter` (timestamp, ID estandar/extendido,
DLC y payload), ambos sin depender de Zephyr y con manejo explicito de
argumentos nulos y DLC invalido. Integrados en CMake, todavia sin conectar a
runtime.

## Rama 5 completada

`test/unit-coverage` anadio 26 casos `ztest` (10 de `app_config_model`, 10
de `app_logic`, 6 de `can_formatter`) sobre la plataforma `unit_testing`.
`west twister -T tests/unit -p unit_testing` pasa 26/26 en local y en CI.

## Siguiente hito

La siguiente rama es `feat/can-tx-periodic`: los 3 mensajes TX periodicos
con payload aleatorio sobre loopback en `native_sim`.
