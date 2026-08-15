# Wordclock, AVR Dx backend.
#
# The firmware core is not copied or vendored in here: the sources are picked up from
# ../../firmware/src and its header directories are put on the include path. There is
# therefore exactly one copy of the clock logic, shared with the simulator and the ESP32.
#
# Build and flash:
#   make -C platform/hardware
#   make -C platform/hardware flash
#
# Debian's avr-libc already knows this device, so no Microchip device pack is needed. For a
# Dx part it does not know, point DFP at an unpacked .atpack:
#   make DFP=/path/to/Microchip.AVR-Dx_DFP MCU=avr64dd32

MCU        ?= avr128da48
F_CPU      ?= 24000000UL
# UPDI programmer for `make flash`; pymcuprog names it, see its --help for the alternatives.
PROGRAMMER ?= pkobn_updi

BUILD_DIR  ?= build
TARGET     := $(BUILD_DIR)/wordclock

ROOT       := ../..
FIRMWARE   := $(ROOT)/firmware

# Every directory under the core's include tree, because its headers include each other by
# bare name rather than by path.
INCLUDE_DIRS := include include/stl $(shell find $(FIRMWARE)/inc -type d)
INCLUDES     := $(addprefix -I,$(INCLUDE_DIRS))

SOURCES := $(wildcard src/*.cpp) $(shell find $(FIRMWARE)/src -name '*.cpp')
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(subst $(ROOT)/,,$(SOURCES)))

# -fno-threadsafe-statics: the singletons are function-local statics, and the guards the
#   standard asks for pull in __cxa_guard_acquire, which avr-libc does not have. There is
#   one thread here, so the guards protect against nothing.
# -fno-exceptions / -fno-rtti: neither is used, and both cost flash whether used or not.
# -flto: worth about a quarter of the image, see the README.
CXXFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
            -std=gnu++17 -Os -flto \
            -Wall -Wextra \
            -fno-exceptions -fno-rtti -fno-threadsafe-statics \
            -ffunction-sections -fdata-sections \
            $(INCLUDES)

LDFLAGS  := -mmcu=$(MCU) -Os -flto -Wl,--gc-sections

ifdef DFP
CXXFLAGS += -B $(DFP)/gcc/dev/$(MCU)/ -I$(DFP)/include
LDFLAGS  += -B $(DFP)/gcc/dev/$(MCU)/
endif

.PHONY: all size flash clean

all: $(TARGET).hex size

$(BUILD_DIR)/%.o: $(ROOT)/%.cpp
	@mkdir -p $(dir $@)
	avr-g++ $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	avr-g++ $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	avr-g++ $(LDFLAGS) $^ -o $@

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -R .eeprom $< $@

size: $(TARGET).elf
	@avr-size --format=avr --mcu=$(MCU) $< 2>/dev/null || avr-size $<

flash: $(TARGET).hex
	pymcuprog write -d $(MCU) -t $(PROGRAMMER) -f $< --erase --verify

clean:
	rm -rf $(BUILD_DIR)
