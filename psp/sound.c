/******************************************************************************

	sound.c

	PSP ・オ・ヲ・□ノ・ケ・□テ・ノ

******************************************************************************/

#include "psp.h"
#include "emumain.h"

static INT16 ALIGN_PSPDATA sound_buffer[SOUND_BUFFER_SIZE];

static struct sound_t sound_info;

struct sound_t *sound = &sound_info;

void sound_update_proc(INT16 *buffer);
void sound_update_proc(INT16 *buffer)
{
	(*sound->update)(sound_buffer);
}
