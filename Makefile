#
# Copyright (c) 2015 Sergi Granell (xerpi)
# based on Cirne's vita-toolchain test Makefile
#

TARGET		:= neopoppsp
PSPAPP = System_PSP

PSP_APP_NAME=NeoPopVITA
PSP_APP_VER=0.71.15

#SOURCES		:= src $(ZLIB)

CORE=Core
TLCS900=$(CORE)/TLCS-900h
Z80=$(CORE)/z80

SOURCES		:= System_PSP

INCLUDES=-I$(SOURCES) -I$(Z80) -I$(TLCS900) -I$(SOURCES)/unzip/ -I$(CORE)

BUILD_APP=$(Z80)/Z80.o $(Z80)/dasm.o $(CORE)/Z80_interface.o \
          $(TLCS900)/TLCS900h_registers.o \
          $(TLCS900)/TLCS900h_interpret_src.o \
          $(TLCS900)/TLCS900h_interpret_single.o \
          $(TLCS900)/TLCS900h_interpret_reg.o \
          $(TLCS900)/TLCS900h_interpret_dst.o \
          $(TLCS900)/TLCS900h_interpret.o \
          $(TLCS900)/TLCS900h_disassemble_src.o \
          $(TLCS900)/TLCS900h_disassemble_reg.o \
          $(TLCS900)/TLCS900h_disassemble_extra.o \
          $(TLCS900)/TLCS900h_disassemble_dst.o \
          $(TLCS900)/TLCS900h_disassemble.o \
          $(CORE)/dma.o $(CORE)/bios.o $(CORE)/biosHLE.o $(CORE)/mem.o \
          $(CORE)/interrupt.o $(CORE)/gfx.o $(CORE)/sound.o \
          $(CORE)/gfx_scanline_colour.o $(CORE)/gfx_scanline_mono.o \
          $(CORE)/flash.o $(CORE)/rom.o $(CORE)/state.o $(CORE)/neopop.o
BUILD_MZ=$(SOURCES)/unzip/ioapi.o $(SOURCES)/unzip/unzip.o
BUILD_PORT=$(PSPAPP)/emulate.o $(PSPAPP)/emumenu.o $(PSPAPP)/neopop.o \
             $(PSPAPP)/system_rom.o $(PSPAPP)/main.o


OBJS=$(BUILD_APP) $(BUILD_MZ) $(BUILD_PORT)

LIBS= -lpsplib -lvita2d -lfreetype -lpng -lz -lm  -lSceDisplay_stub -lSceGxm_stub 	\
	-lSceCtrl_stub -lSceAudio_stub -lSceRtc_stub -lScePower_stub -lSceAppUtil_stub

DEFINES = -DPSP -DPSP_APP_NAME=\"$(PSP_APP_NAME)\" -DPSP_APP_VER=\"$(PSP_APP_VER)\" \
					-DCHIP_FREQUENCY=22050 -DZIPSUPPORT -Dint32=int32_t -Dint16=int16_t -Du32=uint32_t \
					-Du64=uint64_t -DScePspDateTime=SceDateTime

PREFIX  = arm-vita-eabi
AS	= $(PREFIX)-as
CC      = $(PREFIX)-gcc
CXX			=$(PREFIX)-g++
READELF = $(PREFIX)-readelf
OBJDUMP = $(PREFIX)-objdump
CFLAGS  = -Wl,-q -O3 $(INCLUDES) $(DEFINES) -fno-exceptions \
					-fno-unwind-tables -fno-asynchronous-unwind-tables -ftree-vectorize \
					-mfloat-abi=hard -ffast-math -fsingle-precision-constant -ftree-vectorizer-verbose=2 -fopt-info-vec-optimized -funroll-loops
CXXFLAGS = $(CFLAGS) -fno-rtti -Wno-deprecated -Wno-comment -Wno-sequence-point
ASFLAGS = $(CFLAGS)



all: $(TARGET).velf

$(TARGET).velf: $(TARGET).elf
	#advice from xyzz strip before create elf
		$(PREFIX)-strip -g $<
	#i put db.json there use your location
		vita-elf-create  $< $@ $(VITASDK)/bin/db.json

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(ASFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).elf $(TARGET).velf $(OBJS) $(DATA)/*.h

copy: $(TARGET).velf
	@cp $(TARGET).velf ~/PSPSTUFF/compartido/$(TARGET).elf
