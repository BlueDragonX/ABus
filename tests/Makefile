# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := tests
ARDUINO_LIBS := ArduinoCaster AUnit
include ../../EpoxyDuino/EpoxyDuino.mk

test: all
	@./$(APP_NAME).out
