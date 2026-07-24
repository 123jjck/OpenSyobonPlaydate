.PHONY: all simulator device run assets clean

PRODUCT = SyobonAction.pdx

SDK = ${PLAYDATE_SDK_PATH}
ifeq ($(SDK),)
	SDK = $(shell sed -n 's/^SDKRoot[[:space:]]*//p' ~/.Playdate/config | head -1)
endif
ifeq ($(SDK),)
	$(error Set PLAYDATE_SDK_PATH or configure SDKRoot in ~/.Playdate/config)
endif

TRGT = arm-none-eabi-
TOOLCHAIN = $(dir $(shell which $(TRGT)gcc))
ifeq ($(TOOLCHAIN),)
	TOOLCHAIN = /usr/local/bin/
endif

CXX = $(TOOLCHAIN)$(TRGT)g++
LD = $(TOOLCHAIN)$(TRGT)gcc
PDC = $(SDK)/bin/pdc
LDSCRIPT = $(SDK)/C_API/buildsupport/link_map.ld

SOURCES = src/DxLib.cpp src/loadg.cpp src/main.cpp src/cpp_setup.cpp
OBJECTS = $(patsubst src/%.cpp,build/%.o,$(SOURCES))
DEPENDENCIES = $(patsubst build/%.o,build/dep/%.d,$(OBJECTS))

MCFLAGS = -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16
CPPFLAGS = -I. -I$(SDK)/C_API -DTARGET_PLAYDATE=1 -DTARGET_EXTENSION=1
CXXFLAGS = $(MCFLAGS) -O2 -g3 -std=gnu++17
CXXFLAGS += -Wall -Wno-unused -Wno-unknown-pragmas -Wno-parentheses
CXXFLAGS += -Wdouble-promotion
CXXFLAGS += -falign-functions=16 -fomit-frame-pointer -fno-common
CXXFLAGS += -ffunction-sections -fdata-sections -mword-relocations
CXXFLAGS += -fno-exceptions -fno-rtti -fno-use-cxa-atexit
CXXFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables
CXXFLAGS += -Wstack-usage=4096 -Walloca-larger-than=4096

LDFLAGS = -nostartfiles $(MCFLAGS) -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map=build/pdex.map,--cref,--gc-sections
LDFLAGS += -Wl,--no-warn-mismatch,--emit-relocs

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	SIMULATOR_EXT = dylib
	SIMULATOR_CXX = clang++
	SIMULATOR_FLAGS = -dynamiclib -rdynamic -Wno-absolute-value
else
	SIMULATOR_EXT = so
	SIMULATOR_CXX = g++
	SIMULATOR_FLAGS = -shared -fPIC
endif

all: Source/pdex.elf Source/pdex.$(SIMULATOR_EXT)
	rm -rf "$(PRODUCT)"
	"$(PDC)" Source "$(PRODUCT)"

simulator: Source/pdex.$(SIMULATOR_EXT)
	rm -rf "$(PRODUCT)"
	"$(PDC)" Source "$(PRODUCT)"

device: Source/pdex.elf
	rm -rf "$(PRODUCT)"
	"$(PDC)" Source "$(PRODUCT)"

run: all
	open -a "$(SDK)/bin/Playdate Simulator.app" "$(CURDIR)/$(PRODUCT)"

assets:
	python3 tools/convert_assets.py ../OpenSyobonAction/res Source/res
	tools/convert_audio.sh ../OpenSyobonAction Source
	python3 tools/make_system_assets.py

Source/pdex.elf: build/pdex.elf
	cp "$<" "$@"

Source/pdex.$(SIMULATOR_EXT): $(SOURCES)
	mkdir -p build
	$(SIMULATOR_CXX) -g $(SIMULATOR_FLAGS) -lm \
		-DTARGET_SIMULATOR=1 -DTARGET_EXTENSION=1 \
		-I. -I"$(SDK)/C_API" \
		-o build/pdex.$(SIMULATOR_EXT) $(SOURCES)
	cp build/pdex.$(SIMULATOR_EXT) "$@"

build/pdex.elf: $(OBJECTS) $(LDSCRIPT)
	$(LD) $(OBJECTS) $(LDFLAGS) -o "$@"

build/%.o: src/%.cpp
	mkdir -p build build/dep
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) \
		-MMD -MP -MF "build/dep/$*.d" \
		-c "$<" -o "$@"

clean:
	rm -rf build "$(PRODUCT)"
	rm -f Source/pdex.elf Source/pdex.dylib Source/pdex.so

-include $(DEPENDENCIES)
