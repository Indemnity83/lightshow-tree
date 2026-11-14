SDCC ?= sdcc
PACKIHX ?= packihx
STC8PROG ?= tools/stc8prog/stc8prog
BAUD ?= 115200

TARGET ?= blank
BUILD_DIR := build/$(TARGET)
SOURCES := src/main.c src/board.c
CFLAGS := -mmcs51 --model-small --opt-code-size -Iinclude

ifneq (,$(filter $(TARGET),mapper color-test demo receiver))
SOURCES += src/mapper.c src/serial.c
endif
ifeq ($(TARGET),mapper)
CFLAGS += -DFIRMWARE_MAPPER
endif
ifeq ($(TARGET),color-test)
CFLAGS += -DFIRMWARE_COLOR_TEST
endif
ifeq ($(TARGET),demo)
CFLAGS += -DFIRMWARE_DEMO
endif
ifeq ($(TARGET),receiver)
SOURCES += src/receiver.c
CFLAGS += -DFIRMWARE_RECEIVER --opt-code-speed
endif
ifeq ($(TARGET),probe)
SOURCES += src/probe.c src/serial.c
CFLAGS += -DFIRMWARE_PROBE
endif
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.rel)
FIRMWARE := $(BUILD_DIR)/lightshow-tree

.PHONY: all clean check-toolchain detect flash blank mapper color-test demo receiver probe flash-blank flash-mapper flash-color-test flash-demo flash-receiver flash-probe terminal

all: $(FIRMWARE).hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.rel: src/%.c | $(BUILD_DIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(FIRMWARE).ihx: $(OBJECTS)
	$(SDCC) $(CFLAGS) $(OBJECTS) -o $@

$(FIRMWARE).hex: $(FIRMWARE).ihx
	$(PACKIHX) $< > $@

check-toolchain:
	@command -v $(SDCC) >/dev/null || (echo "SDCC is not installed"; exit 1)
	@command -v $(PACKIHX) >/dev/null || (echo "packihx is not installed"; exit 1)
	@command -v $(STC8PROG) >/dev/null || (echo "stc8prog is not installed"; exit 1)

detect:
	@test -n "$(PORT)" || (echo "Use: make detect PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT)

flash: $(FIRMWARE).hex
	@test -n "$(PORT)" || (echo "Use: make flash PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f $(FIRMWARE).hex

blank:
	$(MAKE) TARGET=blank all

mapper:
	$(MAKE) TARGET=mapper all

color-test:
	$(MAKE) TARGET=color-test all

demo:
	$(MAKE) TARGET=demo all

receiver:
	$(MAKE) TARGET=receiver all

probe:
	$(MAKE) TARGET=probe all

flash-blank:
	$(MAKE) TARGET=blank all
	@test -n "$(PORT)" || (echo "Use: make flash-blank PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f build/blank/lightshow-tree.hex

flash-mapper:
	$(MAKE) TARGET=mapper all
	@test -n "$(PORT)" || (echo "Use: make flash-mapper PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f build/mapper/lightshow-tree.hex

flash-color-test:
	$(MAKE) TARGET=color-test all
	@test -n "$(PORT)" || (echo "Use: make flash-color-test PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f build/color-test/lightshow-tree.hex

flash-demo:
	$(MAKE) TARGET=demo all
	@test -n "$(PORT)" || (echo "Use: make flash-demo PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f build/demo/lightshow-tree.hex

flash-receiver:
	$(MAKE) TARGET=receiver all
	@test -n "$(PORT)" || (echo "Use: make flash-receiver PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f build/receiver/lightshow-tree.hex

flash-probe:
	$(MAKE) TARGET=probe all
	@test -n "$(PORT)" || (echo "Use: make flash-probe PORT=/dev/cu.usbserial-XXXX"; exit 1)
	$(STC8PROG) -p $(PORT) -e -f build/probe/lightshow-tree.hex

terminal:
	@test -n "$(PORT)" || (echo "Use: make terminal PORT=/dev/cu.usbserial-XXXX"; exit 1)
	screen $(PORT) $(BAUD)

clean:
	rm -rf $(BUILD_DIR)
