/******************************************************************************

	sound.c

	PSP ・オ・ヲ・□ノ・ケ・□テ・ノ

******************************************************************************/

#include "psp.h"
#include "emumain.h"
#include "libretro.h"

static INT16 ALIGN_PSPDATA sound_buffer[SOUND_BUFFER_SIZE];

static struct sound_t sound_info;

struct sound_t *sound = &sound_info;

static retro_audio_sample_batch_t audio_batch_cb;
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }

void render_audio(void)
{
	(*sound->update)(sound_buffer);
   audio_batch_cb(sound_buffer, 400);
   audio_batch_cb(sound_buffer + 800, 400);
}
