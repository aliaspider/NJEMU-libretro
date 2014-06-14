/******************************************************************************

	cps2.c

	CPS2・ィ・゜・螂□`・キ・逾□ウ・「

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	・□`・ォ・□vハ□
******************************************************************************/

/*--------------------------------------------------------
	CPS2・ィ・゜・螂□`・キ・逾□□レサッ
--------------------------------------------------------*/

static int cps2_init(void)
{
	if (!cps2_driver_init())
		return 0;

	msg_printf(TEXT(DONE2));

	video_clear_screen();

	return cps2_video_init();
}


/*--------------------------------------------------------
	CPS2・ィ・゜・螂□`・キ・逾□□サ・テ・ネ
--------------------------------------------------------*/

static void cps2_reset(void)
{
	video_set_mode(16);
	video_clear_screen();

	Loop = LOOP_EXEC;

	autoframeskip_reset();

	cps2_driver_reset();
	cps2_video_reset();

	timer_reset();
	input_reset();
	sound_reset();

	blit_clear_all_sprite();
}

/*--------------------------------------------------------
	CPS・ィ・゜・螂□`・キ・逾□Kチヒ
--------------------------------------------------------*/

static void cps2_exit(void)
{
	video_set_mode(32);
	video_clear_screen();

	video_clear_screen();

	msg_printf(TEXT(PLEASE_WAIT2));

	cps2_video_exit();
	cps2_driver_exit();

	msg_printf(TEXT(DONE2));

	show_exit_screen();
}

/*--------------------------------------------------------
	cheats
--------------------------------------------------------*/


/*--------------------------------------------------------
	CPS・ィ・゜・螂□`・キ・逾□gミミ
--------------------------------------------------------*/

static void cps2_run(void)
{
	while (Loop >= LOOP_RESET)
	{
		cps2_reset();

		while (Loop == LOOP_EXEC)
		{
			if (Sleep)
			{
#if USE_CACHE
				cache_sleep(1);
#endif

				do
				{
					sceKernelDelayThread(5000000);
				} while (Sleep);

#if USE_CACHE
				cache_sleep(0);
#endif
				autoframeskip_reset();
			}
			
			timer_update_cpu();
			update_screen();
			update_inputport();
		}

		video_clear_screen();
	}
}


/******************************************************************************
	・ー・□`・ミ・□vハ□
******************************************************************************/

/*--------------------------------------------------------
	CPS2・ィ・゜・螂□`・キ・逾□皈、・□
--------------------------------------------------------*/

void cps2_main(void)
{
	Loop = LOOP_RESET;

	while (Loop >= LOOP_RESTART)
	{
		Loop = LOOP_EXEC;

		fatal_error = 0;

		video_clear_screen();

		if (memory_init())
		{
			if (sound_init())
			{
				if (input_init())
				{
					if (cps2_init())
					{
						cps2_run();
					}
					cps2_exit();
				}
				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}
