/******************************************************************************

	emumain.c

	エミュレーションコア

******************************************************************************/

#ifndef EMUMAIN_H
#define EMUMAIN_H

#include "psp/psp.h"
#include "include/cpuintrf.h"
#include "include/memory.h"
#include "zip/zfile.h"
#include "common/loadrom.h"
#include "common/state.h"

#if USE_CACHE
#include "common/cache.h"
#if (EMU_SYSTEM == CPS2)
#include "cps2/romcnv.h"
#endif
#endif /* USE_CACHE */

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "common/coin.h"
#endif

#if (EMU_SYSTEM == CPS1)
#include "cps1/cps1.h"
#elif (EMU_SYSTEM == CPS2)
#include "cps2/cps2.h"
#elif (EMU_SYSTEM == MVS)
#include "mvs/mvs.h"
#elif (EMU_SYSTEM == NCDZ)
#include "ncdz/ncdz.h"
#endif


extern char game_name[16];
extern char parent_name[16];
extern char game_dir[MAX_PATH];

#if USE_CACHE
extern char cache_parent_name[16];
extern char cache_dir[MAX_PATH];
#endif


extern int option_sound_enable;

extern int machine_driver_type;
extern int machine_input_type;
extern int machine_init_type;
extern int machine_screen_type;
extern int machine_sound_type;

extern UINT32 frames_displayed;


#endif /* EMUMAIN_H */
