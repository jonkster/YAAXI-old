### DISCLAIMER
### This is an example Makefile and it MUST be configured to suit your needs.
### For detailed explanations about all the available options,
### please refer to https://github.com/sudar/Arduino-Makefile/blob/master/arduino-mk-vars.md

### PROJECT_DIR
### This is the path to where you have created/cloned your project
PROJECT_DIR       = $(shell dirname $(shell pwd))

### ARDMK_DIR
### Path to the Arduino-Makefile directory.
ARDMK_DIR         = /usr/share/arduino

### ARDUINO_DIR
ARDUINO_DIR       = /usr/share/arduino

### USER_LIB_PATH
### Path to where the your project's libraries are stored.
#USER_LIB_PATH     =  $(realpath $(PROJECT_DIR)/lib)
USER_LIB_PATH     =  $(PROJECT_DIR)/lib

ARDUINO_LIBS      += Ethernet SPI Rotencoder 

### BOARD_TAG & BOARD_SUB
BOARD_TAG         = mega2560
#BOARD_SUB         = atmega2560

### MONITOR_PORT
MONITOR_PORT      = /dev/ttyACM*

### MONITOR_BAUDRATE
MONITOR_BAUDRATE  = 115200
MONITOR_CMD = minicom -D


### AVR_TOOLS_DIR
AVR_TOOLS_DIR     = /usr

### AVRDUDE
### Path to avrdude directory.
AVRDUDE           = /usr/bin/avrdude
AVRDUDE_CONF      = /etc/avrdude.conf

ISP_PORT     = /dev/ttyACM*

### CFLAGS_STD
CFLAGS_STD        = -std=gnu11

### CXXFLAGS_STD
### You can choose wich ever you like
# CXXFLAGS_STD      = -std=gnu++11
CXXFLAGS_STD      = -std=gnu++17


### CPPFLAGS
### Flags you might want to set for debugging purpose. Comment to stop.
CXXFLAGS         += -pedantic -Wall -Wextra
LDFLAGS          += -fdiagnostics-color


### OBJDIR
### Don't touch this!
### This is were you put the binaries you just compile using 'make'
CURRENT_DIR       = $(shell basename $(CURDIR))
OBJDIR            = $(PROJECT_DIR)/build/$(CURRENT_DIR)/$(BOARD_TAG)

### path to Arduino.mk, inside the ARDMK_DIR, don't touch.
include $(ARDMK_DIR)/Arduino.mk

include helper.mk
