TARGET := njemu_cps2_libretro_psp1.a

CC  = psp-gcc
AR  = psp-ar


DEFINES  += -D__LIBRETRO__ -DPSP
#DEFINES  += -DVIDEO_RGB565
DEFINES  += -DBUILD_CPS2PSP=1 -DPSP_SLIM=1 -DPSP_VIDEO_32BPP=0 -DRELEASE=0 -D_PSP_FW_VERSION=150
DEFINES  += -Ddriver=njemu_driver -Ddriver_t=njemu_driver_t

#DEFINES  += -DCZ80_USE_JUMPTABLE



ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

CFLAGS  += -G0
CFLAGS  += $(DEFINES)

CFLAGS += -fstrict-aliasing -falign-functions=32 -falign-loops -falign-labels -falign-jumps -Wall -Wundef -Wpointer-arith  -Wbad-function-cast -Wwrite-strings -Wmissing-prototypes -Wsign-compare -DZLIB_CONST
CFLAGS += -Werror -fomit-frame-pointer

#CFLAGS += -march=allegrex -mfp32 -mgp32 -mlong32 -mabi=eabi
#CFLAGS += -fomit-frame-pointer -fstrict-aliasing
#CFLAGS += -falign-functions=32 -falign-loops -falign-labels -falign-jumps


OBJS := zip/zfile.o zip/unzip.o
OBJS += common/cache.o common/loadrom.o common/state.o common/coin.o
OBJS += psp/filer.o psp/ui_text.o psp/input.o psp/ticker.o psp/sound.o psp/video.o
OBJS += cpu/m68000/m68000.o cpu/m68000/c68k.o cpu/z80/z80.o cpu/z80/cz80.o
OBJS += sound/sndintrf.o

OBJS += cps2/cps2crpt.o cps2/driver.o cps2/memintrf.o cps2/inptport.o cps2/timer.o cps2/vidhrdw.o cps2/sprite.o cps2/eeprom.o sound/qsound.o
OBJS += cps2/cps2.o

OBJS += emumain.o
OBJS += libretro.o

INCDIRS := -I. -Icps2
INCDIRS += -I$(shell psp-config --pspsdk-path)/include
#INCDIRS += -I$(shell psp-config --psp-prefix)/include

all: $(TARGET) copy_lib

$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

copy_lib:
	cp $(TARGET) ../Retroarch/libretro_psp1.a

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCDIRS)

clean-objs:
	rm -f $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

.PHONY: copy_lib clean clean-objs

