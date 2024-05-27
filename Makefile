BOARD=esp32dax
MONITOR_SPEED=921600

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"
BUILD_EXTRA_FLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\" 
#-DFLASH_MODE=qio -DFLASH_SPEED=80m -DPART_FILE=/home/jim/.arduino15/packages/esp32/hardware/esp32/2.0.15/tools/partitions/huge_app.csv 

#EXCLUDE_DIRS=/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/internal
EXCLUDE_DIRS=/home/jim/Arduino/libraries/LovyanGFX|/home/jim/Arduino/libraries/TFT_eSPI/Extensions/|/home/jim/Arduino/libraries/TFT_eSPI/Fonts/|/home/jim/Arduino/libraries/TFT_eSPI/Processors

CHIP=esp32
OTA_ADDR=192.168.5.189
IGNORE_STATE=1

include ${HOME}/Arduino/libraries/makeEspArduino/makeEspArduino.mk

fixtty:
	stty -F ${UPLOAD_PORT} -hupcl -crtscts -echo raw  ${MONITOR_SPEED}

cat:	fixtty
	cat ${UPLOAD_PORT}

socat:  
	socat udp-recv:9000 - 
mocat:
	mosquitto_sub -h 192.168.5.1 -t "${MAIN_NAME}/#" -F "%I %t %p"   

${MAIN_NAME}_csim:	${MAIN_NAME}.ino ${HOME}/Arduino/libraries/jimlib/src/jimlib.h ${HOME}/Arduino/libraries/jimlib/src/ESP32sim_ubuntu.h
	g++  -DGIT_VERSION=\"$(GIT_VERSION)\" -x c++ -g $< -o $@ -DESP32 -DUBUNTU -I./ -I${HOME}/Arduino/libraries/jimlib/src 

csim: ${MAIN_NAME}_csim 
