BOARD ?= esp32s3
PORT ?= /dev/ttyUSB0
CHIP ?= esp32
#ESP_ROOT=${HOME}/esp32
LIBS=$(foreach L,esp32jimlib Arduino_CRC32 Adafruit_HX711 PubSubClient \
	OneWireNg ArduinoJson DHT_sensor_library Adafruit_Unified_Sensor \
        TAMC_GT911 LovyanGFX lvgl esp32-micro-sdcard\
	,${HOME}/Arduino/libraries/${L})
PART_FILE=/home/jim/esp32/tools/partitions/huge_app.csv

UPLOAD_PORT ?= /dev/ttyUSB0
BUILD_EXTRA_FLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\" 
#BUILD_EXTRA_FLAGS += -DARDUINO_PARTITION_huge_app 
BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
BUILD_MEMORY_TYPE = qio_opi

#BUILD_EXTRA_FLAGS += -DF_CPU=240000000L -DARDUINO=10607 -DARDUINO_ESP32S3_DEV -DARDUINO_ARCH_ESP32 -DARDUINO_BOARD=\"ESP32S3_DEV\" -DARDUINO_VARIANT=\"esp32s3\" -DARDUINO_PARTITION_huge_app -DESP32 -DCORE_DEBUG_LEVEL=0 -DARDUINO_RUNNING_CORE=1 -DARDUINO_EVENT_RUNNING_CORE=1 -DBOARD_HAS_PSRAM -DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=0 -DARDUINO_USB_MSC_ON_BOOT=0 -DARDUINO_USB_DFU_ON_BOOT=0 @/tmp/arduino/sketches/15C8E895E625CE8A149B245E655C9A81/build_opt.h @/tmp/arduino/sketches/15C8E895E625CE8A149B245E655C9A81/file_opts 

LGFX=${HOME}/Arduino/libraries/LovyanGFX/src
LVGL=${HOME}/Arduino/libraries/lvgl
LIBS += ${LGFX}/lgfx/v1/platforms/esp32s3/Panel_RGB.cpp 
LIBS += ${LGFX}/lgfx/v1/platforms/esp32s3/Bus_RGB.cpp
LIBS += ${LVGL}/demos/widgets/assets/img_lvgl_logo.c
LIBS += ${LVGL}/demos/widgets/assets/img_demo_widgets_avatar.c
LIBS += ${LVGL}/demos/widgets/assets/img_demo_widgets_avatar.c
LIBS += ${LVGL}/demos/widgets/assets/img_clothes.c
LIBS += ${LVGL}/demos/widgets/assets/img_demo_widgets_needle.c

X0=${LGFX}/lgfx_user
X1=|${LGFX}/lgfx/v0
X2=|${LGFX}/lgfx/v1/platforms/arduino_default
X3=
#|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32
X4=|${LGFX}/lgfx/v1/platforms/esp32c3
X5=|${LGFX}/lgfx/v1/platforms/esp32s2
XF=
#|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s3
X6=|${LGFX}/lgfx/v1/platforms/esp8266
X7=|${LGFX}/lgfx/v1/platforms/framebuffer
X8=|${LGFX}/lgfx/v1/platforms/opencv
X9=|${LGFX}/lgfx/v1/platforms/rp2040
XA=|${LGFX}/lgfx/v1/platforms/samd21
XB=|${LGFX}/lgfx/v1/platforms/samd51
XC=|${LGFX}/lgfx/v1/platforms/sdl
XD=|${LGFX}/lgfx/v1/platforms/spresense
XE=|${LGFX}/lgfx/v1/platforms/stm32
XF=|${LGFX}/lgfx/internal
XG=|${HOME}/Arduino/libraries/LovyanGFX/examples
XH=|${LVGL}/src/libs/thorvg/rapidjson/msinttypes
XI=|${LVGL}/src/draw/sw/blend/helium
XJ=|${LVGL}/src/draw/sw/blend/neon

EXCLUDE_DIRS=$(X0)$(X1)$(X2)$(X3)$(X4)$(X5)$(X6)$(X7)$(X8)$(X9)$(XA)$(XB)$(XC)$(XD)$(XE)$(XF)$(XG)$(XH)$(XI)$(XJ)




GIT_VERSION := "$(shell git describe --abbrev=6 --dirty --always)"


EXTRA_CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
SKETCH_NAME=$(shell basename `pwd`)
BOARD_OPTIONS = PartitionScheme=min_spiffs


ifeq ($(BOARD),csim)
CSIM_BUILD_DIR=./build/csim
CSIM_LIBS=Arduino_CRC32 ArduinoJson Adafruit_HX711 esp32jimlib
CSIM_LIBS+=esp32csim
CSIM_SRC_DIRS=$(foreach L,$(CSIM_LIBS),${HOME}/Arduino/libraries/${L}/src)
CSIM_SRC_DIRS+=$(foreach L,$(CSIM_LIBS),${HOME}/Arduino/libraries/${L})
CSIM_SRC_DIRS+=$(foreach L,$(CSIM_LIBS),${HOME}/Arduino/libraries/${L}/src/csim_include)
CSIM_SRCS=$(foreach DIR,$(CSIM_SRC_DIRS),$(wildcard $(DIR)/*.cpp)) 
CSIM_SRC_WITHOUT_PATH = $(notdir $(CSIM_SRCS))
CSIM_OBJS=$(CSIM_SRC_WITHOUT_PATH:%.cpp=${CSIM_BUILD_DIR}/%.o)
CSIM_INC=$(foreach DIR,$(CSIM_SRC_DIRS),-I${DIR})

CSIM_CFLAGS+=-g -MMD -fpermissive -DGIT_VERSION=\"${GIT_VERSION}\" -DESP32 -DCSIM -DUBUNTU 
#CSIM_CFLAGS+=-DGPROF=1 -pg
#CSIM_CFLAGS+=-O2

${CSIM_BUILD_DIR}/%.o: %.cpp 
	echo $@
	${CCACHE} g++ ${CSIM_CFLAGS} -x c++ -c ${CSIM_INC} $< -o $@

${CSIM_BUILD_DIR}/%.o: %.ino
	echo $@
	${CCACHE} g++ ${CSIM_CFLAGS} -x c++ -c ${CSIM_INC} $< -o $@

${SKETCH_NAME}_csim: ${CSIM_BUILD_DIR} ${CSIM_OBJS} ${CSIM_BUILD_DIR}/${SKETCH_NAME}.o
	echo $@
	g++ -g ${CSIM_CFLAGS} ${CSIM_OBJS} ${CSIM_BUILD_DIR}/${SKETCH_NAME}.o -o $@         

csim: ${SKETCH_NAME}_csim 
	cp $< $@

${CSIM_BUILD_DIR}:
	mkdir -p ${CSIM_BUILD_DIR}

VPATH = $(sort $(dir $(CSIM_SRCS)))

.PHONY: csim-clean
csim-clean:
	rm -f ${CSIM_BUILD_DIR}/*.[od] ${SKETCH_NAME}_csim csim

-include ${CSIM_BUILD_DIR}/*.d
else
	include ~/Arduino/libraries/makeEspArduino/makeEspArduino.mk
endif

cat:    
	while sleep .01; do if [ -c ${PORT} ]; then stty -F ${PORT} -echo raw 115200 && cat ${PORT}; fi; done  | tee ./cat.`basename ${PORT}`.out
socat:  
	socat udp-recvfrom:9000,fork - 
mocat:
	mosquitto_sub -h rp1.local -t "${MAIN_NAME}/#" -F "%I %t %p"   
uc:
	${MAKE} upload && ${MAKE} cat

backtrace:
	tr ' ' '\n' | addr2line -f -i -e ./build/${BOARD}/*.elf


