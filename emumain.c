/*****************************************************************************

	emumain.c

	エミュレーションコア

******************************************************************************/

#include "emumain.h"
#include <stdarg.h>


#define FRAMESKIP_LEVELS	12


/******************************************************************************
	グローバル変数
******************************************************************************/

char game_name[16];
char parent_name[16];

char game_dir[MAX_PATH];
#if USE_CACHE
char cache_dir[MAX_PATH];
char cache_parent_name[16];
#endif

int option_showfps;
int option_speedlimit;
int option_autoframeskip;
int option_frameskip;
int option_vsync;
int option_stretch;

int option_sound_enable;
int option_samplerate;
int option_sound_volume;

int machine_driver_type;
int machine_init_type;
int machine_input_type;
int machine_screen_type;
int machine_sound_type;

UINT32 frames_displayed;
int fatal_error;


/******************************************************************************
	ローカル変数
******************************************************************************/

static int frameskip;
static int frameskipadjust;
static int frameskip_counter;

static TICKER last_skipcount0_time;
static TICKER this_frame_base;
static int warming_up;

static int frames_since_last_fps;
static int rendered_frames_since_last_fps;
static int game_speed_percent;
static int frames_per_second;

static int snap_no = -1;

static char fatal_error_message[256];

static const UINT8 skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};



/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	エミュレーション開始
--------------------------------------------------------*/

void emu_main(void)
{
#if defined(ADHOC) && (EMU_SYSTEM == MVS)
	int save_neogeo_bios = neogeo_bios;
#endif

	snap_no = -1;

	machine_main();

#if defined(ADHOC) && (EMU_SYSTEM == MVS)
	if (adhoc_enable)
		neogeo_bios = save_neogeo_bios;
#endif
}


/*--------------------------------------------------------
	フレームスキップを初期化
--------------------------------------------------------*/

void autoframeskip_reset(void)
{
	frameskip = option_autoframeskip ? 0 : option_frameskip;
	frameskipadjust = 0;
	frameskip_counter = 0;

	rendered_frames_since_last_fps = 0;
	frames_since_last_fps = 0;

	game_speed_percent = 100;
	frames_per_second = PSP_REFRESH_RATE;
	frames_displayed = 0;

	warming_up = 1;
}


/*--------------------------------------------------------
	フレームスキップテーブル
--------------------------------------------------------*/

UINT8 skip_this_frame(void)
{
	return skiptable[frameskip][frameskip_counter];
}


/*--------------------------------------------------------
	画面更新
--------------------------------------------------------*/

void update_screen(void)
{
	if (warming_up)
	{
		sceDisplayWaitVblankStart();
		last_skipcount0_time = ticker() - (int)((float)FRAMESKIP_LEVELS * PSP_TICKS_PER_FRAME);
		warming_up = 0;
	}

	if (frameskip_counter == 0)
		this_frame_base = last_skipcount0_time + (int)((float)FRAMESKIP_LEVELS * PSP_TICKS_PER_FRAME);

	frames_displayed++;
	frames_since_last_fps++;

   TICKER curr = ticker();
   int flip = 0;

   if (option_speedlimit)
   {
      TICKER target = this_frame_base + (int)((float)frameskip_counter * PSP_TICKS_PER_FRAME);

      if (option_vsync)
      {
         if (curr < target - 100)
         {
            video_flip_screen(1);
            flip = 1;
         }
      }

      while (curr < target)
         curr = ticker();
   }
   if (!flip) video_flip_screen(0);

   rendered_frames_since_last_fps++;

   if (frameskip_counter == 0)
   {
      float seconds_elapsed = (float)(curr - last_skipcount0_time) * (1.0 / 1000000.0);
      float frames_per_sec = (float)frames_since_last_fps / seconds_elapsed;

      game_speed_percent = (int)(100.0 * frames_per_sec / PSP_REFRESH_RATE + 0.5);
      frames_per_second = (int)((float)rendered_frames_since_last_fps / seconds_elapsed + 0.5);

      last_skipcount0_time = curr;
      frames_since_last_fps = 0;
      rendered_frames_since_last_fps = 0;

      if (option_autoframeskip)
      {
         if (option_speedlimit && frames_displayed > 2 * FRAMESKIP_LEVELS)
         {
            if (game_speed_percent >= 99)
            {
               frameskipadjust++;

               if (frameskipadjust >= 3)
               {
                  frameskipadjust = 0;
                  if (frameskip > 0) frameskip--;
               }
            }
            else
            {
               if (game_speed_percent < 80)
               {
                  frameskipadjust -= (90 - game_speed_percent) / 5;
               }
               else if (frameskip < 8)
               {
                  frameskipadjust--;
               }

               while (frameskipadjust <= -2)
               {
                  frameskipadjust += 2;
                  if (frameskip < FRAMESKIP_LEVELS - 1)
                     frameskip++;
               }
            }
         }
      }
   }


	frameskip_counter = (frameskip_counter + 1) % FRAMESKIP_LEVELS;
}


/*--------------------------------------------------------
	致命的エラーメッセージ
--------------------------------------------------------*/

void fatalerror(const char *text, ...)
{
	va_list arg;

	va_start(arg, text);
	vsprintf(fatal_error_message, text, arg);
	va_end(arg);

	fatal_error = 1;
	Loop = LOOP_BROWSER;
}




/*------------------------------------------------------
	スクリーンショット保存
------------------------------------------------------*/

