/******************************************************************************

	memintrf.c

	CPS2・皈筵□、・□ソ・ユ・ァゥ`・ケ騅ハ□

******************************************************************************/

#include "cps2.h"
#include "rominfo.h"


#define M68K_AMASK 0x00ffffff
#define Z80_AMASK 0x0000ffff

#define READ_BYTE(mem, offset)			mem[offset ^ 1]
#define READ_WORD(mem, offset)			*(UINT16 *)&mem[offset]
#define WRITE_BYTE(mem, offset, data)	mem[offset ^ 1] = data
#define WRITE_WORD(mem, offset, data)	*(UINT16 *)&mem[offset] = data

#define str_cmp(s1, s2)		strnicmp(s1, s2, strlen(s2))

enum
{
	REGION_CPU1 = 0,
	REGION_CPU2,
	REGION_GFX1,
	REGION_SOUND1,
	REGION_USER1,
	REGION_SKIP
};

#define MAX_CPU1ROM		8
#define MAX_CPU2ROM		8
#define MAX_GFX1ROM		32
#define MAX_SND1ROM		8


/******************************************************************************
	・ー・□`・ミ・□萍□
******************************************************************************/

UINT8 *memory_region_cpu1;
UINT8 *memory_region_cpu2;
UINT8 *memory_region_gfx1;
UINT8 *memory_region_sound1;
UINT8 *memory_region_user1;

UINT32 memory_length_cpu1;
UINT32 memory_length_cpu2;
UINT32 memory_length_gfx1;
UINT32 memory_length_sound1;
UINT32 memory_length_user1;

UINT32 gfx_total_elements[3];
UINT8 *gfx_pen_usage[3];

UINT8  ALIGN_DATA cps1_ram[0x10000];
UINT8  ALIGN_DATA cps2_ram[0x4000 + 2];
UINT16 ALIGN_DATA cps1_gfxram[0x30000 >> 1];
UINT16 ALIGN_DATA cps1_output[0x100 >> 1];

UINT16 ALIGN_DATA cps2_objram[2][0x2000 >> 1];
UINT16 ALIGN_DATA cps2_output[0x10 >> 1];

UINT8 *qsound_sharedram1;
UINT8 *qsound_sharedram2;

#if !USE_CACHE
char cache_parent_name[16];
#endif


/******************************************************************************
	・□`・ォ・□欽□□我ハ□
******************************************************************************/

static struct rom_t cpu1rom[MAX_CPU1ROM];
static struct rom_t cpu2rom[MAX_CPU2ROM];
       struct rom_t gfx1rom[MAX_GFX1ROM];
static struct rom_t snd1rom[MAX_SND1ROM];

static int num_cpu1rom;
static int num_cpu2rom;
       int num_gfx1rom;
static int num_snd1rom;

static UINT8 *static_ram1;
static UINT8 *static_ram2;
static UINT8 *static_ram3;
static UINT8 *static_ram4;
static UINT8 *static_ram5;
static UINT8 *static_ram6;

#if !RELEASE
static int phoenix_edition;
#endif


/******************************************************************************
	ROMユi、゜゛z、゜
******************************************************************************/

/*--------------------------------------------------------
	CPU1 (M68000 program ROM / encrypted)
--------------------------------------------------------*/

static int load_rom_cpu1(void)
{
	int i, res;
	char fname[32], *parent;

	if ((memory_region_cpu1 = memalign(MEM_ALIGN, memory_length_cpu1)) == NULL)
	{
		error_memory("REGION_CPU1");
		return 0;
	}
	memset(memory_region_cpu1, 0, memory_length_cpu1);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_cpu1rom; )
	{
		strcpy(fname, cpu1rom[i].name);
		if ((res = file_open(game_name, parent, cpu1rom[i].crc, fname)) < 0)
		{
			if (res == -1)
				error_file(fname);
			else
				error_crc(fname);
			return 0;
		}

		msg_printf(TEXT(LOADING), fname);

		i = rom_load(cpu1rom, memory_region_cpu1, i, num_cpu1rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	CPU2 (Z80 program ROM)
--------------------------------------------------------*/

static int load_rom_cpu2(void)
{
	int i, res;
	char fname[32], *parent;

	if ((memory_region_cpu2 = memalign(MEM_ALIGN, memory_length_cpu2)) == NULL)
	{
		error_memory("REGION_CPU2");
		return 0;
	}
	memset(memory_region_cpu2, 0, memory_length_cpu2);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_cpu2rom; )
	{
		strcpy(fname, cpu2rom[i].name);
		if ((res = file_open(game_name, parent, cpu2rom[i].crc, fname)) < 0)
		{
			if (res == -1)
				error_file(fname);
			else
				error_crc(fname);
			return 0;
		}

		msg_printf(TEXT(LOADING), fname);

		i = rom_load(cpu2rom, memory_region_cpu2, i, num_cpu2rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	GFX1 (graphic ROM)
--------------------------------------------------------*/

static int load_rom_gfx1(void)
{
#if !USE_CACHE
	int i, res;
	char fname[32], *parent;
#endif

	gfx_total_elements[TILE08] = (memory_length_gfx1 - 0x800000) >> 6;
	gfx_total_elements[TILE16] = memory_length_gfx1 >> 7;
	gfx_total_elements[TILE32] = (memory_length_gfx1 - 0x800000) >> 9;

	if (gfx_total_elements[TILE08] > 0x10000) gfx_total_elements[TILE08] = 0x10000;
	if (gfx_total_elements[TILE32] > 0x10000) gfx_total_elements[TILE32] = 0x10000;

	if ((gfx_pen_usage[TILE08] = memalign(MEM_ALIGN, gfx_total_elements[TILE08])) == NULL)
	{
		error_memory("GFX_PEN_USAGE (tile8)");
		return 0;
	}
	if ((gfx_pen_usage[TILE16] = memalign(MEM_ALIGN, gfx_total_elements[TILE16])) == NULL)
	{
		error_memory("GFX_PEN_USAGE (tile16)");
		return 0;
	}
	if ((gfx_pen_usage[TILE32] = memalign(MEM_ALIGN, gfx_total_elements[TILE32])) == NULL)
	{
		error_memory("GFX_PEN_USAGE (tile32)");
		return 0;
	}

#if USE_CACHE
	memory_length_gfx1 = driver->cache_size;

	if (cache_start() == 0)
	{
		msg_printf(TEXT(PRESS_ANY_BUTTON2));
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}
#else
	memset(gfx_pen_usage[TILE08], 0, gfx_total_elements[TILE08]);
	memset(gfx_pen_usage[TILE16], 0, gfx_total_elements[TILE16]);
	memset(gfx_pen_usage[TILE32], 0, gfx_total_elements[TILE32]);

   if ((memory_region_gfx1 = memalign(MEM_ALIGN, memory_length_gfx1)) == NULL)
   {
      error_memory("REGION_GFX1");
      return 0;
   }

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_gfx1rom; )
	{
		strcpy(fname, gfx1rom[i].name);
		if ((res = file_open(game_name, parent, gfx1rom[i].crc, fname)) < 0)
		{
			if (res == -1)
				error_file(fname);
			else
				error_crc(fname);
			return 0;
		}

		msg_printf(TEXT(LOADING), fname);

		i = rom_load(gfx1rom, memory_region_gfx1, i, num_gfx1rom);

		file_close();
	}

	msg_printf(TEXT(DECODING_GFX), fname);
	cps2_gfx_decode();
#endif

	return 1;
}


/*--------------------------------------------------------
	SOUND1 (Q-SOUND PCM ROM)
--------------------------------------------------------*/

static int load_rom_sound1(void)
{
	int i, res;
	char fname[32], *parent;

	if ((memory_region_sound1 = memalign(MEM_ALIGN, memory_length_sound1)) == NULL)
	{
		error_memory("REGION_SOUND1");
		return 0;
	}
	memset(memory_region_sound1, 0, memory_length_sound1);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_snd1rom; )
	{
		strcpy(fname, snd1rom[i].name);
		if ((res = file_open(game_name, parent, snd1rom[i].crc, fname)) < 0)
		{
			if (res == -1)
				error_file(fname);
			else
				error_crc(fname);
			return 0;
		}

		msg_printf(TEXT(LOADING), fname);

		i = rom_load(snd1rom, memory_region_sound1, i, num_snd1rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	USER1 (MC68000 ROM decrypt region)
--------------------------------------------------------*/

static int load_rom_user1(void)
{
	if (memory_length_user1)
	{
		if ((memory_region_user1 = memalign(MEM_ALIGN, memory_length_user1)) == NULL)
		{
			error_memory("REGION_USER1");
			return 0;
		}
		memset(memory_region_user1, 0, memory_length_user1);
	}

	return 1;
}


/*--------------------------------------------------------
   ROMヌ驤□□ヌゥ`・ソ・ルゥ`・ケ、ヌス簧□
--------------------------------------------------------*/

int load_rom_info(const char *game_name)
{
   int i;

   rom_info_t* rom_info = cps2_rom_db;

   while (rom_info->game_name)
   {
      if (stricmp(rom_info->game_name, game_name) == 0)
      {
         strcpy(parent_name, rom_info->parent_name);

         machine_driver_type  = 0;
         machine_input_type   = rom_info->machine_input_type;
         machine_init_type    = rom_info->machine_init_type;
         machine_screen_type  = rom_info->machine_screen_type;

         memory_length_user1 = rom_info->memory_length_user1;

         num_cpu1rom = rom_info->num_cpu1rom;
         memory_length_cpu1 = rom_info->memory_length_cpu1;
         for (i = 0; i < rom_info->num_cpu1rom; i++)
         {
            cpu1rom[i].type   = rom_info->cpu1rom[i].type;
            cpu1rom[i].group  = rom_info->cpu1rom[i].group;
            cpu1rom[i].skip   = rom_info->cpu1rom[i].skip;
            cpu1rom[i].length = rom_info->cpu1rom[i].length;
            cpu1rom[i].offset = rom_info->cpu1rom[i].offset;
            cpu1rom[i].crc    = rom_info->cpu1rom[i].crc;
            strcpy(cpu1rom[i].name, rom_info->cpu1rom[i].name);
         }

         num_cpu2rom = rom_info->num_cpu2rom;
         memory_length_cpu2 = rom_info->memory_length_cpu2;
         for (i = 0; i < rom_info->num_cpu2rom; i++)
         {
            cpu2rom[i].type   = rom_info->cpu2rom[i].type;
            cpu2rom[i].group  = rom_info->cpu2rom[i].group;
            cpu2rom[i].skip   = rom_info->cpu2rom[i].skip;
            cpu2rom[i].length = rom_info->cpu2rom[i].length;
            cpu2rom[i].offset = rom_info->cpu2rom[i].offset;
            cpu2rom[i].crc    = rom_info->cpu2rom[i].crc;
            strcpy(cpu2rom[i].name, rom_info->cpu2rom[i].name);
         }

         num_gfx1rom = rom_info->num_gfx1rom;
         memory_length_gfx1 = rom_info->memory_length_gfx1;
         for (i = 0; i < rom_info->num_gfx1rom; i++)
         {
            gfx1rom[i].type   = rom_info->gfx1rom[i].type;
            gfx1rom[i].group  = rom_info->gfx1rom[i].group;
            gfx1rom[i].skip   = rom_info->gfx1rom[i].skip;
            gfx1rom[i].length = rom_info->gfx1rom[i].length;
            gfx1rom[i].offset = rom_info->gfx1rom[i].offset;
            gfx1rom[i].crc    = rom_info->gfx1rom[i].crc;
            strcpy(gfx1rom[i].name, rom_info->gfx1rom[i].name);
         }

         num_snd1rom = rom_info->num_snd1rom;
         memory_length_sound1 = rom_info->memory_length_sound1;
         for (i = 0; i < rom_info->num_snd1rom; i++)
         {
            snd1rom[i].type   = rom_info->snd1rom[i].type;
            snd1rom[i].group  = rom_info->snd1rom[i].group;
            snd1rom[i].skip   = rom_info->snd1rom[i].skip;
            snd1rom[i].length = rom_info->snd1rom[i].length;
            snd1rom[i].offset = rom_info->snd1rom[i].offset;
            snd1rom[i].crc    = rom_info->snd1rom[i].crc;
            strcpy(snd1rom[i].name, rom_info->snd1rom[i].name);
         }

         return 0;
      }

      rom_info++;
   }

   return 2;
}


/******************************************************************************
	・皈筵□、・□ソ・ユ・ァゥ`・ケ騅ハ□
******************************************************************************/

/*------------------------------------------------------
	・皈筵□、・□ソ・ユ・ァゥ`・ケウ□レサッ
-----------------------------------------------------*/

int memory_init(void)
{
	int i, res;

	memory_region_cpu1   = NULL;
	memory_region_cpu2   = NULL;
	memory_region_gfx1   = NULL;
	memory_region_sound1 = NULL;
	memory_region_user1  = NULL;

	memory_length_cpu1   = 0;
	memory_length_cpu2   = 0;
	memory_length_gfx1   = 0;
	memory_length_sound1 = 0;
	memory_length_user1  = 0;

	gfx_pen_usage[TILE08] = NULL;
	gfx_pen_usage[TILE16] = NULL;
	gfx_pen_usage[TILE32] = NULL;

#if USE_CACHE
	cache_init();
#endif
	pad_wait_clear();

	msg_printf(TEXT(CHECKING_ROM_INFO));

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: msg_printf(TEXT(THIS_GAME_NOT_SUPPORTED)); break;
		case 2: msg_printf(TEXT(ROM_NOT_FOUND)); break;
		case 3: msg_printf(TEXT(ROMINFO_NOT_FOUND)); break;
		}
		msg_printf(TEXT(PRESS_ANY_BUTTON2));
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (!strcmp(game_name, "ssf2ta")
	||	!strcmp(game_name, "ssf2tu")
	||	!strcmp(game_name, "ssf2tur1")
	||	!strcmp(game_name, "ssf2xj"))
	{
		strcpy(cache_parent_name, "ssf2t");
	}
	else if (!strcmp(game_name, "ssf2t"))
	{
		cache_parent_name[0] = '\0';
	}
#if USE_CACHE
	else if (!strcmp(game_name, "mpangj"))
	{
		// カ犢ヨネユアセユZー讀マBAD DUMP(メサイソ・ケ・ラ・鬣、・ネ、ホヌキ、ア、「、□
		cache_parent_name[0] = '\0';
	}
#endif
	else
	{
		strcpy(cache_parent_name, parent_name);
	}

	i = 0;
	driver = NULL;
	while (CPS2_driver[i].name)
	{
		if (!strcmp(game_name, CPS2_driver[i].name) || !strcmp(cache_parent_name, CPS2_driver[i].name))
		{
			driver = &CPS2_driver[i];
			break;
		}
		i++;
	}
	if (!driver)
	{
		msg_printf(TEXT(DRIVER_FOR_x_NOT_FOUND), game_name);
		msg_printf(TEXT(PRESS_ANY_BUTTON2));
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

#if !RELEASE
	if (!strcmp(game_name, "ddtodd")
	||	!strcmp(game_name, "ecofghtrd")
	||	!strcmp(game_name, "ssf2ud")
	||	!strcmp(game_name, "ssf2tbd")
	||	!strcmp(game_name, "armwar1d")
	||	!strcmp(game_name, "avspd")
	||	!strcmp(game_name, "dstlku1d")
	||	!strcmp(game_name, "ringdstd")
	||	!strcmp(game_name, "ssf2xjd")
	||	!strcmp(game_name, "xmcotar1d")
	||	!strcmp(game_name, "mshud")
	||	!strcmp(game_name, "cybotsud")
	||	!strcmp(game_name, "cybotsjd")
	||	!strcmp(game_name, "nwarrud")
	||	!strcmp(game_name, "sfad")
	||	!strcmp(game_name, "19xxd")
	||	!strcmp(game_name, "ddsomud")
	||	!strcmp(game_name, "gigaman2")
	||	!strcmp(game_name, "megamn2d")
	||	!strcmp(game_name, "sfz2ad")
	||	!strcmp(game_name, "sfz2jd")
	||	!strcmp(game_name, "spf2td")
	||	!strcmp(game_name, "spf2xjd")
	||	!strcmp(game_name, "sfz2ald")
	||	!strcmp(game_name, "xmvsfu1d")
	||	!strcmp(game_name, "batcird")
	||	!strcmp(game_name, "csclub1d")
	||	!strcmp(game_name, "mshvsfu1d")
	||	!strcmp(game_name, "sgemfd")
	||	!strcmp(game_name, "vsavd")
	||	!strcmp(game_name, "vhunt2d")
	||	!strcmp(game_name, "vsav2d")
	||	!strcmp(game_name, "mvscud")
	||	!strcmp(game_name, "sfa3ud")
	||	!strcmp(game_name, "sfz3jr2d")
	||	!strcmp(game_name, "gigawingd")
	||	!strcmp(game_name, "gigawingjd")
	||	!strcmp(game_name, "1944d")
	||	!strcmp(game_name, "dimahoud")
	||	!strcmp(game_name, "mmatrixd")
	||	!strcmp(game_name, "progearud")
	||	!strcmp(game_name, "progearjd")
	||	!strcmp(game_name, "progearjbl") // not an actual phoenix set, but works as one
	||	!strcmp(game_name, "hsf2d"))
		phoenix_edition = 1;
	else
		phoenix_edition = 0;
#endif

	if (parent_name[0])
		msg_printf(TEXT(ROMSET_x_PARENT_x), game_name, parent_name);
	else
		msg_printf(TEXT(ROMSET_x), game_name);

	if (load_rom_cpu1() == 0) return 0;
	if (load_rom_user1() == 0) return 0;
	if (load_rom_cpu2() == 0) return 0;
	if (option_sound_enable)
	{
		if (load_rom_sound1() == 0) return 0;
	}
	if (load_rom_gfx1() == 0) return 0;

	static_ram1 = (UINT8 *)cps1_ram    - 0xff0000;
	static_ram2 = (UINT8 *)cps1_gfxram - 0x900000;
	static_ram3 = (UINT8 *)cps2_ram    - 0x660000;
	static_ram4 = (UINT8 *)cps2_output - 0x400000;
	static_ram5 = (UINT8 *)cps2_objram[0];
	static_ram6 = (UINT8 *)cps2_objram[1];

	qsound_sharedram1 = &memory_region_cpu2[0xc000];
	qsound_sharedram2 = &memory_region_cpu2[0xf000];

	memory_region_cpu2[0xd007] = 0x80;

	return 1;
}


/*------------------------------------------------------
	・皈筵□、・□ソ・ユ・ァゥ`・ケスKチヒ
------------------------------------------------------*/

void memory_shutdown(void)
{
#if USE_CACHE
	cache_shutdown();
#endif

	if (gfx_pen_usage[TILE08]) free(gfx_pen_usage[TILE08]);
	if (gfx_pen_usage[TILE16]) free(gfx_pen_usage[TILE16]);
	if (gfx_pen_usage[TILE32]) free(gfx_pen_usage[TILE32]);

	if (memory_region_cpu1)   free(memory_region_cpu1);
	if (memory_region_cpu2)   free(memory_region_cpu2);
	if (memory_region_gfx1)   free(memory_region_gfx1);
	if (memory_region_sound1) free(memory_region_sound1);
	if (memory_region_user1)  free(memory_region_user1);

}


/******************************************************************************
	M68000 ・皈筵□□`・ノ/・鬣、・ネ騅ハ□
******************************************************************************/

/*------------------------------------------------------
	M68000・皈筵□□`・ノ (byte)
------------------------------------------------------*/

UINT8 m68000_read_memory_8(UINT32 offset)
{
	int shift;
	UINT16 mem_mask;

	offset &= M68K_AMASK;

	if (offset < memory_length_cpu1)
	{
		return READ_BYTE(memory_region_cpu1, offset);
	}

	shift = (~offset & 1) << 3;
	mem_mask = ~(0xff << shift);

	switch (offset >> 16)
	{
	case 0x40:
		return READ_BYTE(static_ram4, offset);

	case 0x61:
		return qsound_sharedram1_r(offset >> 1, mem_mask) >> shift;

	case 0x66:
		return READ_BYTE(static_ram3, offset);

	case 0x70:
		if (offset & 0x8000)
			return READ_BYTE(static_ram6, (offset & 0x1fff));
		else
			return READ_BYTE(static_ram5, (offset & 0x1fff));
		break;

	case 0x80:
		switch (offset & 0xff00)
		{
		case 0x0100:
		case 0x4100:
			return cps1_output_r(offset >> 1, mem_mask) >> shift;

		case 0x4000:
			switch (offset & 0xfe)
			{
			case 0x00: return cps2_inputport0_r(offset >> 1, mem_mask) >> shift;
			case 0x10: return cps2_inputport1_r(offset >> 1, mem_mask) >> shift;
			case 0x20: return cps2_eeprom_port_r(offset >> 1, mem_mask) >> shift;
			case 0x30: return cps2_qsound_volume_r(offset >> 1, mem_mask) >> shift;
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		return READ_BYTE(static_ram2, offset);

	case 0xff:
		return READ_BYTE(static_ram1, offset);
	}

	return 0xff;
}


/*------------------------------------------------------
	M68000・□`・ノ・皈筵□(word)
------------------------------------------------------*/

UINT16 m68000_read_memory_16(UINT32 offset)
{
	offset &= M68K_AMASK;

	if (offset < memory_length_cpu1)
	{
		return READ_WORD(memory_region_cpu1, offset);
	}

	switch (offset >> 16)
	{
	case 0x40:
		return READ_WORD(static_ram4, offset);

	case 0x61:
		return qsound_sharedram1_r(offset >> 1, 0);

	case 0x66:
		return READ_WORD(static_ram3, offset);

	case 0x70:
		if (offset & 0x8000)
			return READ_WORD(static_ram6, (offset & 0x1fff));
		else
			return READ_WORD(static_ram5, (offset & 0x1fff));
		break;

	case 0x80:
		switch (offset & 0xff00)
		{
		case 0x0100:
		case 0x4100:
			return cps1_output_r(offset >> 1, 0);

		case 0x4000:
			switch (offset & 0xfe)
			{
			case 0x00: return cps2_inputport0_r(offset >> 1, 0);
			case 0x10: return cps2_inputport1_r(offset >> 1, 0);
			case 0x20: return cps2_eeprom_port_r(offset >> 1, 0);
			case 0x30: return cps2_qsound_volume_r(offset >> 1, 0);
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		return READ_WORD(static_ram2, offset);

	case 0xff:
		return READ_WORD(static_ram1, offset);
	}

	return 0xffff;
}


/*------------------------------------------------------
	M68000・鬣、・ネ・皈筵□(byte)
------------------------------------------------------*/

void m68000_write_memory_8(UINT32 offset, UINT8 data)
{
	int shift = (~offset & 1) << 3;
	UINT16 mem_mask = ~(0xff << shift);

	offset &= M68K_AMASK;

	switch (offset >> 16)
	{
	case 0x40:
#if !RELEASE
		if (!phoenix_edition)
#endif
			WRITE_BYTE(static_ram4, offset, data);
		return;

	case 0x61:
		qsound_sharedram1_w(offset >> 1, data << shift, mem_mask);
		return;

	case 0x66:
		WRITE_BYTE(static_ram3, offset, data);
		return;

	case 0x70:
		if (offset & 0x8000)
			WRITE_BYTE(static_ram6, (offset & 0x1fff), data);
		else
			WRITE_BYTE(static_ram5, (offset & 0x1fff), data);
		return;

	case 0x80:
		switch (offset & 0xff00)
		{
		case 0x0100:
		case 0x4100:
			cps1_output_w(offset >> 1, data << shift, mem_mask);
			return;

		case 0x4000:
			switch (offset & 0xfe)
			{
			case 0x40:
				cps2_eeprom_port_w(offset >> 1, data << shift, mem_mask);
				return;

			case 0xe0:
				if (offset & 1)
				{
					cps2_objram_bank = data & 1;
					static_ram6 = (UINT8 *)cps2_objram[cps2_objram_bank ^ 1];
				}
				return;
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		WRITE_BYTE(static_ram2, offset, data);
		return;

	case 0xff:
#if !RELEASE
		if (phoenix_edition)
		{
			if (offset >= 0xfffff0)
			{
				offset -= 0xbffff0;
				WRITE_BYTE(static_ram4, offset, data);
				return;
			}
		}
#endif
		WRITE_BYTE(static_ram1, offset, data);
		return;
	}
}


/*------------------------------------------------------
	M68000・鬣、・ネ・皈筵□(word)
------------------------------------------------------*/

void m68000_write_memory_16(UINT32 offset, UINT16 data)
{
	offset &= M68K_AMASK;

	switch (offset >> 16)
	{
	case 0x40:
#if !RELEASE
		if (!phoenix_edition)
#endif
			WRITE_WORD(static_ram4, offset, data);
		return;

	case 0x61:
		qsound_sharedram1_w(offset >> 1, data, 0);
		return;

	case 0x66:
		WRITE_WORD(static_ram3, offset, data);
		return;

	case 0x70:
		if (offset & 0x8000)
			WRITE_WORD(static_ram6, (offset & 0x1fff), data);
		else
			WRITE_WORD(static_ram5, (offset & 0x1fff), data);
		break;

	case 0x80:
		switch (offset & 0xff00)
		{
		case 0x0100:
		case 0x4100:
			cps1_output_w(offset >> 1, data, 0);
			return;

		case 0x4000:
			switch (offset & 0xfe)
			{
			case 0x40:
				cps2_eeprom_port_w(offset >> 1, data, 0);
				return;

			case 0xe0:
				cps2_objram_bank = data & 1;
				static_ram6 = (UINT8 *)cps2_objram[cps2_objram_bank ^ 1];
				return;
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		WRITE_WORD(static_ram2, offset, data);
		return;

	case 0xff:
#if !RELEASE
		if (phoenix_edition)
		{
			if (offset >= 0xfffff0)
			{
				offset -= 0xbffff0;
				WRITE_WORD(static_ram4, offset, data);
				return;
			}
		}
#endif
		WRITE_WORD(static_ram1, offset, data);
		return;
	}
}


/******************************************************************************
	Z80 ・皈筵□□`・ノ/・鬣、・ネ騅ハ□
******************************************************************************/

/*------------------------------------------------------
	Z80・□`・ノ・皈筵□(byte)
------------------------------------------------------*/

UINT8 z80_read_memory_8(UINT32 offset)
{
	return memory_region_cpu2[offset & Z80_AMASK];
}


/*------------------------------------------------------
	Z80・鬣、・ネ・皈筵□(byte)
------------------------------------------------------*/

void z80_write_memory_8(UINT32 offset, UINT8 data)
{
	offset &= Z80_AMASK;

	switch (offset & 0xf000)
	{
	case 0xc000: case 0xf000:
		// c000-cfff: QSOUND shared RAM
		// f000-ffff: RAM
		memory_region_cpu2[offset] = data;
		break;

	case 0xd000:
		switch (offset)
		{
		case 0xd000: qsound_data_h_w(0, data); break;
		case 0xd001: qsound_data_l_w(0, data); break;
		case 0xd002: qsound_cmd_w(0, data); break;
		case 0xd003: qsound_banksw_w(0, data); break;
		}
		break;
	}
}


/******************************************************************************
	・サゥ`・ヨ/・□`・ノ ・ケ・ニゥ`・ネ
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( memory )
{
	state_save_byte(cps1_ram, 0x10000);
	state_save_byte(cps1_gfxram, 0x30000);
	state_save_byte(cps1_output, 0x100);
	state_save_byte(cps2_ram, 0x4002);
	state_save_byte(cps2_objram[0], 0x2000);
	state_save_byte(cps2_objram[1], 0x2000);
	state_save_byte(cps2_output, 0x10);
	state_save_byte(qsound_sharedram1, 0x1000);
	state_save_byte(qsound_sharedram2, 0x1000);
}

STATE_LOAD( memory )
{
	state_load_byte(cps1_ram, 0x10000);
	state_load_byte(cps1_gfxram, 0x30000);
	state_load_byte(cps1_output, 0x100);
	state_load_byte(cps2_ram, 0x4002);
	state_load_byte(cps2_objram[0], 0x2000);
	state_load_byte(cps2_objram[1], 0x2000);
	state_load_byte(cps2_output, 0x10);
	state_load_byte(qsound_sharedram1, 0x1000);
	state_load_byte(qsound_sharedram2, 0x1000);
}

#endif /* SAVE_STATE */
