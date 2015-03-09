
#include "libretro.h"

#include "emumain.h"

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;

#define NJEMU_DISPLAY_LIST_ID   0x1300
static PspGeContext main_context_buffer;
static PspGeContext njemu_context_buffer;
//static int main_context_state;
//static int njemu_context_state;

static bool game_is_loading = false;

void msg_printf(const char *text, ...)
{
   static char message_buffer[1024];
   va_list arg;

   va_start(arg, text);
   vsnprintf(message_buffer, sizeof(message_buffer), text, arg);
   va_end(arg);

   if (game_is_loading)
   {
      pspDebugScreenPrintf("%s",message_buffer);
   }

   printf("%s", message_buffer);
}

void fatalerror(const char *text, ...)
{
   va_list arg;

   va_start(arg, text);
   vprintf(text, arg);
   va_end(arg);
}

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "NJEMU-" SYSTEM_NAME;
   info->library_version = "v0.0.1";
   info->need_fullpath = true;
   info->block_extract = true;
   info->valid_extensions = "zip";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width   = FRAME_WIDTH;
   info->geometry.base_height  = FRAME_HEIGHT;
   info->geometry.max_width    = FRAME_WIDTH;
   info->geometry.max_height   = FRAME_HEIGHT;
   info->geometry.aspect_ratio = 16.0/9.0;

   info->timing.fps = FPS;
   info->timing.sample_rate = sound->frequency;
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
   game_is_loading = false;
   getcwd(launchDir, MAX_PATH - 1);
   strcat(launchDir, "/");

   strcpy(cache_dir, launchDir);
   strcat(cache_dir, "cache");

   pad_init();
   video_init();

   sceGuSync(0,0);
//   njemu_context_state = sceGuGetAllStatus();
   sceGeSaveContext(&njemu_context_buffer);

   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

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
   bool ret;
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

   sceGeSaveContext(&main_context_buffer);
   sceGeRestoreContext(&njemu_context_buffer);

   pspDebugScreenInitEx(NULL, PSP_DISPLAY_PIXEL_FORMAT_565, 1);

   game_is_loading = true;
   ret = machine_main();
   game_is_loading = false;

//   sceGuSync(0,0);
//   sceGeSaveContext(&njemu_context_buffer);
//   sceGeRestoreContext(&main_context_buffer);

   return ret;
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


static inline void list_finish_callback(int id)
{
   if (id != NJEMU_DISPLAY_LIST_ID)
      return;

   sceGeSaveContext(&njemu_context_buffer);
   sceGeRestoreContext(&main_context_buffer);

}

void retro_run()
{
   static unsigned int __attribute__((aligned(64))) d_list[1024];
   u16* texture_vram_p = (void*) ((u32)work_frame|0x44000000);
//   u16* texture_vram_p = (void*) ((u32)draw_frame|0x44000000);

   input_poll_cb();



   sceGuSync(0,0);

//   main_context_state = sceGuGetAllStatus();
   sceGeSaveContext(&main_context_buffer);

//   sceGuSetAllStatus(njemu_context_state);
//   sceGeRestoreContext(&njemu_context_buffer);
   sceGuSetCallback(GU_CALLBACK_FINISH, list_finish_callback);

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
   sceGuClearDepth(0);

//   sceGuEnable(GU_DEPTH_TEST);
//   sceGuDepthMask(GU_FALSE);


   sceGuEnable(GU_TEXTURE_2D);
   sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
   sceGuTexScale(1.0f / BUF_WIDTH, 1.0f / BUF_WIDTH);
   sceGuTexOffset(0, 0);
   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
   sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
   sceGuFinish();

   sceGuSync(0,0);

   timer_update_cpu();
   render_audio();
	frames_displayed++;
   update_inputport();

   sceGuSync(0,0);
   static unsigned int __attribute__((aligned(64))) restore_context_d_list[1024];
   sceGuStart(GU_DIRECT, restore_context_d_list);
   sceGuFinishId(NJEMU_DISPLAY_LIST_ID);

   sceGuSync(0,0);
   //  might be necessary to check video_enable here and draw a black frame instead
//   sceGuSync(0,0);


//   njemu_context_state= sceGuGetAllStatus();
//   sceGuSetAllStatus(main_context_state);


   sceGuStart(GU_DIRECT, d_list);
   sceGuEnable(GU_TEXTURE_2D);
   sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
   sceGuTexImage(0, 512, 512, BUF_WIDTH, (texture_vram_p + (FRAME_OFFSET_X + FRAME_OFFSET_Y * BUF_WIDTH)) );

   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
//   sceGuDisable(GU_BLEND);

//   sceGuEnable(GU_SCISSOR_TEST);
//   sceGuScissor(0, 0, SCR_WIDTH, 272);

//   sceGuDepthFunc(GU_GEQUAL);
//   sceGuDepthMask(GU_TRUE);
//   sceGuDisable(GU_DEPTH_TEST);
//   sceGuTexScale(1.0f, 1.0f );
//   sceGuTexOffset(0, 0);
//   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
//   sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);

//   sceGuTexFilter(GU_LINEAR,GU_LINEAR);
   sceGuFinish();


//   video_cb(texture_vram_p, FRAME_WIDTH, FRAME_HEIGHT, BUF_WIDTH);
   video_cb(RETRO_HW_FRAME_BUFFER_VALID, FRAME_WIDTH, FRAME_HEIGHT, BUF_WIDTH);
//   sceDisplaySetFrameBuf(texture_vram_p, 512, PSP_DISPLAY_PIXEL_FORMAT_5551, PSP_DISPLAY_SETBUF_NEXTFRAME );

}

unsigned retro_api_version() { return RETRO_API_VERSION; }

