#!/bin/bash
python3 "${HOME}/.arduino15/packages/esp32/tools/esptool_py/4.5.1/esptool.py" --chip esp32s3 --port "/dev/ttyUSB0" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 *.ino.bootloader.bin 0x8000 *.ino.partitions.bin 0xe000 "${HOME}/.arduino15/packages/esp32/hardware/esp32/2.0.17/tools/partitions/boot_app0.bin" 0x10000 *.ino.bin   

