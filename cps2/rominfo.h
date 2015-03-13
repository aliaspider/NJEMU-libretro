#ifndef ROMINFO_H
#define ROMINFO_H

#include "stdint.h"

typedef struct
{
   unsigned type  : 2;
   unsigned group : 2;
   unsigned skip  : 3;
   unsigned length : 25;

   uint32_t offset;
   uint32_t crc;
   const char* name;
} rom_x_t;


typedef struct
{
   const char* game_name;
   const char* parent_name;

   unsigned machine_input_type  : 4;
   unsigned machine_init_type   : 1;
   unsigned machine_screen_type : 1;

   unsigned memory_length_user1 : 26;

   unsigned num_cpu1rom : 4;
   unsigned memory_length_cpu1 : 28;
   rom_x_t* cpu1rom;

   unsigned num_cpu2rom : 4;
   unsigned memory_length_cpu2 : 28;
   rom_x_t* cpu2rom;

   unsigned num_gfx1rom : 6;
   unsigned memory_length_gfx1 : 26;
   rom_x_t* gfx1rom;

   unsigned num_snd1rom : 4;
   unsigned memory_length_sound1 : 28;
   rom_x_t* snd1rom;

} rom_info_t;

extern rom_info_t cps2_rom_db [];

#endif // ROMINFO_H
