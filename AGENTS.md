# lvglConfigPanel Agent Notes

This repo-local file captures the build and dependency context that kept biting
the LVGL/csim path. Keep it focused on durable project facts and recovery steps.

## Project Shape

- `lvglConfigPanel` is the LVGL-backed panel shared with the `winglevlr`/
  `autotrim` csim workflow.
- The project depends on the sibling `~/src/lv_port_linux` checkout for host
  simulation headers and built libraries.
- `notes` is the practical scratchpad for build/setup discoveries. If a note
  becomes long-lived, move the durable part here.

## Build And Runtime

- The csim flow uses the sibling host project and `espnow` pipe wiring:
  - `./winglevlr_csim --espnowPipe /tmp/fifo2 /tmp/fifo1`
  - `./lvglConfigPanel_csim --espnowPipe /tmp/fifo1 /tmp/fifo2`
- For the host LVGL support tree, use the CMake path:
  - `cd ~/src/lv_port_linux`
  - `cmake -B build`
  - `make -C build -j2`
- Do not use the old repo-root `make -j2` path for `lv_port_linux`; it can
  produce unresolved X11 symbols because it does not carry the right link
  setup through to the final binary.

## Dependency Notes

- Host simulation depends on a direct git clone of `TinyGPSPlus` in
  `~/Arduino/libraries/TinyGPSPlus`.
  - The Arduino library index only exposes `TinyGPSPlus 1.0.3`.
  - `arduino-cli lib install TinyGPSPlus@1.1.0` will fail because that version
    is not in the index.
  - Keep the git checkout synchronized across machines instead of trying to
    upgrade it through the Arduino registry.
- The LVGL host build has required X11/OpenGL/GLUT development packages in past
  troubleshooting, but the important fix for the failing csim path was the
  CMake build entry point, not an extra apt package.

## Cross-Repo Coordination

- `winglevlr` and `autotrim` both depend on this repo's LVGL host setup.
- If either downstream repo starts failing at `lvgl.h` include time, check that
  `~/src/lv_port_linux/lvgl/` exists and that the CMake-built host artifacts are
  present under `~/src/lv_port_linux/build/`.
- If the downstream repo's notes mention TinyGPSPlus, that is the same git
  checkout dependency described above, not an Arduino-indexed library.
