
#include "libretro.h"

#include "emumain.h"

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];


#include <stdio.h>
#include <string.h>
//#include <stdarg.h>

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "NJEMU-" SYSTEM_NAME;
   info->library_version = "v0.0.1";
   info->need_fullpath = true;
   info->block_extract = true;
   info->valid_extensions = "zip";
}

static struct retro_system_timing g_timing;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   struct retro_game_geometry geom = { 160, 144, 160, 144, 16.0/9.0 };
   info->geometry = geom;
   info->timing   = g_timing;
}

//void msg_printf(const char *text, ...)
//{
//   if(!log_cb)
//      return;
//   va_list arg;
//   va_start(arg, text);
//   log_cb(0,text, arg);
//   va_end(arg);
//}


void retro_init()
{

   getcwd(launchDir, MAX_PATH - 1);
   strcat(launchDir, "/");
   printf("\n%s\n",launchDir);

   pad_init();
   video_init();


   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;


   if (environ_cb)
   {
      g_timing.fps = 60.0;
      g_timing.sample_rate = 48000;
   }

}

void retro_deinit()
{

}

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_set_controller_port_device(unsigned port, unsigned device) {}

void retro_reset()
{
   machine_reset();
}


size_t retro_serialize_size()
{
   return 0;
}

bool retro_serialize(void *data, size_t size)
{
   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   return true;
}

void retro_cheat_reset() {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}


bool retro_load_game(const struct retro_game_info *info)
{
   char* temp_p;
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[NJEMU]: RGB565 is not supported.\n");
      return false;
   }
   strcpy(game_dir, info->path);

   temp_p = strrchr(game_dir, '/');
   if (temp_p)
   {
      *temp_p = '\0';
      strcpy(game_name, temp_p + 1);
   }
   else
   {
      strcpy(game_dir, ".");
      strcpy(game_name, info->path );
   }


   temp_p = strchr(game_name, '.');
   if (temp_p)
      *temp_p = '\0';

   return machine_main();
}


bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{ return false; }

void retro_unload_game()
{
   machine_exit();
}

unsigned retro_get_region() { return RETRO_REGION_NTSC; }

void *retro_get_memory_data(unsigned id)
{

   return 0;
}

size_t retro_get_memory_size(unsigned id)
{

   return 0;
}

void retro_run()
{


   static unsigned int __attribute__((aligned(16))) d_list[32];
   u16* texture_vram_p = (void*) ((u32)work_frame|0x04000000);
   input_poll_cb();

   sceGuStart(GU_DIRECT, gulist);
   sceGuEnable(GU_SCISSOR_TEST);
   sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
   sceGuDisable(GU_ALPHA_TEST);
   sceGuAlphaFunc(GU_LEQUAL, 0, 0x01);
   sceGuDisable(GU_BLEND);
   sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
   sceGuDisable(GU_DEPTH_TEST);
   sceGuDepthRange(65535, 0);
   sceGuDepthFunc(GU_GEQUAL);
   sceGuDepthMask(GU_TRUE);
   sceGuEnable(GU_TEXTURE_2D);
   sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
   sceGuTexScale(1.0f / BUF_WIDTH, 1.0f / BUF_WIDTH);
   sceGuTexOffset(0, 0);
   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
   sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
   sceGuFinish();


   timer_update_cpu();
   render_audio();
	frames_displayed++;
   update_inputport();

   //  might be necessary to check video_enable here and draw a black frame instead

   sceGuStart(GU_DIRECT, d_list);
   sceGuTexImage(0, 512, 512, 512, (texture_vram_p + (64 + 16 *512)) );
   sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
   sceGuDisable(GU_BLEND);

   sceGuEnable(GU_SCISSOR_TEST);
   sceGuScissor(0, 0, SCR_WIDTH, 272);


   sceGuDepthFunc(GU_GEQUAL);
   sceGuDepthMask(GU_TRUE);
   sceGuDisable(GU_DEPTH_TEST);
   sceGuTexScale(1.0f, 1.0f );
   sceGuTexOffset(0, 0);
//   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
//   sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);

   sceGuTexFilter(GU_LINEAR,GU_LINEAR);   
   sceGuFinish();


   video_cb(texture_vram_p, 384, 224, 512);

}

unsigned retro_api_version() { return RETRO_API_VERSION; }

