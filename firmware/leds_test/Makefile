
OPENCM3_DIR=../libopencm3
SHARED_DIR=../common ../keyboard_common
PROJECT=keyboard_test
CFILES=main.c util.c keyboard_leds.c millis.c
AFILES=bitbang.S m0delay.S

DFU_FILE=$(PROJECT).dfu

TOOLS_DIR=../../tools
ELF_TO_DFUSE=$(TOOLS_DIR)/elf_to_dfuse.py

DFU_ID_VENDOR=0483
DFU_ID_PRODUCT=df11
DFU_UTIL=dfu-util

# target chip
DEVICE=stm32f072c8t6

%.dfu: %.elf
	python3 $(ELF_TO_DFUSE) $(DFU_ID_VENDOR):$(DFU_ID_PRODUCT) $< $@

default: all $(DFU_FILE)

INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
VPATH += $(SHARED_DIR)

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
include ../rules.mk

dfu_flash: $(DFU_FILE)
	$(DFU_UTIL) -a 0 -D $<
	

