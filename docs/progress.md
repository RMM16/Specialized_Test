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
`native_sim`, enabled by default in `native_sim.dts`), applies
`config->can` (loopback mode via `can_set_mode()`, bitrate and sample point
via `can_calc_timing()` + `can_set_timing()`, both required before
`can_start()` since the driver rejects them with `-EBUSY` afterwards),
starts it, and schedules one `k_work_delayable` per configured TX message,
each resubmitting itself at its own period and filling its payload with
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

## Branch 7: CAN RX message queue and printer

`feat/can-rx-uart-printer` added `runtime_start_rx_printer()`: two RX
filters on a shared `k_msgq` (one for standard, one for extended IDs --
`can_filter`'s IDE flag is matched exactly rather than masked, so a single
catch-all filter cannot cover both), a consumer thread converting each
`struct can_frame` into the portable `app_can_message` (`k_uptime_get()` for
the timestamp), and printing it via `can_formatter_format()`.

Deliberately not wired to `app_logic` yet: every received frame is printed
unconditionally, including the loopback of our own periodic TX. Branch 8
adds the `app_logic_handle_message()` call that turns this into "print only
when the configured start/stop/hello state says so."

CI now also greps the captured boot output for a formatted RX line
(`ID=0x10[012] (STD) DLC=8 DATA=...`), not just the raw TX log lines.

## Branch 8: wire start/stop/hello triggers

`feat/wire-triggers` connects the portable `app_logic` to the RX printer
thread added in Branch 7. `runtime_start_rx_printer()` now takes
`const struct app_config *config` and calls `app_logic_init()` once with
`config->start_trigger`, `config->stop_trigger` and `config->hello_trigger`,
storing the resulting `struct app_logic_state` in a module-static variable
next to `rx_msgq` and `rx_printer_started`. `main.c` passes `&config`.

In `rx_thread_entry()`, after converting the frame to `app_can_message`,
`app_logic_handle_message()` decides the action:

- `APP_LOGIC_ACTION_PRINT`: format with `can_formatter_format()` and
  `printk()` it, same as before.
- `APP_LOGIC_ACTION_HELLO`: `printk("hello specialized\n")`. The exact
  greeting text is not specified anywhere in `docs/plan.md` beyond
  "hello specialized" (also the name of the optional CLI trigger in the
  exercise), so that literal string was chosen rather than inventing
  different wording.
- `APP_LOGIC_ACTION_NONE`: print nothing (this is what happens for the
  start/stop control frames themselves, and for any frame while printing
  is disabled).
- `APP_LOGIC_ACTION_INVALID_ARGS`: not expected on this path since both
  arguments are always valid local objects, but handled defensively with a
  `printk()` warning instead of asserting or panicking, so a single
  unexpected return value cannot take the RX thread down.

Validated locally on `native_sim` (WSL2 west build, `ZEPHYR_TOOLCHAIN_VARIANT=host`):
with the default Kconfig (all three triggers disabled), `printing_enabled`
starts `true` and every loopback TX frame is still printed exactly as in
Branch 7. With `-DCONFIG_APP_START_TRIGGER_ENABLE=y
-DCONFIG_APP_STOP_TRIGGER_ENABLE=y -DCONFIG_APP_HELLO_TRIGGER_ENABLE=y`,
`printing_enabled` starts `false` (a start trigger is configured) and,
since nothing on the bus ever transmits the configured start ID `0x200`
(only `0x100`/`0x101`/`0x102` are sent), no formatted RX line appears for
the full 3-second run -- only the raw `CAN TX sent ID ...` lines from the
TX side. This confirms the "blocked before start" half of the trigger
behavior.

What was *not* validated, locally or in CI: actually crossing the
start/stop/hello transitions by injecting a frame with one of the trigger
IDs. Doing that needs either a second CAN peer or a test-only frame
injector wired into the build, neither of which exists yet -- that gap is
explicitly left for Branch 9 (`test/native-sim-smoke`).

CI mirrors the local validation: the existing "default configuration" run
is untouched (still greps for the formatted RX lines, proving triggers-off
behavior is unchanged), and a new step runs the already-built "alternate
configuration" binary (all three triggers enabled) for 3 seconds and
asserts that no formatted `ID=0x10[012] (STD) DLC=8 DATA=...` line appears,
while the raw TX log lines still do.

`tests/unit/` was not touched; the existing 26 `ztest` cases for
`app_config_model`, `app_logic` and `can_formatter` still pass unchanged
since `app_logic.c`'s public API did not change, only how `runtime.c`
calls it.

## Branch 9: native_sim smoke scenario

`test/native-sim-smoke` closed the gap left by Branch 8: with no second CAN
bus peer available, there was no way to actually cross the start/stop/hello
transitions in CI, only to prove printing stayed blocked.

Since `can_loopback0` loops everything back to its own RX filters, the app
can inject the trigger frames itself. `CONFIG_APP_SMOKE_TEST_INJECT_TRIGGERS`
(off by default, only set in the new `app/prj_smoke.conf` overlay) adds
`runtime_start_smoke_injector()`: a one-shot `k_work_delayable` sequence that
sends the enabled start, hello and stop trigger IDs 600 ms apart. Guarded by
`#if defined(CONFIG_APP_SMOKE_TEST_INJECT_TRIGGERS)` so it compiles to
nothing -- not just a runtime no-op -- in every other configuration,
including the default and alternate builds already in CI.

A single `native_sim` run with `prj_smoke.conf` now shows the entire
lifecycle in order: periodic TX with no RX output, the start frame, RX
output resuming, the hello frame, `hello specialized`, the stop frame, and
RX output stopping again. CI does not just grep for those lines -- an `awk`
check confirms every formatted RX line's position falls between the start
and stop marker lines, i.e. the gating is real, not coincidental ordering.

## Delivery status

The native_sim implementation, unit tests, and smoke scenario are complete
and green in CI. Remaining future work is validation on a physical CAN
target.

