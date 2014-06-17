/******************************************************************************

	mvs.c

	MVS・ィ・゜・螂□`・キ・逾□ウ・「

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	・ー・□`・ミ・□萍□
******************************************************************************/

int neogeo_bios;
int neogeo_region;
int neogeo_save_sound_flag;



/******************************************************************************
	・□`・ォ・□vハ□
******************************************************************************/

/*--------------------------------------------------------
	MVS・ィ・゜・螂□`・キ・逾□□レサッ
--------------------------------------------------------*/

static int neogeo_init(void)
{
	SceUID fd;
	char path[MAX_PATH];


   sprintf(path, "%smemcard/%s.bin", launchDir, game_name);
   if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0)
   {
      sceIoRead(fd, neogeo_memcard, 0x800);
      sceIoClose(fd);
   }

   sprintf(path, "%snvram/%s.nv", launchDir, game_name);
   if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0)
   {
      sceIoRead(fd, neogeo_sram16, 0x2000);
      sceIoClose(fd);
      swab(neogeo_sram16, neogeo_sram16, 0x2000);
   }


	neogeo_driver_init();
	neogeo_video_init();

	msg_printf(TEXT(DONE2));

   return 1;
}


/*--------------------------------------------------------
	MVS・ィ・゜・螂□`・キ・逾□□サ・テ・ネ
--------------------------------------------------------*/

void neogeo_reset(void)
{
	timer_reset();
	input_reset();

	neogeo_driver_reset();
	neogeo_video_reset();

	sound_reset();
	blit_clear_all_sprite();	

	Loop = LOOP_EXEC;
}


/*--------------------------------------------------------
	MVS・ィ・゜・螂□`・キ・逾□Kチヒ
--------------------------------------------------------*/

void neogeo_exit(void)
{
	SceUID fd;
	char path[MAX_PATH];

	msg_printf(TEXT(PLEASE_WAIT2));

   sprintf(path, "%smemcard/%s.bin", launchDir, game_name);
   if ((fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777)) >= 0)
   {
      sceIoWrite(fd, neogeo_memcard, 0x800);
      sceIoClose(fd);
   }

   sprintf(path, "%snvram/%s.nv", launchDir, game_name);
   if ((fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777)) >= 0)
   {
      swab(neogeo_sram16, neogeo_sram16, 0x2000);
      sceIoWrite(fd, neogeo_sram16, 0x2000);
      sceIoClose(fd);
   }

	msg_printf(TEXT(DONE2));
   sound_exit();
   memory_shutdown();
}



/******************************************************************************
	・ー・□`・ミ・□vハ□
******************************************************************************/

/*--------------------------------------------------------
	MVS・ィ・゜・螂□`・キ・逾□皈、・□
--------------------------------------------------------*/

int neogeo_main(void)
{
   if (memory_init())
   {
      if (sound_init())
      {
         if (input_init())
         {
            if (neogeo_init())
            {
               neogeo_reset();
               return 1;
            }
         }
      }
      sound_exit();
   }
   memory_shutdown();
   return 0;
}
