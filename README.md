# Specialized Test

Firmware exercise implemented with Zephyr RTOS and designed to be developed
and tested without a physical microcontroller.

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
- CI builds the app for `native_sim`, runs it (checking boot, config
  validation, periodic TX and the formatted RX output), and runs the unit
  tests on every push; it also builds the alternate configuration (all
  three triggers enabled) and checks that, with the start trigger gating
  printing and no real `0x200` frame ever transmitted, none of the TX
  loopback frames get printed (see
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

Not implemented yet:

- integration test on `native_sim` exercising real CAN traffic end to end
  (actually crossing the start/stop/hello transitions needs a second CAN
  peer or a test-only frame injector; CI today only proves the
  "blocked before start" half of the trigger behavior)

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
then merged into `main`. With `feat/wire-triggers` merged, the remaining
follow-up from [docs/plan.md](docs/plan.md) is an optional native_sim smoke
scenario:

```bash
git switch main
git pull --ff-only
git switch -c test/native-sim-smoke
```

Do not commit `build/`, Python environments, IDE state, downloaded SDKs or
generated Zephyr workspaces.

## Language and architecture

The firmware is written in C. CMake, Kconfig, Python and `west` are build and
configuration tools required by Zephyr; they do not replace the C application.

Portable business logic will avoid Zephyr headers. Only the runtime adapter,
entry point and Zephyr configuration bridge will depend on Zephyr APIs. This
allows unit testing on a PC and later reuse on a physical target.
