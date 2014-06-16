/*****************************************************************************

	emumain.c

	エミュレーションコア

******************************************************************************/

#include "emumain.h"
#include <stdarg.h>

/******************************************************************************
	グローバル変数
******************************************************************************/

char game_name[16];
char parent_name[16];
char game_dir[MAX_PATH];

#if USE_CACHE
char cache_parent_name[16];
char cache_dir[MAX_PATH];
#endif

int option_sound_enable = 1;

int machine_driver_type;
int machine_init_type;
int machine_input_type;
int machine_screen_type;
int machine_sound_type;

UINT32 frames_displayed;


