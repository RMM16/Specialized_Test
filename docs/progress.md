# Project Progress

## Purpose

This document records what has actually been completed, why each decision was
made and which parts were necessary for the firmware versus workarounds for the
original development PC.

## Completed work

### Exercise and execution plan

The original exercise is stored at:

```text
docs/input/Embedded Pre-Screen.pdf
```

The implementation was divided into short branches with an acceptance command
for each functionality. This keeps the Git history reviewable and prevents CAN,
configuration, concurrency and tests from being introduced in one large
unverifiable change.

### Zephyr manifest and application scaffold

`west.yml` declares this repository as a west manifest repository and pins
Zephyr to `v4.4.0`.

The scaffold contains:

- `app/CMakeLists.txt`
- `app/Kconfig`
- `app/prj.conf`
- `app/src/main.c`
- reserved `tests/unit/` structure

This was necessary. A Zephyr application requires CMake and Kconfig metadata in
addition to its C source files.

### Native simulator boot

The application entry point uses the standard C signature:

```c
int main(void)
```

It calls Zephyr `printk` to prove that the kernel boots and the console path is
working. The application was built and executed successfully with:

```bash
export ZEPHYR_TOOLCHAIN_VARIANT=host
west build -p always -b native_sim/native/64 Specialized_Test/app
west build -t run
```

The output was:

```text
*** Booting Zephyr OS build v4.4.0 ***
Specialized Test booted on native_sim
```

## Why it was not only a C compiler command

The application logic is C, but Zephyr is an RTOS with generated configuration,
devicetree data, kernel libraries and board-specific build rules. Running
`gcc main.c` alone would only compile a normal host program; it would not build
or validate a Zephyr application.

The roles are:

- C: firmware implementation
- Kconfig and `prj.conf`: build-time feature configuration
- CMake and Ninja: build generation and execution
- `west`: workspace and dependency management
- Python: Zephyr build scripts and test tooling
- `native_sim`: Zephyr board simulation on the host PC

A plain C prototype was possible, but it would not prove integration with
Zephyr and therefore would not satisfy the technical objective of the exercise.

## Necessary decisions

- Pin Zephyr to an exact tag for reproducible builds.
- Keep application logic in C.
- Use `native_sim` because no microcontroller is available.
- Keep future logic modules independent from Zephyr headers for unit testing.
- Preserve the exercise PDF and technical plan in the repository.
- Exclude generated workspaces, builds and downloaded toolchains from Git.

## Original-PC workarounds

The following were not product requirements:

- WSL1 instead of WSL2
- elevated PowerShell to access the registered Ubuntu distribution
- Miniconda because Ubuntu 20.04 system Python was too old
- `native_sim/native/64` because the default 32-bit ELF did not execute on the
  available WSL1 installation
- manual WSL kernel and Ubuntu installation attempts

These workarounds must not be copied to a modern PC unless the same limitations
exist. Use the clean setup in `README.md` or `docs/build.md`.

## Git history

Relevant completed commits:

- `d640d4b` - scaffold Zephyr application
- `085e2db` - document validated Zephyr environment
- `7954d7a` - boot application on `native_sim`

The feature branches were merged into `main` without squashing so the technical
steps remain visible.

## Branch 3: build-time configuration

`feat/build-time-config` added the Kconfig options for the CAN bus, the three
periodic TX messages and the optional start/stop/hello triggers, plus the
portable `app_config_model` (no Zephyr headers) that validates them: zero
periods, out-of-range standard/extended CAN IDs, trigger collisions and
TX/trigger ID collisions. `zephyr_app_config.c` bridges `CONFIG_*` into that
model, and `main.c` validates it at boot.

CI (`.github/workflows/build-and-test.yml`) builds the app for `native_sim` on
a bare `ubuntu-22.04` runner (Python 3.12, host GCC toolchain, no Zephyr SDK)
on every push, and is green as of this branch.

## Branch 4: portable control-state and formatter

`feat/portable-logic` added `app_can_message` (a portable classic-CAN frame
representation, decoupled from Zephyr's `struct can_frame`), `app_logic`
(the `printing_enabled` state machine deciding print/ignore/hello per
message) and `can_formatter` (renders timestamp, standard/extended ID, DLC
and payload into a caller buffer).

Both modules are Zephyr-free, return explicit error results instead of
dereferencing NULL arguments, and `can_formatter_format()` rejects a DLC
above 8 instead of silently truncating the payload while reporting the
original, inconsistent DLC. They are compiled into the app via CMake to
confirm they build and link, but are not yet called from `main.c`/runtime;
that wiring happens in Branches 6-8 alongside CAN TX/RX.

## Branch 5: unit coverage

`test/unit-coverage` added 26 `ztest` cases on the `unit_testing` platform:
10 for `app_config_model` (defaults, zero period, out-of-range standard and
extended IDs, trigger collisions, TX/trigger collisions, a standard and an
extended ID sharing a numeric value not colliding, invalid CAN bus
parameters, a NULL config pointer), 10 for `app_logic` (initial print state
with and without a start trigger, start/stop/hello decisions, a standard and
an extended frame with the same numeric ID not cross-matching a trigger,
NULL arguments) and 6 for `can_formatter` (standard/extended output,
zero-length and full payload, small-buffer truncation, invalid DLC, NULL
message, length-only query).

The `tests/unit/CMakeLists.txt` structure (the `unit_testing` board check,
`find_package(Zephyr COMPONENTS unittest)`, `testbinary` target) was
confirmed against the real Zephyr v4.4.0 sources already fetched into the
local workspace, instead of guessed. `west twister -T tests/unit -p
unit_testing` passes 26/26 locally and the equivalent CI step, dropped in
Branch 3 because the directory was still empty, is back and green.

Also fixed in this branch: a `can_formatter_format()` internal `snprintf()`
failure was being reported as `CAN_FORMATTER_ERR_NULL_ARG`; it now has its
own `CAN_FORMATTER_ERR_FORMAT_FAILED`.

## Branch 6: periodic CAN TX

`feat/can-tx-periodic` added `runtime.c`/`runtime.h`, the first Zephyr-
dependent module beyond `main.c` and `zephyr_app_config.c`. It gets the CAN
device from the `zephyr,canbus` devicetree chosen node (`can_loopback0` on
`native_sim`, enabled by default in `native_sim.dts`), starts it, and
schedules one `k_work_delayable` per configured TX message, each
resubmitting itself at its own period and filling its payload with
`sys_rand_get()`.

Two non-obvious Kconfig requirements found by actually building rather than
guessing: `CONFIG_CAN=y` (the loopback driver depends on the devicetree node
but the CAN subsystem itself is opt-in), and `CONFIG_ENTROPY_GENERATOR=y`
(without it, `sys_rand_get()` compiles but fails to link --
`ENTROPY_GENERATOR` is a `menuconfig` with no default, so the native_sim
fake entropy driver under it never gets built even though its own Kconfig
defaults to "y").

CI now runs the built binary for 3 seconds (enough for the fastest 250 ms
TX period to fire several times) and greps for all three TX IDs, not just
the boot/config lines.

## Next implementation step

Create `feat/can-rx-uart-printer` and add the CAN RX message queue and
printer thread: `can_add_rx_filter_msgq()`, conversion from the real
`struct can_frame` to the portable `app_can_message`, and printing through
`app_logic` + `can_formatter`.

