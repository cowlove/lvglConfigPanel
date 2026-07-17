# Session Handoff — lvglConfigPanel

Updated 2026-07-17.

## Role in the system

This is the LVGL-backed panel used with the `winglevlr`/`autotrim` host
simulation. Its host build depends on the sibling `~/src/lv_port_linux`
checkout and the same TinyGPSPlus git checkout used by the navigation projects.

## Known-good host setup

Build the sibling LVGL host project through CMake:

```sh
cd ~/src/lv_port_linux
cmake -B build
make -C build -j2
```

The old top-level `make -j2` path in `lv_port_linux` can produce unresolved
X11 symbols. The CMake path carries the required host link setup.

The panel and winglevlr simulators communicate with named pipes, for example:

```sh
mkfifo /tmp/fifo1 /tmp/fifo2
./winglevlr_csim --espnowPipe /tmp/fifo2 /tmp/fifo1
./lvglConfigPanel_csim --espnowPipe /tmp/fifo1 /tmp/fifo2
```

Use the actual pipe names only when they do not already exist; avoid deleting
or replacing someone else's FIFOs.

## Dependency cautions

- The Arduino index does not provide the required TinyGPSPlus 1.1.0 package;
  use the synchronized git checkout instead.
- Historical Arduino builds used LVGL 8.3.11 and LovyanGFX 1.1.8/1.2.0 in
  specific board environments. Do not upgrade those blindly while debugging
  CSIM or hardware regressions.
- The repository has an existing uncommitted `AGENTS.md`; preserve it and
treat its build notes as the project-local source of truth.

## Expanded recovery notes

The panel is part of a two-process CSIM setup rather than a standalone demo.
The ESP-NOW pipe direction matters: `winglevlr_csim` is launched with
`/tmp/fifo2 /tmp/fifo1`, while `lvglConfigPanel_csim` is launched with
`/tmp/fifo1 /tmp/fifo2`. If a process blocks at startup, first check FIFO
existence, endpoint order, and whether an old process still owns a pipe.

The historical Arduino environment is sensitive to versions. A previously
working setup used ESP32 core 2.0.17, LVGL 8.3.11, and LovyanGFX 1.1.8/1.2.0;
newer versions may be valid but should be treated as an intentional migration,
not an incidental dependency update. The project notes also reference
`ESP32_Display_Panel` 0.0.2 for the relevant hardware path.

For hardware troubleshooting, distinguish compile failures from panel bring-up
failures. GT911 I2C errors can indicate a board/touch wiring or board-support
problem and are not automatically evidence of an LVGL widget bug. Restarting
the Arduino IDE previously helped with an index-out-of-range build failure, but
that is a workaround, not a dependency diagnosis.

Keep generated binaries and simulator logs out of durable handoff prose. Record
the command, board/library versions, display variant, and the first meaningful
error instead.
