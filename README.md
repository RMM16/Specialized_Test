# Specialized Test

Firmware exercise implemented with Zephyr RTOS and designed to be developed
and tested without a physical microcontroller.

## What this is

This repository is a take-home firmware exercise (the original brief is in
[docs/input/](docs/input/)) asking for a Zephyr application that, with
build-time configuration: periodically transmits 3 CAN messages with random
payloads, receives and prints CAN frames over UART/console (timestamp, CAN
ID, DLC, data), and supports optional `start`/`stop`/`hello specialized`
triggers that gate the printing -- all backed by unit tests.

No physical board was available, so the whole exercise is solved and
validated on Zephyr's `native_sim` board, using its CAN loopback driver as
the bus: the app's own periodic TX is what gets received and printed, and
(in the smoke scenario) the app injects its own trigger frames to exercise
the `start`/`stop`/`hello` transitions. What is intentionally *not* done is
anything that requires real hardware: no physical CAN transceiver, no real
second bus peer sending trigger frames, no board-specific drivers or
devicetree overlays beyond what `native_sim` already provides. The
"Not implemented yet" note further down spells out the one concrete gap
that follows from this (hardware-in-the-loop validation of the trigger
frames).

The implementation is split into ten short, independently validated
branches (scaffold, boot, build-time config, portable logic, unit tests,
periodic TX, RX printer, triggers, smoke scenario, and this documentation
pass), all merged into `main` -- see [docs/plan.md](docs/plan.md) for the
full roadmap and [docs/progress.md](docs/progress.md) for what was actually
built in each one.

## Current status

Completed:

- Zephyr manifest pinned to `v4.4.0`
- application scaffold
- successful build for `native_sim`
- visible boot message validated on `native_sim/native/64`
- build-time configuration via Kconfig (CAN bus, periodic TX, optional
  triggers) and a portable `app_config_model` that validates it
- portable `app_logic` (printing-enabled state, start/stop/hello trigger
  decisions) and `can_formatter` (timestamp, std/extended ID, DLC, payload),
  both Zephyr-free
- 26 `ztest` unit cases covering `app_config_model`, `app_logic` and
  `can_formatter` on the `unit_testing` platform
- periodic CAN TX: the three messages from build-time config are sent on
  `native_sim`'s loopback CAN device with a random payload, each on its own
  configured period, via `runtime.c`
- CAN RX printer: every received frame (the loopback of our own TX, for
  now) is converted to the portable `app_can_message` and routed through
  `app_logic`, on its own consumer thread reading a build-time-sized
  `k_msgq`
- optional start, stop and hello triggers wired end to end: `runtime.c`
  initializes `app_logic_state` from the build-time config and lets it
  decide, per received frame, whether to print the formatted line, print a
  hello greeting, or stay silent
- native_sim smoke scenario (`app/prj_smoke.conf` + `runtime_start_smoke_injector()`):
  with all three triggers enabled, the app sends its own start, hello and
  stop trigger frames once each over the loopback bus, proving the full
  trigger lifecycle end to end on a single CAN device, no second bus peer
  needed
- CI builds the app for `native_sim`, runs it (checking boot, config
  validation, periodic TX and the formatted RX output), runs the unit
  tests, builds the alternate configuration (all three triggers enabled,
  checking printing stays blocked with no real `0x200` frame transmitted),
  and builds + runs the smoke scenario, verifying every formatted RX line
  falls between the injected start and stop markers (see
  [.github/workflows/build-and-test.yml](.github/workflows/build-and-test.yml))

Validated output:

```text
*** Booting Zephyr OS build v4.4.0 ***
Specialized Test booted on native_sim
Build-time configuration validated
CAN TX sent ID 0x102 DLC 8
[260 ms] ID=0x102 (STD) DLC=8 DATA=3B DB 31 CC B6 D5 08 AA
CAN TX sent ID 0x101 DLC 8
[510 ms] ID=0x101 (STD) DLC=8 DATA=E1 BE 87 E4 7B 88 AE B1
```

Run the unit tests with:

```bash
west twister -T Specialized_Test/tests/unit -p unit_testing
```

Run the native_sim smoke scenario (the full start/hello/stop trigger
lifecycle, self-injected over the loopback CAN bus) with:

```bash
west build -p always -b native_sim -d build-smoke Specialized_Test/app -- \
  -DEXTRA_CONF_FILE=prj_smoke.conf
./build-smoke/zephyr/zephyr.exe
```

Stop it with `Ctrl+C`. Within the first couple of seconds the log shows
periodic TX with no RX output, the injected start trigger, RX output
resuming, the injected hello trigger (`hello specialized`), the injected
stop trigger, and RX output stopping again -- the same sequence CI verifies
with the `awk` ordering check in
[.github/workflows/build-and-test.yml](.github/workflows/build-and-test.yml).

Not implemented yet:

- a real second CAN bus peer (the smoke scenario injects trigger frames
  from the same node being tested, which is enough to validate the
  application's logic but not a substitute for hardware-in-the-loop testing)

The complete implementation order is defined in [docs/plan.md](docs/plan.md).

## Repository layout

```text
Specialized_Test/
|-- app/                 Zephyr application
|-- docs/
|   |-- input/           Original exercise PDF
|   |-- approach.md      Architecture and design rules
|   |-- build.md         Setup, build and troubleshooting
|   |-- plan.md          Branch and test roadmap
|   `-- progress.md      Work completed and technical rationale
|-- tests/unit/          Unit test area
|-- west.yml             Zephyr workspace manifest
`-- README.md
```

## Set up on another PC

Recommended environment:

- Ubuntu 24.04 or a current Linux distribution
- alternatively, Windows with WSL2 and Ubuntu
- Python 3.12 or newer
- Git, CMake, Ninja, devicetree compiler and GCC multilib

Install the Ubuntu dependencies:

```bash
sudo apt update
sudo apt install --no-install-recommends \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
  python3-dev python3-venv python3-tk xz-utils file make gcc gcc-multilib \
  g++-multilib libsdl2-dev libmagic1
```

Create the workspace from this manifest repository:

```bash
python3 -m venv ~/specialized-workspace/.venv
source ~/specialized-workspace/.venv/bin/activate
pip install west

west init -m https://github.com/RMM16/Specialized_Test.git \
  --mr main ~/specialized-workspace
cd ~/specialized-workspace
west update
west zephyr-export
west packages pip --install
```

For the current `native_sim` work, the host GCC toolchain is sufficient:

```bash
export ZEPHYR_TOOLCHAIN_VARIANT=host
west build -p always -b native_sim/native/64 Specialized_Test/app
west build -t run
```

Stop the simulator with `Ctrl+C`.

For future physical targets, install the Zephyr SDK:

```bash
cd ~/specialized-workspace/zephyr
west sdk install
```

Full setup details and the legacy WSL1 workaround used on the original PC are
kept in [docs/build.md](docs/build.md).

## Development workflow

Each functionality is implemented in a short branch, validated, committed and
then merged into `main`; the full branch sequence (now all merged) is
tracked in [docs/plan.md](docs/plan.md). For any future work on this
codebase, follow the same pattern:

```bash
git switch main
git pull --ff-only
git switch -c <type>/<short-description>
```

Do not commit `build/`, Python environments, IDE state, downloaded SDKs or
generated Zephyr workspaces.

## Language and architecture

The firmware is written in C. CMake, Kconfig, Python and `west` are build and
configuration tools required by Zephyr; they do not replace the C application.

Portable business logic (`app_config_model`, `app_logic`, `can_formatter`)
avoids Zephyr headers. Only the runtime adapter (`runtime.c`), entry point
(`main.c`) and Zephyr configuration bridge (`zephyr_app_config.c`) depend on
Zephyr APIs. This allows unit testing on a PC and later reuse on a physical
target.
