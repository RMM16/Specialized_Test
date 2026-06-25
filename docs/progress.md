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

## Next implementation step

Create `feat/portable-logic` and implement the printing-enabled state machine,
`start`/`stop`/`hello` trigger handling and CAN message formatting described in
`docs/plan.md`, still without depending on Zephyr headers. Unit tests for this
logic, plus the config model added in Branch 3, are introduced in
`test/unit-coverage`.

