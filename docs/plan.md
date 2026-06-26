# Specialized Test Plan

## 1. Objetivo

Resolver la prueba de firmware con Zephyr RTOS cumpliendo estos requisitos:

- imprimir por UART o consola los mensajes CAN recibidos
- incluir `timestamp`, `CAN ID`, `DLC` y `data`
- enviar 3 mensajes TX periodicos con payload aleatorio
- permitir configuracion en build-time
- soportar triggers opcionales `start`, `stop` y `hello specialized`
- incluir tests unitarios que demuestren la funcionalidad

Ademas, la solucion debe poder desarrollarse sin microcontrolador fisico.

## 2. Principios de diseno

### 2.1 Separacion de capas

Capas portables, sin headers de Zephyr:

- `app_config_model.*`
- `app_logic.*`
- `can_formatter.*`

Capas dependientes de Zephyr:

- `main.c`
- `runtime.c`
- `zephyr_app_config.c`

### 2.2 Alcance CAN

- la implementacion usara CAN clasico
- se trabajara con `struct can_frame`
- no se implementara CAN-FD
- no se contemplan `BRS`, `FDF` ni comportamiento especifico de tramas FD

### 2.3 Estrategia de validacion

- `unit_testing` para la logica portable
- `native_sim` para arranque local y smoke test
- `loopback` CAN en `native_sim` para validar RX/TX sin hardware

### 2.4 Estrategia de commits

- commits pequenos
- una funcionalidad por rama
- cada rama con criterio de validacion claro antes de mergear

## 3. Estructura objetivo

```text
Specialized_Test/
|-- README.md
|-- west.yml
|-- app/
|   |-- CMakeLists.txt
|   |-- prj.conf
|   |-- Kconfig
|   |-- boards/
|   |   `-- native_sim.overlay
|   |-- src/
|   |   |-- main.c
|   |   |-- zephyr_app_config.c
|   |   |-- app_config_model.c
|   |   |-- app_logic.c
|   |   |-- can_formatter.c
|   |   `-- runtime.c
|   `-- include/specialized/
|       |-- app_config_model.h
|       |-- app_logic.h
|       |-- can_formatter.h
|       `-- runtime.h
|-- tests/
|   `-- unit/
|       |-- CMakeLists.txt
|       |-- prj.conf
|       |-- tests.yaml
|       `-- src/
|           |-- test_app_config_model.c
|           |-- test_app_logic.c
|           `-- test_can_formatter.c
`-- docs/
    |-- input/
    |   `-- Embedded Pre-Screen.pdf
    |-- approach.md
    |-- build.md
    `-- plan.md
```

## 4. Responsabilidad de cada fichero

### 4.1 Ficheros de configuracion

- `west.yml`
  Define Zephyr como dependencia del workspace y fija la version.

- `app/CMakeLists.txt`
  Punto de entrada CMake de la aplicacion Zephyr.

- `app/prj.conf`
  Opciones base de configuracion Zephyr.

- `app/Kconfig`
  Opciones build-time de la aplicacion.

- `app/boards/native_sim.overlay`
  Ajustes de Devicetree solo si hacen falta para `native_sim`.

### 4.2 Ficheros de codigo portable

- `app_config_model.h/.c`
  Modelo de configuracion tipado, validaciones y reglas basicas.

- `app_logic.h/.c`
  Estado `printing_enabled`, evaluacion de triggers y decisiones de control.

- `can_formatter.h/.c`
  Formateo de linea para salida UART o consola.

### 4.3 Ficheros de integracion Zephyr

- `main.c`
  Entry point de la app Zephyr.

- `zephyr_app_config.c`
  Traduce `CONFIG_*` a la estructura portable.

- `runtime.c`
  Integra CAN, timers, RX queue, timestamps, UART/console y llamadas al nucleo portable.
  La fuente de timestamp sera `k_uptime_get()` o `k_uptime_get_32()`, expresado en milisegundos desde boot.

### 4.4 Tests

- `tests/unit/src/test_app_config_model.c`
  Tests del modelo y validacion de configuracion.

- `tests/unit/src/test_app_logic.c`
  Tests de triggers y estado de impresion.

- `tests/unit/src/test_can_formatter.c`
  Tests de formateo de salida.

## 5. Paso 0: entorno

Esto no es commit. Es prerequisito.

### 5.1 Objetivo del paso 0

Dejar un entorno capaz de:

- ejecutar `west update`
- compilar samples de Zephyr
- compilar `native_sim`
- ejecutar `unit_testing`

### 5.2 Checklist de entorno

1. Instalar `WSL2`.
2. Instalar `Ubuntu`.
3. Instalar dependencias Linux para Zephyr.
4. Crear workspace Linux.
5. Copiar o clonar este repo dentro del filesystem Linux.
6. Crear virtualenv Python.
7. Instalar `west`.
8. Verificar el tag de Zephyr a fijar.
9. Inicializar `west` con este repo como manifest repo.
10. Ejecutar `west update`.
11. Ejecutar `west zephyr-export`.
12. Instalar dependencias Python via `west packages pip --install`.
13. Instalar `Zephyr SDK`.
14. Validar `hello_world` en `native_sim`.
15. Verificar el nombre correcto del archivo de metadata de tests en la version pineada.

### 5.3 Criterio de salida del paso 0

El paso 0 se considera cerrado si:

- el tag elegido existe en el repo oficial
- `west update` termina sin errores
- el sample `hello_world` compila en `native_sim`
- el ejecutable generado arranca

## 6. Roadmap de ramas y funcionalidades

## 6.1 Rama 1

- Rama: `chore/scaffold-app-skeleton`
- Commit principal: `chore: scaffold zephyr app skeleton`

### Objetivo

Crear la base del proyecto sin implementar funcionalidad de negocio.

### Entregables

- `west.yml`
- estructura `app/`
- estructura `tests/unit/`
- estructura `docs/`
- movimiento del PDF a `docs/input/`

### Que no entra

- no CAN TX
- no CAN RX
- no triggers
- no formatter real
- no tests reales

### Validacion

- `west update`
- `west build -p always -b native_sim/native/64 Specialized_Test/app`

### Estado

Completada y fusionada en `main`.

### Riesgos

- error en `west.yml`
- typo en `CMakeLists.txt`
- estructura no alineada con `west init -l .`

## 6.2 Rama 2

- Rama: `feat/native-sim-boot`
- Commit principal: `feat: hello world boots on native_sim`

### Objetivo

Confirmar que el proyecto compila y arranca en `native_sim`.

### Entregables

- `main.c` minimo
- build funcional
- salida visible por consola

### Validacion

- `west build -p always -b native_sim/native/64 Specialized_Test/app`
- ejecucion del binario o `west build -t run`

### Criterio de aceptacion

La app arranca y emite una salida simple en consola.

### Estado

Completada y fusionada en `main`.

## 6.3 Rama 3

- Rama: `feat/build-time-config`
- Commit principal: `feat: add build-time configuration via Kconfig`

### Objetivo

Introducir toda la configuracion que debe estar disponible en build-time.

### Configuracion prevista

- bitrate CAN
- sample point
- modo loopback
- profundidad de cola RX
- `TX1_ID`, `TX1_PERIOD_MS`
- `TX2_ID`, `TX2_PERIOD_MS`
- `TX3_ID`, `TX3_PERIOD_MS`
- `START_TRIGGER_ENABLE`, `START_TRIGGER_ID`
- `STOP_TRIGGER_ENABLE`, `STOP_TRIGGER_ID`
- `HELLO_TRIGGER_ENABLE`, `HELLO_TRIGGER_ID`

### Restricciones de configuracion

- los triggers no pueden colisionar entre si
- un `TX_ID` no puede coincidir con un `START_TRIGGER_ID`, `STOP_TRIGGER_ID` o `HELLO_TRIGGER_ID`
- las validaciones se implementaran en `app_config_model`

### Entregables

- opciones `Kconfig`
- `prj.conf` base
- `zephyr_app_config.c` inicial
- `app_config_model.h` con estructuras tipadas

### Validacion

- compilar con defaults
- compilar con varios `-D` o configuraciones alternativas

### Criterio de aceptacion

La app compila con configuracion por defecto y con al menos una variante de configuracion.

### Estado

Completada en rama, validada por CI en `native_sim`. Pendiente de merge a `main`.

## 6.4 Rama 4

- Rama: `feat/portable-logic`
- Commit principal: `feat: add portable control-state and formatter modules`

### Objetivo

Crear el nucleo portable y estable de la solucion.

### Entregables

- `app_config_model.c`
- `app_logic.c`
- `can_formatter.c`
- headers asociados

### Logica a implementar

- inicializacion del estado
- activacion de impresion inmediata o bloqueada por trigger
- parada por trigger
- accion `hello specialized`
- validacion de configuracion
- serializacion de mensaje CAN a string de salida
- tratamiento del timestamp como milisegundos desde boot

### Validacion

- compila como parte de la app
- compila sin depender de headers Zephyr

### Criterio de aceptacion

La logica queda aislada de Zephyr y lista para ser testeada.

### Estado

Completada en rama, validada por CI en `native_sim`. Mergeada a `main`.

## 6.5 Rama 5

- Rama: `test/unit-coverage`
- Commit principal: `test: add ztest unit coverage for portable logic`

### Objetivo

Cerrar cobertura sobre la parte importante antes de tocar CAN real y concurrencia.

### Suite de tests prevista

#### `test_app_config_model.c`

- configuracion valida minima
- periodo invalido igual a cero
- IDs fuera de rango
- conflicto de triggers
- colision entre un `TX_ID` y un trigger
- configuracion con triggers deshabilitados

#### `test_app_logic.c`

- arranque con impresion activa si no hay trigger de inicio
- arranque bloqueado si `start` esta habilitado
- `start` habilita impresion
- `stop` deshabilita impresion
- `hello` dispara accion de saludo
- mensajes normales se imprimen si el estado esta activo
- mensajes normales no se imprimen si el estado esta inactivo

#### `test_can_formatter.c`

- mensaje STD con 0 bytes
- mensaje STD con 8 bytes
- mensaje EXT con 8 bytes
- truncado o proteccion ante buffer pequeno
- formato estable de timestamp, ID, DLC y payload

### Validacion

- `west twister -T tests/unit -p unit_testing`

### Criterio de aceptacion

Todos los tests unitarios pasan en verde.

### Estado

Completada en rama (26/26 tests en verde, local y CI). Pendiente de merge a `main`.

## 6.6 Rama 6

- Rama: `feat/can-tx-periodic`
- Commit principal: `feat: add periodic CAN TX on native_sim loopback`

### Objetivo

Implementar los 3 mensajes periodicos TX con payload aleatorio.

### Entregables

- timers o `k_work_delayable`
- construccion de `can_frame`
- payload aleatorio
- uso de los periodos configurados en build-time

### Validacion

- build en `native_sim`
- log o trazas que muestren envios periodicos

### Criterio de aceptacion

Se observa actividad TX recurrente de los 3 mensajes.

### Estado

Completada en rama, validada por CI (logs de TX de los 3 IDs) y en local. Pendiente de merge a `main`.

## 6.7 Rama 7

- Rama: `feat/can-rx-uart-printer`
- Commit principal: `feat: add CAN RX msgq and UART printer thread`

### Objetivo

Recibir los mensajes CAN, convertirlos al modelo interno y emitirlos formateados.

### Entregables

- `can_add_rx_filter_msgq()`
- `k_msgq`
- thread consumidor
- conversion `struct can_frame` -> struct interna
- llamada a `can_formatter`
- impresion por consola

### Validacion

- loopback de los propios TX
- ver en consola `timestamp`, `ID`, `DLC`, `data`

### Criterio de aceptacion

Los mensajes loopback aparecen impresos con el formato correcto.

## 6.8 Rama 8

- Rama: `feat/wire-triggers`
- Commit principal: `feat: wire optional start stop and hello triggers`

### Objetivo

Conectar la logica portable con el runtime de Zephyr.

### Entregables

- aplicacion de `start`
- aplicacion de `stop`
- impresion de `hello specialized`
- comportamiento por defecto si los triggers no estan definidos

### Casos funcionales

- sin `start`: imprime desde el principio
- con `start`: no imprime hasta recibir el ID
- sin `stop`: sigue indefinidamente
- con `stop`: deja de imprimir tras ese ID
- con `hello`: imprime saludo sin romper el flujo

### Validacion

- inyeccion de frames con IDs configurados
- observacion de consola

### Criterio de aceptacion

Cada trigger altera el comportamiento esperado y no introduce regresiones.

## 6.9 Rama 9

- Rama: `test/native-sim-smoke`
- Commit principal: `test: add native_sim smoke scenario`

### Objetivo

Tener una validacion minima de integracion end-to-end.

### Entregables

- script o escenario reproducible
- build + run documentados
- evidencia minima de TX, RX y triggers

### Validacion

- `west build -p always -b native_sim/native/64 Specialized_Test/app`
- ejecucion local de la app
- unit tests siguen verdes

### Criterio de aceptacion

La integracion general funciona en local sin hardware.

## 6.10 Rama 10

- Rama: `docs/setup-build-guide`
- Commit principal: `docs: add setup build run and test guide`

### Objetivo

Cerrar documentacion para entrega y mantenimiento.

### Entregables

- `README.md`
- `docs/build.md`
- `docs/approach.md`
- actualizacion de `docs/plan.md` si hiciera falta

### Contenido esperado

- setup de entorno
- comandos de build
- comandos de test
- explicacion de arquitectura
- como cambiar Kconfig
- limitaciones sin hardware fisico

## 7. Paso a paso de ejecucion real

### Paso 1

Preparar el entorno y validar `hello_world` oficial.

### Paso 2

Crear rama `chore/scaffold-app-skeleton`.

### Paso 3

Crear el esqueleto de carpetas y ficheros base.

### Paso 4

Compilar la app vacia en `native_sim`.

### Paso 5

Crear rama `feat/native-sim-boot` y anadir arranque minimo.

### Paso 6

Crear rama `feat/build-time-config` y anadir `Kconfig` y modelo de configuracion.

### Paso 7

Crear rama `feat/portable-logic` y escribir el nucleo portable.

### Paso 8

Crear rama `test/unit-coverage` y cerrar tests unitarios antes de integrar runtime.

### Paso 9

Crear rama `feat/can-tx-periodic` e integrar los 3 TX periodicos.

### Paso 10

Crear rama `feat/can-rx-uart-printer` para RX, cola e impresion.

### Paso 11

Crear rama `feat/wire-triggers` y conectar la logica de control.

### Paso 12

Crear rama `test/native-sim-smoke` y dejar la validacion integrada.

### Paso 13

Crear rama `docs/setup-build-guide` y cerrar documentacion.

## 8. Matriz de tests

## 8.1 Tests unitarios

- `config_valid_defaults`
- `config_rejects_zero_period`
- `config_rejects_invalid_id`
- `config_rejects_overlapping_triggers`
- `logic_starts_enabled_without_start_trigger`
- `logic_starts_disabled_with_start_trigger`
- `logic_start_trigger_enables_printing`
- `logic_stop_trigger_disables_printing`
- `logic_hello_trigger_sets_action`
- `logic_normal_message_prints_when_enabled`
- `logic_normal_message_skips_when_disabled`
- `formatter_std_id_output`
- `formatter_ext_id_output`
- `formatter_zero_length_payload`
- `formatter_full_payload`

## 8.2 Smoke tests manuales

- build limpia de `native_sim`
- arranque de la app
- observacion de los 3 TX periodicos
- observacion del loopback RX
- validacion visual del formato de salida
- validacion visual del trigger `hello`
- validacion visual del trigger `start`
- validacion visual del trigger `stop`

## 8.3 Regresiones a vigilar

- que la logica portable empiece a depender de Zephyr
- que el formatter cambie formato y rompa tests
- que un trigger interfiera con otro
- que un `TX_ID` coincida con un trigger y active comportamiento no deseado
- que el hilo de impresion bloquee el runtime
- que loopback funcione en `native_sim` pero no quede desacoplado para target real

## 9. Definicion de hecho

Una funcionalidad se considera cerrada cuando:

- compila
- tiene tests o validacion adecuada para su nivel
- no rompe la fase anterior
- queda documentada si introduce comandos o configuracion nueva

El proyecto se considera listo cuando:

- `native_sim` arranca
- los 3 TX periodicos funcionan
- RX imprime `timestamp`, `CAN ID`, `DLC`, `data`
- `start`, `stop` y `hello` funcionan segun configuracion
- `unit_testing` esta en verde
- la documentacion de build y test esta cerrada

## 10. Riesgos y mitigaciones

### Riesgo 1

`native_sim` no expone el comportamiento CAN esperado.

Mitigacion:

- validar temprano con sample o con un spike pequeno
- mantener el runtime desacoplado para poder adaptar la capa Zephyr sin tocar la logica

### Riesgo 2

La configuracion Kconfig se vuelve confusa.

Mitigacion:

- usar `ENABLE + ID` por trigger
- nombres consistentes
- defaults razonables

### Riesgo 3

La logica se mezcla con detalles de Zephyr.

Mitigacion:

- no incluir `<zephyr/...>` fuera de `runtime.c`, `main.c` y `zephyr_app_config.c`
- proteger esa regla en revision de codigo

### Riesgo 4

Falta de evidencia sin hardware real.

Mitigacion:

- tests unitarios fuertes
- smoke tests en `native_sim`
- logs claros en runtime

## 11. Comandos de validacion previstos

### Entorno

```bash
west update
west zephyr-export
west sdk install
west build -p always -b native_sim/native/64 zephyr/samples/hello_world
```

### App

```bash
west build -p always -b native_sim/native/64 Specialized_Test/app
```

### Ejecucion

```bash
west build -t run
```

### Tests unitarios

```bash
west twister -T tests/unit -p unit_testing
```

## 12. Orden recomendado de merge

1. `chore/scaffold-app-skeleton`
2. `feat/native-sim-boot`
3. `feat/build-time-config`
4. `feat/portable-logic`
5. `test/unit-coverage`
6. `feat/can-tx-periodic`
7. `feat/can-rx-uart-printer`
8. `feat/wire-triggers`
9. `test/native-sim-smoke`
10. `docs/setup-build-guide`
