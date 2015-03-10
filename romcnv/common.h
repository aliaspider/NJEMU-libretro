/*****************************************************************************

	common.c

******************************************************************************/

#ifndef COMMON_H
#define COMMON_H

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned int	UINT32;
//typedef char			INT8;
typedef short			INT16;
typedef int				INT32;
__extension__ typedef unsigned long long	UINT64;
__extension__ typedef signed long long		INT64;

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <malloc.h>
#include "zlib/zlib.h"
#include "zfile.h"

#ifndef MAX_PATH
#define MAX_PATH	512
#endif

#define DELIMITER	'/'

struct rom_t
{
	UINT32 type;
	UINT32 offset;
	UINT32 length;
	UINT32 crc;
	int group;
	int skip;
	char name[32];
};

extern int rom_fd;
extern char delimiter;

extern char game_dir[MAX_PATH];
extern char zip_dir[MAX_PATH];
extern char launchDir[MAX_PATH];

extern char game_name[16];
extern char parent_name[16];
extern char cache_name[16];

void error_memory(const char *mem_name);
void error_file(const char *rom_name);
void error_rom(const char *rom_name);

int file_open(const char *fname1, const char *fname2, const UINT32 crc, char *fname);
void file_close(void);
int file_read(void *buf, size_t length);
int file_getc(void);

int rom_load(struct rom_t *rom, UINT8 *mem, int idx, int max);

int str_cmp(const char *s1, const char *s2);


/*--------------------------------------------------------
   Windows
--------------------------------------------------------*/



#endif /* COMMON_H */
