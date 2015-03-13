/*****************************************************************************

   romcnv.c

   ROM converter for CPS2PSP

******************************************************************************/

#include "emumain.h"

#if USE_CACHE

#define DELIMITER "/"

#define SPRITE_BLANK    0x00
#define SPRITE_TRANSPARENT 0x02
#define SPRITE_OPAQUE      0x01

#define MAX_GFX1ROM        32


#define GFX1_BLOCK_SHIFT   21

#if (GFX1_BLOCK_SHIFT < 21)
#undef GFX1_BLOCK_SHIFT
#define GFX1_BLOCK_SHIFT  21
#endif

#define GFX1_BLOCK_SIZE    (1 << GFX1_BLOCK_SHIFT)

enum
{
   REGION_GFX1 = 0,
   REGION_SKIP
};

/******************************************************************************
   ローカル変数
******************************************************************************/

static char cache_name[16];
static FILE* cache_fp;

static int memory_gfx1_current_block;
extern struct rom_t gfx1rom[MAX_GFX1ROM];
extern int num_gfx1rom;

static UINT8 block_empty_local[0x200];

static UINT8 null_tile[128] =
{
   0x67, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x56,
   0x56, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x45,
   0x56, 0x51, 0x15, 0x51, 0x11, 0x15, 0x51, 0x45,
   0x56, 0x11, 0x15, 0x51, 0x11, 0x15, 0x51, 0x45,
   0x56, 0x11, 0x11, 0x51, 0x11, 0x15, 0x51, 0x45,
   0x56, 0x11, 0x15, 0x51, 0x11, 0x15, 0x51, 0x45,
   0x56, 0x11, 0x55, 0x51, 0x11, 0x11, 0x51, 0x45,
   0x56, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x45,
   0x56, 0x11, 0x55, 0x55, 0x11, 0x55, 0x55, 0x45,
   0x56, 0x11, 0x55, 0x55, 0x11, 0x55, 0x55, 0x45,
   0x56, 0x11, 0x55, 0x55, 0x11, 0x55, 0x55, 0x45,
   0x56, 0x11, 0x55, 0x55, 0x11, 0x55, 0x55, 0x45,
   0x56, 0x11, 0x11, 0x51, 0x11, 0x11, 0x51, 0x45,
   0x56, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x45,
   0x56, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x45,
   0x45, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x34
};

static UINT8 blank_tile[128] =
{
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xff,
   0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1,
   0x1f, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xf1, 0xf1,
   0x1f, 0xff, 0xff, 0xff, 0xf1, 0x1f, 0x1f, 0xf1,
   0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xf1,
   0x1f, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xf1,
   0x1f, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xf1,
   0x1f, 0xff, 0xf1, 0xff, 0xf1, 0x1f, 0xff, 0xf1,
   0x1f, 0x1f, 0xff, 0xff, 0xf1, 0xff, 0xf1, 0xf1,
   0x1f, 0x1f, 0xff, 0xff, 0x1f, 0xff, 0xf1, 0xf1,
   0x1f, 0x1f, 0x1f, 0xff, 0x1f, 0xff, 0xf1, 0xf1,
   0x1f, 0xff, 0xff, 0x11, 0xf1, 0xff, 0xff, 0xf1,
   0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1,
   0xff, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


struct cacheinfo_t
{
   const char* name;
   UINT32  zip;
   UINT32  object_start;
   UINT32  object_end;
   UINT32  scroll1_start;
   UINT32  scroll1_end;
   UINT32  scroll2_start;
   UINT32  scroll2_end;
   UINT32  scroll3_start;
   UINT32  scroll3_end;
   UINT32  object2_start;
   UINT32  object2_end;
};

struct cacheinfo_t* cacheinfo;

struct cacheinfo_t CPS2_cacheinfo[] =
{
   //    name           object              scroll1             scroll2             scroll3             object/scroll2
   { "ssf2",     0, 0x000000, 0x7fffff, 0x800000, 0x88ffff, 0x900000, 0xabffff, 0xac0000, 0xbbffff, 0,         0         },
   { "ddtod",    0, 0x000000, 0x7fffff, 0x800000, 0x8fffff, 0x900000, 0xafffff, 0xac0000, 0xbfffff, 0,         0         },
   { "ecofghtr", 1, 0x000000, 0x7fffff, 0x800000, 0x83ffff, 0x880000, 0x99ffff, 0xa00000, 0xabffff, 0,         0         },
   { "ssf2t",    0, 0x000000, 0x7fffff, 0x800000, 0x88ffff, 0x900000, 0xabffff, 0xac0000, 0xffffff, 0,         0         },
   { "xmcota",   0, 0x000000, 0x7dffff, 0x800000, 0x8dffff, 0xb00000, 0xfdffff, 0x8e0000, 0xafffff, 0x1000000, 0x1ffffff },
   { "armwar",   0, 0x000000, 0x7fffff, 0x800000, 0x85ffff, 0x860000, 0x9bffff, 0x9c0000, 0xa5ffff, 0xa60000,  0x12fffff },
   { "avsp",     1, 0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0x9fffff, 0xa00000, 0xafffff, 0,         0         },
   { "dstlk",    0, 0x000000, 0x7cffff, 0x800000, 0x87ffff, 0x880000, 0x9bffff, 0x9c0000, 0xabffff, 0xac0000,  0x13fffff },
   { "ringdest", 0, 0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0x9fffff, 0xac0000, 0xcfffff, 0xd40000,  0x11fffff },
   { "cybots",   0, 0x000000, 0x7dffff, 0x800000, 0x8bffff, 0x8c0000, 0xb3ffff, 0xb40000, 0xcbffff, 0xcc0000,  0x1ffffff },
   { "msh",      0, 0x000000, 0x7fffff, 0x800000, 0x8cffff, 0xb00000, 0xffffff, 0x8e0000, 0xafffff, 0x1000000, 0x1ffffff },
   { "nwarr",    0, 0x000000, 0x7cffff, 0x800000, 0x87ffff, 0x880000, 0x9bffff, 0x9c0000, 0xabffff, 0xac0000,  0x1f8ffff },
   { "sfa",      1, 0x000000, 0x000000, 0x800000, 0x81ffff, 0x820000, 0xf8ffff, 0xfa0000, 0xfeffff, 0,         0         },
   { "mmancp2u", 1, 0x000000, 0x000000, 0x800000, 0x85ffff, 0x860000, 0xe6ffff, 0xe80000, 0xfeffff, 0,         0         },
   { "19xx",     1, 0x000000, 0x16ffff, 0x800000, 0x83ffff, 0x840000, 0x9bffff, 0x9c0000, 0xafffff, 0xb00000,  0xffffff  },
   { "ddsom",    0, 0x000000, 0x7dffff, 0x800000, 0x8bffff, 0x8c0000, 0xbdffff, 0xbe0000, 0xdbffff, 0xde0000,  0x179ffff },
   { "megaman2", 0, 0x000000, 0x000000, 0x800000, 0x85ffff, 0x860000, 0xecffff, 0xee0000, 0xffffff, 0,         0         },
   { "qndream",  1, 0x000000, 0x000000, 0x800000, 0x81ffff, 0x840000, 0xefffff, 0x820000, 0x83ffff, 0,         0         },
   { "sfa2",     0, 0x000000, 0x79ffff, 0x800000, 0x91ffff, 0xa40000, 0xccffff, 0x920000, 0xa3ffff, 0xd20000,  0x138ffff },
   { "spf2t",    1, 0x000000, 0x000000, 0x800000, 0x82ffff, 0x840000, 0xb8ffff, 0xb90000, 0xbcffff, 0,         0         },
   { "xmvsf",    0, 0x000000, 0x7effff, 0x800000, 0x8fffff, 0xaa0000, 0xffffff, 0x900000, 0xa7ffff, 0x1000000, 0x1ffffff },
   { "batcir",   0, 0x000000, 0x7dffff, 0x800000, 0x817fff, 0x818000, 0x937fff, 0x938000, 0xa3ffff, 0xa48000,  0xd8ffff  },
   { "csclub",   1, 0x000000, 0x000000, 0x8c0000, 0x8fffff, 0x900000, 0xffffff, 0x800000, 0x8bffff, 0,         0         },
   { "mshvsf",   0, 0x000000, 0x7fffff, 0x800000, 0x8dffff, 0xa80000, 0xfeffff, 0x8e0000, 0xa6ffff, 0x1000000, 0x1feffff },
   { "sgemf",    0, 0x000000, 0x7fffff, 0x800000, 0x8d1fff, 0xa22000, 0xfdffff, 0x8d2000, 0xa21fff, 0x1000000, 0x13fffff },
   { "vhunt2",   0, 0x000000, 0x7affff, 0x800000, 0x8affff, 0xa10000, 0xfdffff, 0x8c0000, 0xa0ffff, 0x1000000, 0x1fdffff },
   { "vsav",     0, 0x000000, 0x7fffff, 0x800000, 0x8bffff, 0x9c0000, 0xffffff, 0x8c0000, 0x9bffff, 0x1000000, 0x1feffff },
   { "vsav2",    0, 0x000000, 0x7fffff, 0x800000, 0x8affff, 0xa10000, 0xfdffff, 0x8c0000, 0xa0ffff, 0x1000000, 0x1fdffff },
   { "mvsc",     0, 0x000000, 0x7cffff, 0x800000, 0x91ffff, 0xb40000, 0xd0ffff, 0x920000, 0xb2ffff, 0xd20000,  0x1feffff },
   { "sfa3",     0, 0x000000, 0x7dffff, 0x800000, 0x95ffff, 0xb60000, 0xffffff, 0x960000, 0xb5ffff, 0x1000000, 0x1fcffff },
   { "jyangoku", 1, 0x000000, 0x7fffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0,         0         },
   { "hsf2",     0, 0x000000, 0x7fffff, 0x800000, 0x1ffffff, 0x800000, 0x1ffffff, 0x800000, 0x1ffffff, 0,         0         },
   { "gigawing", 0, 0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0xa7ffff, 0xa80000, 0xdcffff, 0xe00000,  0xffffff  },
   { "mmatrix",  0, 0x000000, 0x7fffff, 0x800000, 0x8fffff, 0x800000, 0xd677ff, 0x800000, 0xd677ff, 0x1000000, 0x1ffffff },
   { "mpangj",   1, 0x000000, 0x000000, 0x800000, 0x82ffff, 0x840000, 0x9dffff, 0xa00000, 0xbdffff, 0xc00000,  0xffffff  },
   { "mpang",    1, 0x000000, 0x000000, 0x800000, 0x82ffff, 0x840000, 0x9dffff, 0xa00000, 0xbdffff, 0xc00000,  0xffffff  },
   { "pzloop2",  1, 0x000000, 0x81ffff, 0x800000, 0x97ffff, 0xa00000, 0xc8ffff, 0xd80000, 0xebffff, 0,         0         },
   { "choko",    1, 0x000000, 0x000000, 0x800000, 0xffffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0,         0         },
   { "dimahoo",  0, 0x000000, 0x7fffff, 0x800000, 0x8bffff, 0xb80000, 0xffffff, 0x8e0000, 0xb6ffff, 0,         0         },
   { "1944",     0, 0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0xcdffff, 0xd00000, 0xfeffff, 0x1000000, 0x13bffff },
   { "progear",  0, 0x000000, 0x7fffff, 0x800000, 0xa0afff, 0xa0b000, 0xd86fff, 0xd87000, 0xffffff, 0,         0         },
   { NULL }
};

static inline int str_cmp(const char* s1, const char* s2)
{
   return strncasecmp(s1, s2, strlen(s2));
}


/******************************************************************************
   CPS2用関数
******************************************************************************/

static void unshuffle(UINT64* buf, int len)
{
   int i;
   UINT64 t;

   if (len == 2) return;

   len /= 2;

   unshuffle(buf, len);
   unshuffle(buf + len, len);

   for (i = 0; i < len / 2; i++)
   {
      t = buf[len / 2 + i];
      buf[len / 2 + i] = buf[len + i];
      buf[len + i] = t;
   }
}


static void cps2_gfx_decode(void)
{
   int i, j;
   UINT8* gfx = memory_region_gfx1;


   for (i = 0; i < GFX1_BLOCK_SIZE; i += 0x200000)
      unshuffle((UINT64*)&memory_region_gfx1[i], 0x200000 / 8);

   for (i = 0; i < GFX1_BLOCK_SIZE / 4; i++)
   {
      UINT32 src = gfx[4 * i] + (gfx[4 * i + 1] << 8) + (gfx[4 * i + 2] << 16) +
                   (gfx[4 * i + 3] << 24);
      UINT32 dw = 0, data;

      for (j = 0; j < 8; j++)
      {
         int n = 0;
         UINT32 mask = (0x80808080 >> j) & src;

         if (mask & 0x000000ff) n |= 1;
         if (mask & 0x0000ff00) n |= 2;
         if (mask & 0x00ff0000) n |= 4;
         if (mask & 0xff000000) n |= 8;

         dw |= n << (j * 4);
      }

      data = ((dw & 0x0000000f) >>  0) | ((dw & 0x000000f0) <<  4)
             | ((dw & 0x00000f00) <<  8) | ((dw & 0x0000f000) << 12)
             | ((dw & 0x000f0000) >> 12) | ((dw & 0x00f00000) >>  8)
             | ((dw & 0x0f000000) >>  4) | ((dw & 0xf0000000) >>  0);

      gfx[4 * i + 0] = data >>  0;
      gfx[4 * i + 1] = data >>  8;
      gfx[4 * i + 2] = data >> 16;
      gfx[4 * i + 3] = data >> 24;
   }
}

static void clear_empty_blocks_init(void)
{
   memset(block_empty_local, 1, 0x200);
}

#define min(A,B) ((A)<(B)?(A):(B))
#define max(A,B) ((A)>(B)?(A):(B))

static void clear_empty_blocks_current_block(void)
{
   UINT32 i, j;
   UINT8 temp[512];
   //   int size, blocks_available = 0;

   UINT32 start = ((memory_gfx1_current_block) << GFX1_BLOCK_SHIFT);
   UINT32 end = ((memory_gfx1_current_block + 1) << GFX1_BLOCK_SHIFT);

   UINT32 start_b = ((memory_gfx1_current_block) << (GFX1_BLOCK_SHIFT - 16));
   UINT32 end_b = ((memory_gfx1_current_block + 1) << (GFX1_BLOCK_SHIFT - 16));


   for (i = 0; i < GFX1_BLOCK_SIZE; i += 128)
   {
      if (memcmp(&memory_region_gfx1[i], null_tile, 128) == 0
            || memcmp(&memory_region_gfx1[i], blank_tile, 128) == 0)
         memset(&memory_region_gfx1[i], 0xff, 128);
   }

   if (!strcmp(cacheinfo->name, "avsp"))
   {
      for (i = max(0xb0, start_b); i < min(0x100, end_b); i++)
         memset(&memory_region_gfx1[(i << 16) - start], 0xff, 0x10000);
   }
   else if (!strcmp(cacheinfo->name, "ddtod"))
   {
      memcpy(temp, &memory_region_gfx1[0x5be800], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x657a00], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x707800], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x710b80], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x77d080], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x780000], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x7b5580], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x7d7800], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x93bd00], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0x9a5380], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0xa3eb80], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0xa70300], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0xa84f00], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
      memcpy(temp, &memory_region_gfx1[0xb75000], 512);
      for (i = 0; i < memory_length_gfx1; i += 512)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
            memset(&memory_region_gfx1[i], 0xff, 512);
      }
      memcpy(temp, &memory_region_gfx1[0xb90600], 512);
      for (i = 0; i < memory_length_gfx1; i += 512)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
            memset(&memory_region_gfx1[i], 0xff, 512);
      }
      memcpy(temp, &memory_region_gfx1[0xbcb200], 512);
      for (i = 0; i < memory_length_gfx1; i += 512)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
            memset(&memory_region_gfx1[i], 0xff, 512);
      }
      memcpy(temp, &memory_region_gfx1[0xbd0000], 512);
      for (i = 0; i < memory_length_gfx1; i += 512)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
            memset(&memory_region_gfx1[i], 0xff, 512);
      }
   }
   else if (!strcmp(cacheinfo->name, "dstlk") || !strcmp(cacheinfo->name, "nwarr"))
   {
      for (i = 0x7d; i <= 0x7f; i++)
         memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xff0000 + (16 * 29) * 128], 0xff,
             0x10000 - (16 * 29) * 128);
      memset(&memory_region_gfx1[0x13f0000 + (16 * 11) * 128], 0xff,
             0x10000 - (16 * 11) * 128);

      memcpy(temp, &memory_region_gfx1[0x10000], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }
   }
   else if (!strcmp(cacheinfo->name, "ringdest"))
   {
      for (i = 0xa0; i <= 0xab; i++)
         memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0xd0; i <= 0xd3; i++)
         memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
   }
   else if (!strcmp(cacheinfo->name, "mpang")
            || !strcmp(cacheinfo->name, "mpangj"))
   {
      memset(&memory_region_gfx1[0x820000 + 16 * 11 * 128], 0xff, 16 * 21 * 128);
      memset(&memory_region_gfx1[0x830000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0x840000 + (16 * 31 + 13) * 128], 0xff,
             0x10000 - (16 * 31 + 13) * 128);
      memset(&memory_region_gfx1[0x850000], 0xff, 16 * 16 * 128);
      memset(&memory_region_gfx1[0x9d0000 + (16 * 22 + 13) * 128], 0xff,
             0x10000 - (16 * 22 + 13) * 128);
      memset(&memory_region_gfx1[0x9e0000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0x9f0000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xbd0000 + (16 * 4 + 8) * 128], 0xff,
             0x10000 - (16 * 4 + 8) * 128);
      memset(&memory_region_gfx1[0xbe0000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xbf0000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xd50000 + (16 * 12) * 128], 0xff,
             0x10000 - (16 * 12) * 128);
      memset(&memory_region_gfx1[0xd60000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xd70000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xdf0000 + (16 * 24) * 128], 0xff,
             0x10000 - (16 * 24) * 128);
      memset(&memory_region_gfx1[0xef0000 + (16 * 31) * 128], 0xff,
             0x10000 - (16 * 31) * 128);
      memset(&memory_region_gfx1[0xfb0000 + (16 * 14) * 128], 0xff,
             0x10000 - (16 * 14) * 128);
      memset(&memory_region_gfx1[0xff0000 + (16 * 12) * 128], 0xff,
             0x10000 - (16 * 12) * 128);
   }
   else if (!strcmp(cacheinfo->name, "mmatrix"))
   {
      memset(&memory_region_gfx1[0xd67600], 0xff, (16 * 17 + 4) * 128);
      for (i = 0xd7; i <= 0xff; i++)
         memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
   }
   else if (!strcmp(cacheinfo->name, "pzloop2"))
   {
      memset(&memory_region_gfx1[0x170000 + 16 * 16 * 128], 0xff, 16 * 16 * 128);
      memset(&memory_region_gfx1[0x1c0000 + 16 * 9 * 128], 0xff, 16 * 23 * 128);
      memset(&memory_region_gfx1[0x230000 + 16 * 7 * 128], 0xff, 16 * 25 * 128);
      memset(&memory_region_gfx1[0x270000 + 16 * 17 * 128], 0xff, 16 * 15 * 128);
      memset(&memory_region_gfx1[0x290000 + 16 * 23 * 128], 0xff, 16 * 9 * 128);
      memset(&memory_region_gfx1[0x2d0000 + 16 * 21 * 128], 0xff, 16 * 11 * 128);
      memset(&memory_region_gfx1[0x390000 + 16 * 30 * 128], 0xff, 16 * 2 * 128);
      memset(&memory_region_gfx1[0x410000 + 16 * 17 * 128], 0xff, 16 * 15 * 128);
      memset(&memory_region_gfx1[0x530000 + 16 * 6 * 128], 0xff, 16 * 26 * 128);
      memset(&memory_region_gfx1[0x590000 + 16 * 4 * 128], 0xff, 16 * 28 * 128);
      memset(&memory_region_gfx1[0x670000 + 16 * 9 * 128], 0xff, 16 * 23 * 128);
      memset(&memory_region_gfx1[0x730000 + 16 * 12 * 128], 0xff, 16 * 20 * 128);
      memset(&memory_region_gfx1[0x7a0000 + 16 * 10 * 128], 0xff, 16 * 22 * 128);
      memset(&memory_region_gfx1[0x802000 + 2 * 128], 0xff, 14 * 128);
      memset(&memory_region_gfx1[0x806800 + 4 * 128], 0xff, 12 * 128);
      memset(&memory_region_gfx1[0x810000 + 16 * 19 * 128 + 128], 0xff,
             16 * 13 * 128 - 128);
      memset(&memory_region_gfx1[0xc80000 + 11 * 128], 0xff, 0x10000 - 11 * 128);
      memset(&memory_region_gfx1[0x970000 + (16 * 27 + 11) * 128], 0xff,
             0x10000 - (16 * 17 + 11) * 128);
      memset(&memory_region_gfx1[0xeb0000 + (16 * 2 + 9) * 512], 0xff,
             0x10000 - (16 * 2 + 9) * 512);

      for (i = 0x1d; i <= 0x1f;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x2a; i <= 0x2b;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x3a; i <= 0x3f;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x42; i <= 0x47;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x54; i <= 0x57;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x5a; i <= 0x5f;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x74; i <= 0x77;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x7b; i <= 0x7f;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x82; i <= 0x87;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0x98; i <= 0x9f;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0xc9; i <= 0xd7;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
      for (i = 0xec; i <= 0xff;
            i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
   }
   else if (!strcmp(cacheinfo->name, "1944"))
   {
      for (i = 0x140; i <= 0x1ff; i++)
         memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
   }
   else if (!strcmp(cacheinfo->name, "choko"))
   {
      memcpy(temp, &memory_region_gfx1[0xa60000 + 128], 128);
      for (i = 0; i < memory_length_gfx1; i += 128)
      {
         if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
            memset(&memory_region_gfx1[i], 0xff, 128);
      }

      memset(memory_region_gfx1, 0xff, 0x800000);
      memset(&memory_region_gfx1[0x860000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0x870000], 0xff, 0x10000);

      memset(&memory_region_gfx1[0xa60000 + 128 * 16], 0xff, 0x10000 - 128 * 16);
      for (i = 0xa70000; i < 0xb00000; i += 0x10000)
         memset(&memory_region_gfx1[i], 0xff, 0x10000);

      memset(&memory_region_gfx1[0xc00000 + 128 * 16 * 3], 0xff,
             0x10000 - 128 * 16 * 3);
      for (i = 0xc10000; i < 0xd00000; i += 0x10000)
         memset(&memory_region_gfx1[i], 0xff, 0x10000);

      memset(&memory_region_gfx1[0xfa0000 + 128 * 16 * 12], 0xff,
             0x10000 - 128 * 16 * 12);
      memset(&memory_region_gfx1[0xfb0000], 0xff, 0x10000);

      memset(&memory_region_gfx1[0xfd0000 + 128 * 16 * 17 + 128 * 5], 0xff,
             0x10000 - (128 * 16 * 17 + 128 * 5));
      memset(&memory_region_gfx1[0xfe0000], 0xff, 0x10000);
      memset(&memory_region_gfx1[0xff0000], 0xff, 0x10000);
   }
   else if (!strcmp(cacheinfo->name, "jyangoku"))
      for (i = start; i < min(0x800000, end); i += 0x200000)
         memset(&memory_region_gfx1[i - start], 0xff, 0x200000);

   if (cacheinfo->object_end == 0)
      for (i = start; i < min(0x800000, end); i += 0x200000)
         memset(&memory_region_gfx1[i - start], 0xff, 0x200000);
   else if (cacheinfo->object_end != 0x7fffff)
   {
      for (i = max(cacheinfo->object_end + 1, start); i < min(0x800000, end);
            i += 0x10000)
         memset(&memory_region_gfx1[i - start], 0xff, 0x10000);
   }

   for (i = start_b; i < min((memory_length_gfx1 >> 16), end_b); i++)
   {
      int empty = 1;
      UINT32 offset = i << 16;

      for (j = 0; j < 0x10000; j++)
      {
         if (memory_region_gfx1[offset + j - start] != 0xff)
         {
            empty = 0;
            break;
         }
      }

      block_empty_local[i] = empty;
   }
   //   for (; i < 0x200; i++)
   //      block_empty_local[i] = 1;

   //   for (i = 0; i < (memory_length_gfx1 >> 16); i++)
   //   {
   //      if (!block_empty_local[i]) blocks_available++;
   //   }
   // msg_printf("cache required size = %x\n", blocks_available << 16);

   //   size = blocks_available << 16;
   //   if (size != memory_length_gfx1)
   //      msg_printf("remove empty tiles (total size: %d bytes -> %d bytes)\n",
   //             memory_length_gfx1, size);
}


static int calc_pen_usage_init(void)
{
   gfx_total_elements[TILE08] = (memory_length_gfx1 - 0x800000) >> 6;
   gfx_total_elements[TILE16] = memory_length_gfx1 >> 7;
   gfx_total_elements[TILE32] = (memory_length_gfx1 - 0x800000) >> 9;

   if (gfx_total_elements[TILE08] >= 0x10000) gfx_total_elements[TILE08] = 0x10000;
   if (gfx_total_elements[TILE32] >= 0x10000) gfx_total_elements[TILE32] = 0x10000;

   gfx_pen_usage[TILE08] = malloc(gfx_total_elements[TILE08]);
   gfx_pen_usage[TILE16] = malloc(gfx_total_elements[TILE16]);
   gfx_pen_usage[TILE32] = malloc(gfx_total_elements[TILE32]);


   if (!gfx_pen_usage[TILE08] || !gfx_pen_usage[TILE16] || !gfx_pen_usage[TILE32])
   {
      msg_printf("ERROR: Could not allocate memory.\n");
      return 0;
   }

   memset(gfx_pen_usage[TILE08], 0, gfx_total_elements[TILE08]);
   memset(gfx_pen_usage[TILE16], 0, gfx_total_elements[TILE16]);
   memset(gfx_pen_usage[TILE32], 0, gfx_total_elements[TILE32]);

   return 1;
}

static void calc_pen_usage(void)
{
   int i, j, k, start, end;
   UINT32* tile, data;
   UINT32 s0 = cacheinfo->object_start;
   UINT32 e0 = cacheinfo->object_end;
   UINT32 s1 = cacheinfo->scroll1_start;
   UINT32 e1 = cacheinfo->scroll1_end;
   UINT32 s2 = cacheinfo->scroll2_start;
   UINT32 e2 = cacheinfo->scroll2_end;
   UINT32 s3 = cacheinfo->scroll3_start;
   UINT32 e3 = cacheinfo->scroll3_end;
   UINT32 s4 = cacheinfo->object2_start;
   UINT32 e4 = cacheinfo->object2_end;

   start = 0;
   if (((0x20000 + start) << 6) < (memory_gfx1_current_block << GFX1_BLOCK_SHIFT))
      start = (memory_gfx1_current_block << (GFX1_BLOCK_SHIFT - 6)) - 0x20000;

   end = gfx_total_elements[TILE08];
   if (((0x20000 + end) << 6) > ((memory_gfx1_current_block + 1) <<
                                 GFX1_BLOCK_SHIFT))
      end = ((memory_gfx1_current_block + 1) << (GFX1_BLOCK_SHIFT - 6)) - 0x20000;

   for (i = start; i < end; i++)
   {
      int count = 0;
      UINT32 offset = (0x20000 + i) << 6;
      UINT32 s5 = 0x000000;
      UINT32 e5 = 0x000000;

      if (!strcmp(cacheinfo->name, "pzloop2"))
      {
         s5 = 0x802800;
         e5 = 0x87ffff;
      }

      if ((offset >= s1 && offset <= e1) && !(offset >= s5 && offset <= e5))
      {
         tile = (UINT32*)&memory_region_gfx1[offset - (memory_gfx1_current_block <<
                                             GFX1_BLOCK_SHIFT)];

         for (j = 0; j < 8; j++)
         {
            tile++;
            data = *tile++;
            for (k = 0; k < 8; k++)
            {
               if ((data & 0x0f) == 0x0f)
                  count++;
               data >>= 4;
            }
         }
         if (count == 0)
            gfx_pen_usage[TILE08][i] = SPRITE_OPAQUE;
         else if (count != 8 * 8)
            gfx_pen_usage[TILE08][i] = SPRITE_TRANSPARENT;
      }
   }


   start = 0;
   if ((start << 7) < (memory_gfx1_current_block << GFX1_BLOCK_SHIFT))
      start = memory_gfx1_current_block << (GFX1_BLOCK_SHIFT - 7);

   end = gfx_total_elements[TILE16];
   if ((end << 7) > ((memory_gfx1_current_block + 1) << GFX1_BLOCK_SHIFT))
      end = (memory_gfx1_current_block + 1) << (GFX1_BLOCK_SHIFT - 7);

   for (i = start; i < end; i++)
   {
      UINT32 s5 = 0;
      UINT32 e5 = 0;
      UINT32 offset = i << 7;

      if (!strcmp(cacheinfo->name, "ssf2t"))
      {
         s5 = 0xc00000;
         e5 = 0xfaffff;
      }
      else if (!strcmp(cacheinfo->name, "gigawing"))
      {
         s5 = 0xc00000;
         e5 = 0xc7ffff;
      }
      else if (!strcmp(cacheinfo->name, "progear"))
      {
         s5 = 0xf27000;
         e5 = 0xf86fff;
      }

      if ((offset >= s0 && offset <= e0)
            || (offset >= s2 && offset <= e2)
            || (offset >= s4 && offset <= e4)
            || (offset >= s5 && offset <= e5))
      {
         int count = 0;

         tile = (UINT32*)&memory_region_gfx1[offset - (memory_gfx1_current_block <<
                                             GFX1_BLOCK_SHIFT)];

         for (j = 0; j < 2 * 16; j++)
         {
            data = *tile++;
            for (k = 0; k < 8; k++)
            {
               if ((data & 0x0f) == 0x0f)
                  count++;
               data >>= 4;
            }
         }
         if (count == 0)
            gfx_pen_usage[TILE16][i] = SPRITE_OPAQUE;
         else if (count != 2 * 16 * 8)
            gfx_pen_usage[TILE16][i] = SPRITE_TRANSPARENT;
      }
   }

   start = 0;
   if (((0x4000 + start) << 9) < (memory_gfx1_current_block << GFX1_BLOCK_SHIFT))
      start = (memory_gfx1_current_block << (GFX1_BLOCK_SHIFT - 9)) - 0x4000;

   end = gfx_total_elements[TILE32];
   if (((0x4000 + end) << 9) > ((memory_gfx1_current_block + 1) <<
                                GFX1_BLOCK_SHIFT))
      end = ((memory_gfx1_current_block + 1) << (GFX1_BLOCK_SHIFT - 9)) - 0x4000;

   for (i = start; i < end; i++)
   {
      int count  = 0;
      UINT32 offset = (0x4000 + i) << 9;

      if (!strcmp(cacheinfo->name, "ssf2t"))
      {
         if (offset >= 0xc00000 && offset <= 0xfaffff)
            continue;
      }
      else if (!strcmp(cacheinfo->name, "gigawing"))
      {
         if (offset >= 0xc00000 && offset <= 0xc7ffff)
            continue;
      }
      else if (!strcmp(cacheinfo->name, "progear"))
      {
         if (offset >= 0xf27000 && offset <= 0xf86fff)
            continue;
      }

      if (offset >= s3 && offset <= e3)
      {
         tile = (UINT32*)&memory_region_gfx1[offset - (memory_gfx1_current_block <<
                                             GFX1_BLOCK_SHIFT)];

         for (j = 0; j < 4 * 32; j++)
         {
            data = *tile++;
            for (k = 0; k < 8; k++)
            {
               if ((data & 0x0f) == 0x0f)
                  count++;
               data >>= 4;
            }
         }
         if (count == 0)
            gfx_pen_usage[TILE32][i] = SPRITE_OPAQUE;
         else if (count != 4 * 32 * 8)
            gfx_pen_usage[TILE32][i] = SPRITE_TRANSPARENT;
      }
   }
}

static int rom_load_current_block(struct rom_t* rom, UINT8* mem, int idx,
                                  int max)
{
   int offset, length;

   UINT32 start = ((memory_gfx1_current_block) << GFX1_BLOCK_SHIFT);
   UINT32 end = ((memory_gfx1_current_block + 1) << GFX1_BLOCK_SHIFT);

_continue:
   offset = rom[idx].offset;
   int rom_start, rom_end;

   if (rom[idx].offset > end)
      return idx + 1;

   if (rom[idx].skip == 0)
   {
      rom_start = 0;
      if (start > rom[idx].offset)
         rom_start = start - rom[idx].offset;

      rom_end = rom[idx].length;
      if (rom[idx].length > (end - rom[idx].offset))
         rom_end = (end - rom[idx].offset);

      if (rom_start >= rom_end)
         return idx + 1;

      file_skip_bytes(rom_start);
      file_read(mem, rom_end - rom_start);

      if (rom[idx].type == ROM_WORDSWAP)
         swab(mem, mem, rom_end - rom_start);
   }
   else
   {
      int c;
      int skip = rom[idx].skip + rom[idx].group;

      rom_start = 0;
      if (start > (rom[idx].offset & ~0xf))
         rom_start = (start - (rom[idx].offset & ~0xf)) / (skip / rom[idx].group);

      rom_end = rom[idx].length;
      if (rom[idx].length > ((end - (rom[idx].offset & ~0xf)) /
                             (skip / rom[idx].group)))
         rom_end = (end - (rom[idx].offset & ~0xf)) / (skip / rom[idx].group);

      if (rom_start >= rom_end)
         return idx + 1;

      file_skip_bytes(rom_start);

      length = rom_start;
      offset = offset - start + rom_start * (skip / rom[idx].group);

      if (rom[idx].group == 1)
      {
         if (rom[idx].type == ROM_WORDSWAP)
            offset ^= 1;

         while (length < rom_end)
         {
            if ((c = file_getc()) == EOF) break;
            mem[offset] = c;
            offset += skip;
            length++;
         }
      }
      else
      {
         while (length < rom_end)
         {
            if (offset < 0)
               msg_printf("problem 0 !!!\n");

            if ((c = file_getc()) == EOF) break;
            mem[offset + 0] = c;
            if ((c = file_getc()) == EOF) break;
            mem[offset + 1] = c;
            offset += skip;
            length += 2;
         }
      }
   }

   if (++idx != max)
   {
      if (rom[idx].type == ROM_CONTINUE)
         goto _continue;
   }

   return idx;
}

static int load_rom_gfx1_init(void)
{

   if ((memory_region_gfx1 = malloc(GFX1_BLOCK_SIZE)) == NULL)
   {
      error_memory("REGION_GFX1");
      return 0;
   }

   return 1;
}

static int load_rom_gfx1_current_block(void)
{
   int i, res;
   char fname[32];

   memset(memory_region_gfx1, 0, GFX1_BLOCK_SIZE);

   for (i = 0; i < num_gfx1rom;)
   {
      strcpy(fname, gfx1rom[i].name);
      if ((res = file_open(game_name, parent_name, gfx1rom[i].crc, fname)) < 0)
      {
         if (res == -1)
            error_file(fname);
         else
            error_crc(fname);
         return 0;
      }

      i = rom_load_current_block(gfx1rom, memory_region_gfx1, i, num_gfx1rom);

      file_close();
   }

   return 1;
}

static void free_memory(void)
{
   if (memory_region_gfx1) free(memory_region_gfx1);
   if (gfx_pen_usage[TILE08]) free(gfx_pen_usage[TILE08]);
   if (gfx_pen_usage[TILE16]) free(gfx_pen_usage[TILE16]);
   if (gfx_pen_usage[TILE32]) free(gfx_pen_usage[TILE32]);
}

static int raw_cache_init(void)
{
   UINT32 header_size, aligned_size;
   char fname[MAX_PATH];

   sprintf(fname, "cache" DELIMITER "%s.cache", game_name);
   if ((cache_fp = fopen(fname, "wb")) == NULL)
   {
      msg_printf("ERROR: Could not create file.\n");
      return 0;
   }

   msg_printf("cache name: cache" DELIMITER "%s.cache\n", game_name);
   msg_printf("Create cache file...\n");

   header_size = 8;
   header_size += gfx_total_elements[TILE08];
   header_size += gfx_total_elements[TILE16];
   header_size += gfx_total_elements[TILE32];
   header_size += 0x200 * sizeof(UINT32);

   aligned_size = (header_size + 0xffff) & ~0xffff;

   fseek(cache_fp, aligned_size, SEEK_SET);

   return 1;
}

static int raw_cache_write_current_block(void)
{
   UINT32 i;

   UINT32 start_b = ((memory_gfx1_current_block) << (GFX1_BLOCK_SHIFT - 16));
   UINT32 end_b = ((memory_gfx1_current_block + 1) << (GFX1_BLOCK_SHIFT - 16));

   for (i = start_b; i < end_b; i++)
   {
      if (block_empty_local[i]) continue;

      fwrite(&memory_region_gfx1[(i - start_b) << 16], 1, 0x10000, cache_fp);
   }

   return 1;
}

static int raw_cache_finalize(void)
{
   UINT32 i, offset;
   UINT32 header_size, aligned_size, block[0x200];
   char version[8];
   sprintf(version, "CPS2V23");

   header_size = 8;
   header_size += gfx_total_elements[TILE08];
   header_size += gfx_total_elements[TILE16];
   header_size += gfx_total_elements[TILE32];
   header_size += 0x200 * sizeof(UINT32);

   aligned_size = (header_size + 0xffff) & ~0xffff;
   fseek(cache_fp, 0, SEEK_SET);

   offset = aligned_size;
   for (i = 0; i < 0x200; i++)
   {
      if (block_empty_local[i])
         block[i] = 0xffffffff;
      else
      {
         block[i] = offset;
         offset += 0x10000;
      }
   }

   fwrite(version, 1, 8, cache_fp);
   fwrite(gfx_pen_usage[TILE08], 1, gfx_total_elements[TILE08], cache_fp);
   fwrite(gfx_pen_usage[TILE16], 1, gfx_total_elements[TILE16], cache_fp);
   fwrite(gfx_pen_usage[TILE32], 1, gfx_total_elements[TILE32], cache_fp);
   fwrite(block, 1, 0x200 * sizeof(UINT32), cache_fp);

   for (i = header_size; i < aligned_size; i++)
      fputc(0, cache_fp);

   fclose(cache_fp);

   return 1;

}

static void print_progress(int count, int total)
{
   int i, progress = (count * 100) / total;

   msg_printf("%3d%% [", progress);
   for (i = 0; i < progress / 2; i++) msg_printf("*");
   for (; i < 50; i++) msg_printf(".");
   msg_printf("]\r");
   fflush(stdout);
}


int convert_rom(void)
{
   UINT32 i, size, blocks_available = 0;
   int res;

   msg_printf("Checking ROM file... (%s)\n", game_name);

   memory_region_gfx1 = NULL;
   memory_length_gfx1 = 0;

   gfx_pen_usage[0] = NULL;
   gfx_pen_usage[1] = NULL;
   gfx_pen_usage[2] = NULL;

   if ((res = load_rom_info(game_name)) != 0)
   {
      switch (res)
      {
      case 1:
         msg_printf("ERROR: This game is not supported.\n");
         break;
      case 2:
         msg_printf("ERROR: ROM not found. (zip file name incorrect)\n");
         break;
      case 3:
         msg_printf("ERROR: rominfo.cps2 not found.\n");
         break;
      }
      return 0;
   }

   if (!strcmp(game_name, "ssf2ta")
         || !strcmp(game_name, "ssf2tu")
         || !strcmp(game_name, "ssf2tur1")
         || !strcmp(game_name, "ssf2xj"))
      strcpy(cache_name, "ssf2t");
   else if (!strcmp(game_name, "ssf2t"))
      cache_name[0] = '\0';
   else if (!strcmp(game_name, "mpangj"))
      cache_name[0] = '\0';
   else
      strcpy(cache_name, parent_name);

   if (strlen(parent_name))
      msg_printf("Clone set (parent: %s)\n", parent_name);

   i = 0;
   cacheinfo = NULL;
   while (CPS2_cacheinfo[i].name)
   {
      if (!strcmp(game_name, CPS2_cacheinfo[i].name))
      {
         cacheinfo = &CPS2_cacheinfo[i];
         break;
      }
      if (!strcmp(cache_name, CPS2_cacheinfo[i].name))
      {
         cacheinfo = &CPS2_cacheinfo[i];
         break;
      }
      i++;
   }

   if (!cacheinfo)
   {
      msg_printf("ERROR: Unknown romset.\n");
      return 0;
   }

   calc_pen_usage_init();

   if (!load_rom_gfx1_init())
      return 0;

   if (!raw_cache_init())
      return 0;

   clear_empty_blocks_init();

   memory_gfx1_current_block = 0;
   while (((UINT32)memory_gfx1_current_block << GFX1_BLOCK_SHIFT) <
          memory_length_gfx1)
   {
      print_progress(memory_gfx1_current_block * 4,
                     (memory_length_gfx1 >> GFX1_BLOCK_SHIFT) * 4);

      if (!load_rom_gfx1_current_block())
         return 0;
      print_progress(memory_gfx1_current_block * 4 + 1,
                     (memory_length_gfx1 >> GFX1_BLOCK_SHIFT) * 4);

      cps2_gfx_decode();
      clear_empty_blocks_current_block();
      print_progress(memory_gfx1_current_block * 4 + 2,
                     (memory_length_gfx1 >> GFX1_BLOCK_SHIFT) * 4);


      calc_pen_usage();

      if (!raw_cache_write_current_block())
         return 0;

      print_progress(memory_gfx1_current_block * 4 + 3,
                     (memory_length_gfx1 >> GFX1_BLOCK_SHIFT) * 4);

      memory_gfx1_current_block++;
   }

   if (!raw_cache_finalize())
      return 0;

   free_memory();

   print_progress(1, 1);
   msg_printf("\n");

   for (i = 0; i < (memory_length_gfx1 >> 16); i++)
   {
      if (!block_empty_local[i]) blocks_available++;
   }
   msg_printf("cache required size = %x\n", blocks_available << 16);

   size = blocks_available << 16;
   if (size != memory_length_gfx1)
      msg_printf("remove empty tiles (total size: %d bytes -> %d bytes)\n",
                 memory_length_gfx1, size);


   return 1;
}




#endif /* USE_CACHE */
