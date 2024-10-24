# Trying to build for the ele

BOARD=esp32s3
MONITOR_SPEED=115200

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"
BUILD_EXTRA_FLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\" 
BUILD_EXTRA_FLAGS += -DARDUINO_PARTITION_huge_app -DBOARD_HAS_PSRAM
#BUILD_EXTRA_FLAGS += -DF_CPU=240000000L -DARDUINO=10607 -DARDUINO_ESP32S3_DEV -DARDUINO_ARCH_ESP32 -DARDUINO_BOARD=\"ESP32S3_DEV\" -DARDUINO_VARIANT=\"esp32s3\" -DARDUINO_PARTITION_huge_app -DESP32 -DCORE_DEBUG_LEVEL=0 -DARDUINO_RUNNING_CORE=1 -DARDUINO_EVENT_RUNNING_CORE=1 -DBOARD_HAS_PSRAM -DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=0 -DARDUINO_USB_MSC_ON_BOOT=0 -DARDUINO_USB_DFU_ON_BOOT=0 @/tmp/arduino/sketches/15C8E895E625CE8A149B245E655C9A81/build_opt.h @/tmp/arduino/sketches/15C8E895E625CE8A149B245E655C9A81/file_opts 

LIBS += /home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s3/Panel_RGB.cpp 
LIBS += /home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s3/Bus_RGB.cpp
LIBS += /home/jim/Arduino/libraries/lvgl/demos/widgets/assets/img_lvgl_logo.c
LIBS += /home/jim/Arduino/libraries/lvgl/demos/widgets/assets/img_demo_widgets_avatar.c
LIBS += /home/jim/Arduino/libraries/lvgl/demos/widgets/assets/img_clothes.c

X0=/home/jim/Arduino/libraries/LovyanGFX/src/lgfx_user
X1=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v0
X2=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/arduino_default
X3=
#|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32
X4=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32c3
X5=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s2
XF=
#|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s3
X6=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp8266
X7=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/framebuffer
X8=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/opencv
X9=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/rp2040
XA=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/samd21
XB=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/samd51
XC=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/sdl
XD=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/spresense
XE=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/stm32
EXCLUDE_DIRS=$(X0)$(X1)$(X2)$(X3)$(X4)$(X5)$(X6)$(X7)$(X8)$(X9)$(XA)$(XB)$(XC)$(XD)$(XE)$(XF)



CHIP=esp32
include ${HOME}/Arduino/libraries/makeEspArduino/makeEspArduino.mk

fixtty:
	stty -F ${UPLOAD_PORT} -hupcl -crtscts -echo raw  ${MONITOR_SPEED}

cat:	fixtty
	cat ${UPLOAD_PORT}

climake:
	arduino-cli compile -b esp32:esp32:esp32s3:PartitionScheme=huge_app,PSRAM=opi --port /dev/ttyUSB0 -v -u

