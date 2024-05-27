BOARD=esp32s3x
MONITOR_SPEED=921600

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"
BUILD_EXTRA_FLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\" 

X0=/home/jim/Arduino/libraries/LovyanGFX/src/lgfx_user
X1=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v0
X2=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/arduino_default
#X3=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32
X4=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32c3
X5=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s2
XF=|/home/jim/Arduino/libraries/LovyanGFX/src/lgfx/v1/platforms/esp32s3
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



# for esp32dax
#EXCLUDE_DIRS=/home/jim/Arduino/libraries/LovyanGFX|/home/jim/Arduino/libraries/TFT_eSPI/Extensions/|/home/jim/Arduino/libraries/TFT_eSPI/Fonts/|/home/jim/Arduino/libraries/TFT_eSPI/Processors

# for esp32s3x
#EXCLUDE_DIRS=/home/jim/Arduino/libraries/TFT_eSPI/Extensions/|/home/jim/Arduino/libraries/TFT_eSPI/Fonts/|/home/jim/Arduino/libraries/TFT_eSPI/Processors

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
