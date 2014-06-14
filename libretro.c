
#include "libretro.h"

//#include "emumain.h"
#include "cps2.h"

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];


#include <stdio.h>
#include <string.h>

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "NJEMU";
   info->library_version = "v0.0.1";
   info->need_fullpath = true;
   info->block_extract = true;
   info->valid_extensions = "zip";
}

static struct retro_system_timing g_timing;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   g_timing.fps = 60.0;
   g_timing.sample_rate = 44100;

   struct retro_game_geometry geom = { 160, 144, 160, 144 };
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
//void ui_init(void){}
//int messagebox(int number){return 0;}
void cps2_main(void){}
static int cps2_init(void)
{
   if (!cps2_driver_init())
      return 0;

   msg_printf(TEXT(DONE2));   

   video_clear_screen();
   return cps2_video_init();
}

static void cps2_reset(void)
{
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

static void cps2_exit(void)
{
   video_set_mode(32);
   video_clear_screen();

   video_clear_screen();

   msg_printf(TEXT(PLEASE_WAIT2));

   cps2_video_exit();
   cps2_driver_exit();

   msg_printf(TEXT(DONE2));

}




void retro_init()
{

   getcwd(launchDir, MAX_PATH - 1);
   strcat(launchDir, "/");
   printf("\n%s\n",launchDir);

   pad_init();
   video_init();
//   show_frame = (void *)(0x200000 - FRAMESIZE * 4);
//   draw_frame = (void *)(0x200000 - FRAMESIZE * 3);
//   work_frame = (void *)(0x200000 - FRAMESIZE * 2);
//   tex_frame  = (void *)(0x200000 - FRAMESIZE * 5);

//   sceGuStart(GU_DIRECT, gulist);
//   sceGuDrawBuffer(GU_PSM_5551, draw_frame, BUF_WIDTH);
//   sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, show_frame, BUF_WIDTH);
//   sceGuFinish();
//   sceGuSync(0, GU_SYNC_FINISH);

//   video_clear_frame(show_frame);
//   video_clear_frame(draw_frame);
//   video_clear_frame(work_frame);

//   sceDisplayWaitVblankStart();
   strcpy(game_dir, launchDir);


   video_clear_screen();



   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;


   if (environ_cb)
   {
      g_timing.fps = 60.0;
      g_timing.sample_rate = 44100;
   }

//   video_init();
//   file_browser();
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
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_set_controller_port_device(unsigned port, unsigned device) {}

void retro_reset()
{
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
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[NJEMU]: RGB565 is not supported.\n");
      return false;
   }

//      video_init();
   strcpy(game_name, "sfa");
   memory_init();
   sound_init();
   input_init();
   cps2_init();
   cps2_reset();

//   show_frame = (void*)0x110000;
//   show_frame = memalign(16,FRAMESIZE);

//   printf("show_frame : %08X \n", (u32)show_frame);
//   printf("draw_frame : %08X \n", (u32)draw_frame);
//   printf("work_frame : %08X \n", (u32)work_frame);
//   printf("tex_frame  : %08X \n", (u32)tex_frame);


//   void* texture_vram_p = (void*) ((u32)show_frame|0x04000000);
//   texture_vram_p = memalign(16,FRAMESIZE);
//   sceDisplaySetFrameBuf(texture_vram_p,512,PSP_DISPLAY_PIXEL_FORMAT_5551,PSP_DISPLAY_SETBUF_NEXTFRAME);

//   while(true)
//   {
//   input_poll_cb();

//   update_screen();
//	frames_displayed++;
// video_flip_screen(0);
//   timer_update_cpu();

//   update_inputport();


//   void* current_framebuffer;
//   int fb_w,fb_f;
//   sceDisplayGetFrameBuf(&current_framebuffer,&fb_w,&fb_f,PSP_DISPLAY_SETBUF_NEXTFRAME );
//   sceDisplayWaitVblankStart();
//   memcpy(texture_vram_p,(void*) ((u32)draw_frame|0x04000000),FRAMESIZE);
//   printf("framebuffer: %08X \n", (u32)current_framebuffer );

//   }

//   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_0RGB1555;
//   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
//   {
//      if (log_cb)
//         log_cb(RETRO_LOG_INFO, "[NJEMU]: RGB565 is not supported.\n");
//      return false;
//   }

//   if (!load(info->data, info->size)))
//      return false;


   return true;
}


bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{ return false; }

void retro_unload_game()
{
   cps2_exit();
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
//   printf("retro_run()\n");
//   fflush(stdout);

//   return;
//   static int first_run=1;
//   if (first_run)
//   {
//      printf("first run\n");
//      fflush(stdout);
//      first_run = 0;

//      sceGuStart(GU_DIRECT, gulist);
//   sceGuDrawBuffer(GU_PSM_5650,(void*)0,512);
//   sceGuDispBuffer(512,272,(void*)0,512);
//      sceGuEnable(GU_SCISSOR_TEST);
//      sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
//      sceGuDisable(GU_ALPHA_TEST);
//      sceGuAlphaFunc(GU_LEQUAL, 0, 0x01);
//      sceGuDisable(GU_BLEND);
//      sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
//      sceGuDisable(GU_DEPTH_TEST);
//      sceGuDepthRange(65535, 0);
//      sceGuDepthFunc(GU_GEQUAL);
//      sceGuDepthMask(GU_TRUE);
//      sceGuEnable(GU_TEXTURE_2D);
//      sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
//      sceGuTexScale(1.0f / BUF_WIDTH, 1.0f / BUF_WIDTH);
//      sceGuTexOffset(0, 0);
//      sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
//      sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
//      sceGuFinish();
//      video_clear_frame(show_frame);
//      video_clear_frame(draw_frame);
//      video_clear_frame(work_frame);

//   }


   static unsigned int __attribute__((aligned(16))) d_list[32];
//   void* texture_vram_p = (void*) ((u32)draw_frame|0x04000000);
//   void* texture_vram_p = (void*) ((u32)draw_frame|0x04000000);
   u16* texture_vram_p = (void*) ((u32)work_frame|0x04000000);
//   void* texture_vram_p = (void*) ((u32)tex_frame|0x04000000);

//   static int frames=0;
//   printf("frame = %i\n", frames++);

//   sceDisplaySetFrameBuf((void*) ((u32)draw_frame|0x04000000),512,PSP_DISPLAY_PIXEL_FORMAT_5551,PSP_DISPLAY_SETBUF_NEXTFRAME);
//   sceDisplaySetFrameBuf((void*) ((u32)show_frame|0x04000000),512,PSP_DISPLAY_PIXEL_FORMAT_5551,PSP_DISPLAY_SETBUF_NEXTFRAME);
//   sceDisplaySetFrameBuf((void*)0x040BC000,512,PSP_DISPLAY_PIXEL_FORMAT_5551,PSP_DISPLAY_SETBUF_NEXTFRAME);
//   while(true)
//   {
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

//   update_screen();
//   sceGuSync(0,0);
//   draw_frame = sceGuSwapBuffers();
//   sceGuSync(0,0);
//   sceGuSwapBuffers();

//   static unsigned int current_buf = 0;
//   draw_frame = (void*)(0x44000 * (current_buf^1));
//   draw_frame = (void*)(0x44000);

   timer_update_cpu();
	frames_displayed++;
   update_inputport();

//   sceGuSync(0,0);
//   }
//   return;

//   update_screen();
//   PspGeContext current_context;
//   sceGuSync(0,0);
//   sceGeSaveContext(&current_context);
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

//   sceDisplaySetFrameBuf((void*) ((u32)work_frame|0x04000000),512,PSP_DISPLAY_PIXEL_FORMAT_5551,PSP_DISPLAY_SETBUF_NEXTFRAME);
//   sceGuSync(0,0);
//   sceGeRestoreContext(&current_context);


}

unsigned retro_api_version() { return RETRO_API_VERSION; }

