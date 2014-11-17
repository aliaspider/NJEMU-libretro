SYSTEM   = mvs
PSP      = slim
DEBUG    = 0

CC = psp-gcc
AR = psp-ar


TARGET  := njemu_$(SYSTEM)_libretro_psp1.a

DEFINES  += -D__LIBRETRO__ -DPSP
DEFINES  += -DRELEASE=0 -DINLINE='static __inline'
# avoids conflict with the variable 'driver' in the frontend's code
DEFINES  += -Ddriver=njemu_driver -Ddriver_t=njemu_driver_t

#DEFINES  += -DCZ80_USE_JUMPTABLE

## CPS1 ##
ifeq ($(SYSTEM), cps1)
DEFINES += -DBUILD_CPS1PSP=1
INCDIRS := -Icps1
OBJS    := cps1/cps1.o cps1/driver.o cps1/memintrf.o cps1/inptport.o cps1/dipsw.o cps1/timer.o
OBJS    += cps1/vidhrdw.o cps1/sprite.o cps1/eeprom.o cps1/kabuki.o sound/2151intf.o sound/ym2151.o sound/qsound.o
OBJS    += cpu/m68000/m68000.o cpu/m68000/c68k.o cpu/z80/z80.o cpu/z80/cz80.o common/coin.o

## CPS2 ##
else ifeq ($(SYSTEM), cps2)
DEFINES += -DBUILD_CPS2PSP=1
INCDIRS := -Icps2
OBJS    := cps2/cps2.o cps2/cps2crpt.o cps2/driver.o cps2/memintrf.o cps2/inptport.o cps2/timer.o
OBJS    += cps2/vidhrdw.o cps2/sprite.o cps2/eeprom.o sound/qsound.o
OBJS    += cpu/m68000/m68000.o cpu/m68000/c68k.o cpu/z80/z80.o cpu/z80/cz80.o common/coin.o

## MVS ##
else ifeq ($(SYSTEM), mvs)
DEFINES += -DBUILD_MVSPSP=1
INCDIRS := -Imvs
OBJS    := mvs/mvs.o mvs/driver.o mvs/memintrf.o mvs/inptport.o mvs/dipsw.o mvs/timer.o
OBJS    += mvs/vidhrdw.o mvs/sprite.o mvs/pd4990a.o mvs/neocrypt.o mvs/biosmenu.o sound/2610intf.o sound/ym2610.o
OBJS    += cpu/m68000/m68000.o cpu/m68000/c68k.o cpu/z80/z80.o cpu/z80/cz80.o

## NCDZ ##
else
DEFINES += -DBUILD_NCDZPSP=1
INCDIRS := -Incdz
OBJS    := cps2/cps2.o cps2/cps2crpt.o cps2/driver.o cps2/memintrf.o cps2/inptport.o cps2/timer.o cps2/vidhrdw.o cps2/sprite.o cps2/eeprom.o sound/qsound.o

endif

ifeq ($(PSP), fat)
DEFINES += -D_PSP_FW_VERSION=150
else # psp = slim
DEFINES += -DPSP_SLIM -D_PSP_FW_VERSION=371
endif



ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

CFLAGS  += -G0
CFLAGS  += $(DEFINES)

CFLAGS += -fstrict-aliasing -falign-functions=32 -falign-loops -falign-labels -falign-jumps -Wall -Wundef -Wpointer-arith  -Wbad-function-cast -Wwrite-strings -Wmissing-prototypes -Wsign-compare -DZLIB_CONST
CFLAGS += -fomit-frame-pointer
#CFLAGS += -Werror
#CFLAGS += -march=allegrex -mfp32 -mgp32 -mlong32 -mabi=eabi
#CFLAGS += -fomit-frame-pointer -fstrict-aliasing
#CFLAGS += -falign-functions=32 -falign-loops -falign-labels -falign-jumps


OBJS += zip/zfile.o zip/unzip.o
OBJS += common/cache.o common/loadrom.o common/state.o
OBJS += psp/filer.o psp/ui_text.o psp/input.o psp/ticker.o psp/sound.o psp/video.o
OBJS += sound/sndintrf.o


OBJS += emumain.o
OBJS += libretro.o

INCDIRS += -I.
INCDIRS += -I$(shell psp-config --pspsdk-path)/include
#INCDIRS += -I$(shell psp-config --psp-prefix)/include

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCDIRS)

clean-objs:
	rm -f $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

.PHONY: clean clean-objs

