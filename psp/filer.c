/******************************************************************************

	filer.c

	PSP ・ユ・。・、・□ヨ・鬣ヲ・カ

******************************************************************************/

#include "emumain.h"
#include "zlib.h"



/******************************************************************************
	・□`・ォ・□欽□□我ハ□
******************************************************************************/

struct dirent
{
	int type;
	int flag;
	char name[128];
	char title[128];
};

static SceIoDirent dir;


#if (EMU_SYSTEM == NCDZ)

/*--------------------------------------------------------
	NEOGEO CDZ、ホBIOS、□チ・ァ・テ・ッ
--------------------------------------------------------*/

static int check_neocd_bios(void)
{
	FILE *fp;
	char path[MAX_PATH];
	UINT8 *temp_mem;
   int bios_error;

	bios_error = 0;

	if ((temp_mem = (UINT8 *)malloc(0x80000)) == NULL)
	{
		bios_error = 1;
		return;
	}

	sprintf(path, "%s%s", launchDir, "neocd.bin");

	if ((fp = fopen(path, "rb")) != NULL)
	{
		fread(temp_mem, 1, 0x80000, fp);
		fclose(fp);

		if (crc32(0, temp_mem, 0x80000) != 0xdf9de490)
			bios_error = 2;
	}
	else bios_error = 1;

	free(temp_mem);

   return bios_error;
}

#endif







/*--------------------------------------------------------
	ヨクカィ、キ、ソ・ム・ソゥ`・□ネメサヨツ、ケ、□ユ・。・、・□□ハヒ□
--------------------------------------------------------*/

char *find_file(char *pattern, char *path)
{
	static struct dirent file;
	int fd, i, found, len1, len2;

	memset(&dir, 0, sizeof(dir));

	fd = sceIoDopen(path);
	found = 0;

	len1 = strlen(pattern);

	while (!found)
	{
		if (sceIoDread(fd, &dir) <= 0) break;

		len2 = strlen(dir.d_name);

		for (i = 0; i < len2; i++)
		{
			if (strnicmp(&dir.d_name[i], pattern, len1) == 0)
			{
				strcpy(file.name, dir.d_name);
				found = 1;
				break;
			}
		}
	}

	sceIoDclose(fd);

	return found ? file.name : NULL;
}


void delete_files(const char *dirname, const char *pattern)
{
	int fd, i, len1, len2;
	char path[MAX_PATH];

	memset(&dir, 0, sizeof(dir));

	sprintf(path, "%s%s", launchDir, dirname);

	fd = sceIoDopen(path);
	len1 = strlen(pattern);

	while (1)
	{
		if (sceIoDread(fd, &dir) <= 0) break;

		len2 = strlen(dir.d_name);

		for (i = 0; i < len2; i++)
		{
			if (strnicmp(&dir.d_name[i], pattern, len1) == 0)
			{
				char path2[MAX_PATH];

				sprintf(path2, "%s/%s", path, dir.d_name);
				sceIoRemove(path2);
			}
		}
	}

	sceIoDclose(fd);
}

#ifdef SAVE_STATE

void find_state_file(UINT8 *slot)
{
	int fd, len;
	char path[MAX_PATH], pattern[16];

	memset(&dir, 0, sizeof(dir));

	sprintf(path, "%sstate", launchDir);
	sprintf(pattern, "%s.sv", game_name);

	len = strlen(pattern);
	fd = sceIoDopen(path);

	while (sceIoDread(fd, &dir) > 0)
	{
		if (strnicmp(dir.d_name, pattern, len) == 0)
		{
			int number = dir.d_name[len] - '0';

			if (number >= 0 && number <= 9)
				slot[number] = 1;
		}
	}

	sceIoDclose(fd);
}

#endif


