/******************************************************************************

	filer.c

	PSP ・ユ・。・、・□ヨ・鬣ヲ・カ

******************************************************************************/

#include "emumain.h"
#include "zlib.h"

#define MAX_ENTRY 1024
#define MAX_GAMES 512

#define GAME_NOT_WORK	0x01
#define GAME_BOOTLEG	0x02
#define GAME_HACK		0x04
#define GAME_BADROM		0x08
#define GAME_HAS_TITLE	0x10
#define GAME_IS_NEOCD	0x20

#define FTYPE_UPPERDIR	0
#define FTYPE_DIR		1
#define FTYPE_ZIP		2
#define FTYPE_FILE		3


/******************************************************************************
	・ー・□`・ミ・□萍□
******************************************************************************/

char startupDir[MAX_PATH];


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

static struct dirent *files[MAX_ENTRY];
static SceIoDirent dir;

static char curr_dir[MAX_PATH];
static int nfiles;

#if (EMU_SYSTEM == NCDZ)

static int neocddir;
static int has_mp3;
static int bios_error;
static char zipped_rom[MAX_PATH];

#else

static struct zipname_t
{
	char zipname[16];
	char title[128];
	int flag;
} zipname[MAX_GAMES];

static int zipname_num;

#endif


#if (EMU_SYSTEM == NCDZ)

/******************************************************************************
	・□`・ォ・□vハ□(NCDZPSP)
******************************************************************************/

/*--------------------------------------------------------
	title_x.sys、□、・皓`・ク・ミ・テ・ユ・。、ヒテ霆ュ
--------------------------------------------------------*/

#if PSP_VIDEO_32BPP
static void title_draw_spr(int sx, int sy, UINT8 *spr, UINT32 *palette, int tileno)
#else
static void title_draw_spr(int sx, int sy, UINT8 *spr, UINT16 *palette, int tileno)
#endif
{
	UINT32 tile, lines = 16;
	UINT32 *src = (UINT32 *)(spr + tileno * 128);
#if PSP_VIDEO_32BPP
	UINT32 *dst = (UINT32 *)video_frame_addr(tex_frame, sx, sy);
	UINT32 *pal = &palette[tileno << 4];
#else
	UINT16 *dst = (UINT16 *)video_frame_addr(tex_frame, sx, sy);
	UINT16 *pal = &palette[tileno << 4];
#endif

	while (lines--)
	{
		tile = src[0];
		dst[ 0] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 4] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 1] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 5] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 2] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 6] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 3] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 7] = pal[tile & 0x0f];
		tile = src[1];
		dst[ 8] = pal[tile & 0x0f]; tile >>= 4;
		dst[12] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 9] = pal[tile & 0x0f]; tile >>= 4;
		dst[13] = pal[tile & 0x0f]; tile >>= 4;
		dst[10] = pal[tile & 0x0f]; tile >>= 4;
		dst[14] = pal[tile & 0x0f]; tile >>= 4;
		dst[11] = pal[tile & 0x0f]; tile >>= 4;
		dst[15] = pal[tile & 0x0f];
		src += 2;
		dst += BUF_WIDTH;
	}
}


/*--------------------------------------------------------
	title_x.sys、□i、゜゛z、□
--------------------------------------------------------*/

static int load_title(const char *path, int number)
{
	int i, fd, region, tileno, x, y, found = 0;
	UINT8  title_spr[0x1680];
	UINT16 palette[0x5a0 >> 1];
#if PSP_VIDEO_32BPP
	UINT32 palette32[0x5a0 >> 1];
#endif
	char title_path[MAX_PATH], region_chr[3] = {'j','u','e'};

	zip_open(path);

	fd = -1;
	for (region = neogeo_region & 0x03; region >= 0; region--)
	{
		sprintf(title_path, "title_%c.sys", region_chr[region]);

		if ((fd = zopen(title_path)) != -1)
		{
			found = 1;
			break;
		}
	}

	if (!found)
	{
		zip_close();
		return 0;
	}

	zread(fd, palette, 0x5a0);
	zread(fd, title_spr, 0x1680);
	zclose(fd);

	zip_close();

	swab((UINT8 *)palette, (UINT8 *)palette, 0x5a0);

	for (i = 0; i < 0x5a0 >> 1; i++)
	{
#if PSP_VIDEO_32BPP
		int r = ((palette[i] >> 7) & 0x1e) | ((palette[i] >> 14) & 0x01);
		int g = ((palette[i] >> 3) & 0x1e) | ((palette[i] >> 13) & 0x01);
		int b = ((palette[i] << 1) & 0x1e) | ((palette[i] >> 12) & 0x01);

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		palette32[i] = MAKECOL32(r, g, b);
#else
		int r = ((palette[i] >> 7) & 0x1e) | ((palette[i] >> 14) & 0x01);
		int g = ((palette[i] >> 3) & 0x1e) | ((palette[i] >> 13) & 0x01);
		int b = ((palette[i] << 1) & 0x1e) | ((palette[i] >> 12) & 0x01);

		palette[i] = MAKECOL15(r, g, b);
#endif
	}

	neogeo_decode_spr(title_spr, 0, 0x1680);

	tileno = 0;

	for (y = 0; y < 80; y += 16)
	{
		for (x = 0; x < 144; x += 16)
		{
#if PSP_VIDEO_32BPP
			title_draw_spr(x, y, title_spr, palette32, tileno);
#else
			title_draw_spr(x, y, title_spr, palette, tileno);
#endif
			tileno++;
		}
	}

	return 1;
}


/*--------------------------------------------------------
	title_x.sys、□□セ
--------------------------------------------------------*/

static void show_title(int sx, int sy)
{
	RECT clip1 = { 0, 0, 144, 80 };
	RECT clip2 = { sx, sy, sx + 144, sy + 80 };

	draw_box_shadow(sx, sy, sx + 144, sy + 80);
	video_copy_rect(tex_frame, draw_frame, &clip1, &clip2);
}


/*--------------------------------------------------------
	NEOGEO CDZ、ホBIOS、□チ・ァ・テ・ッ
--------------------------------------------------------*/

static void check_neocd_bios(void)
{
	FILE *fp;
	char path[MAX_PATH];
	UINT8 *temp_mem;

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
}


#else

/******************************************************************************
	・□`・ォ・□vハ□(NCDZPSPメヤヘ□
******************************************************************************/

/*--------------------------------------------------------
	zipname.dat、ォ、騷ip・ユ・。・、・□詹ヌゥ`・ソ・ルゥ`・ケ、□i、゜゛z、゜
--------------------------------------------------------*/

#if (EMU_SYSTEM == CPS1)
#define EXT		"cps1"
#elif (EMU_SYSTEM == CPS2)
#define EXT		"cps2"
#elif (EMU_SYSTEM == MVS)
#define EXT		"mvs"
#endif

static int load_zipname(void)
{
	FILE *fp;
	char path[MAX_PATH], buf[256];
	int size;

	sprintf(path, "%szipname." EXT, launchDir);
	if ((fp = fopen(path, "rb")) == NULL)
		return 0;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	zipname_num = 0;
	while (zipname_num < MAX_GAMES)
	{
		char *linebuf;
		char *name;
		char *title;
		char *flag;

		memset(buf, 0, 256);

		if (!fgets(buf, 255, fp)) break;

		linebuf = strtok(buf, "\r\n");

		zipname[zipname_num].flag = 0;

		name  = strtok(linebuf, ",\r\n");
		title = strtok(NULL, ",\r\n");
		flag  = strtok(NULL, ",\r\n");

		strcpy(zipname[zipname_num].zipname, name);
		strcpy(zipname[zipname_num].title, title);
		if (flag)
		{
			if (strstr(flag, "GAME_BOOTLEG"))
				zipname[zipname_num].flag |= GAME_BOOTLEG;
			if (strstr(flag, "GAME_HACK"))
				zipname[zipname_num].flag |= GAME_HACK;
			if (strstr(flag, "GAME_NOT_WORK"))
				zipname[zipname_num].flag |= GAME_NOT_WORK;
		}
		zipname_num++;
	}

	fclose(fp);

	return 1;
}


/*--------------------------------------------------------
	zip・ユ・。・、・□詹ヌゥ`・ソ・ルゥ`・ケ、□箙ナ
--------------------------------------------------------*/

static void free_zipname(void)
{
	zipname_num = 0;
}


/*--------------------------------------------------------
	zip・ユ・。・、・□訷ォ、鬣イゥ`・爭ソ・、・ネ・□訷□。オテ
--------------------------------------------------------*/


static char *get_zipname(const char *name, int *flag)
{
	int i, length;
	char fname[MAX_PATH];

	strcpy(fname, name);
	*strrchr(fname, '.') = '\0';

	length = strlen(fname);
	if (length > 16) return NULL;//ZIP NAME length

	for (i = 0; i < length; i++)
		fname[i] = tolower((int)fname[i]);

	for (i = 0; i < zipname_num; i++)
	{
		if (stricmp(fname, zipname[i].zipname) == 0)
		{
			*flag = zipname[i].flag;
			return zipname[i].title;
		}
	}
	*flag = 0;
	return NULL;
}

#endif


/******************************************************************************
	・□`・ォ・□vハ□(ケイヘィ)
******************************************************************************/

/*--------------------------------------------------------
	・ヌ・」・□ッ・ネ・熙ホエ贇レ・チ・ァ・テ・ッ
--------------------------------------------------------*/

static void checkDir(const char *name)
{
	int fd, found;
	char path[MAX_PATH];

	memset(&dir, 0, sizeof(dir));

	fd = sceIoDopen(launchDir);
	found = 0;

	while (!found)
	{
		if (sceIoDread(fd, &dir) <= 0) break;

		if (dir.d_stat.st_attr == FIO_SO_IFDIR)
			if (stricmp(dir.d_name, name) == 0)
				found = 1;
	}

	sceIoDclose(fd);

	if (!found)
	{
		sprintf(path, "%s%s", launchDir, name);
		sceIoMkdir(path, 0777);
	}
}


/*--------------------------------------------------------
	・ヌ・」・□ッ・ネ・熙ホラ□ノ、ネニ□モ・ヌ・」・□ッ・ネ・熙ホ・チ・ァ・テ・ッ
--------------------------------------------------------*/

static void checkStartupDir(void)
{
	int fd;

#if USE_CACHE
	checkDir("cache");
#endif
	checkDir("roms");
	checkDir("config");
//	checkDir("snap");
#ifdef SAVE_STATE
	checkDir("state");
#endif
#if PSP_VIDEO_32BPP
	checkDir("data");
#endif
#if (EMU_SYSTEM == MVS)
	checkDir("memcard");
#endif
#if (EMU_SYSTEM != NCDZ)
	checkDir("nvram");
#endif

	fd = sceIoDopen(startupDir);
	if (fd >= 0)
	{
		strcpy(curr_dir, startupDir);
		sceIoDclose(fd);
	}
	else
	{
		strcpy(startupDir, launchDir);
		strcat(startupDir, "roms");
	}
}


/*--------------------------------------------------------
	・ユ・。・、・□ホ・ユ・鬣ー、□Oカィ
--------------------------------------------------------*/

static int set_file_flags(const char *path, int number)
{
#if (EMU_SYSTEM == NCDZ)
	if (files[number]->type == FTYPE_ZIP)
	{
		int fd;
		char zipname[MAX_PATH];

		sprintf(zipname, "%s/%s", path, files[number]->name);
		zip_open(zipname);

		if ((fd = zopen("ipl.txt")) != -1)
		{
			zclose(fd);
			strcpy(zipped_rom, files[number]->name);
			neocddir = 2;
		}
		zip_close();
		return 0;
	}
	else
	{
		files[number]->flag = GAME_HAS_TITLE;

		strcpy(files[number]->title, files[number]->name);
		strcat(files[nfiles]->title, "/");
	}
#else
	if (files[number]->type == FTYPE_ZIP)
	{
		char *title;

		if ((title = get_zipname(dir.d_name, &files[number]->flag)) == NULL)
		{
			files[number]->flag = GAME_BADROM;
			strcpy(files[number]->title, files[number]->name);
		}
		else
		{
			strcpy(files[number]->title, title);
		}
#if RELEASE
		if (files[nfiles]->flag & GAME_BOOTLEG)
		{
			return 0;
		}
#endif
	}
	else
	{
		files[number]->flag = 0;
		strcpy(files[number]->title, files[number]->name);
		strcat(files[nfiles]->title, "/");
	}
#endif

	return 1;
}


/*--------------------------------------------------------
	・ヌ・」・□ッ・ネ・□ィ・□ネ・熙□。オテ
--------------------------------------------------------*/

static void getDir(const char *path)
{
	int i, j, type, fd;

	memset(&dir, 0, sizeof(dir));

	nfiles = 0;
#if (EMU_SYSTEM == NCDZ)
	neocddir = 0;
	has_mp3 = 0;
#endif

	if (strcmp(path, "ms0:/") != 0)
	{
		strcpy(files[nfiles]->name, "..");
		strcpy(files[nfiles]->title, "..");
		files[nfiles]->type = FTYPE_UPPERDIR;
		files[nfiles]->flag = 0;
		nfiles++;
	}

	fd = sceIoDopen(path);

	while (nfiles < MAX_ENTRY)
	{
		char *ext;

		if (sceIoDread(fd, &dir) <= 0)
		{
			break;
		}
		if (dir.d_name[0] == '.')
		{
			continue;
		}
#if (EMU_SYSTEM == MVS)
		if (stricmp(dir.d_name, "neogeo.zip") == 0)
		{
			continue;
		}
#elif (EMU_SYSTEM == NCDZ)
		if (stricmp(dir.d_name, "ipl.txt") == 0)
		{
			neocddir = 1;
			continue;
		}
#endif
		if ((ext = strrchr(dir.d_name, '.')) != NULL)
		{
#ifdef COMMAND_LIST
			if (stricmp(dir.d_name, "command.dat") == 0)
			{
				strcpy(files[nfiles]->name, dir.d_name);
				strcpy(files[nfiles]->title, dir.d_name);
				files[nfiles]->type = FTYPE_FILE;
				files[nfiles]->flag = 0;
				nfiles++;
				continue;
			}
#endif
			if (stricmp(ext, ".zip") == 0)
			{
				strcpy(files[nfiles]->name, dir.d_name);
				files[nfiles]->type = FTYPE_ZIP;
				if (set_file_flags(path, nfiles))
				{
					nfiles++;
				}
				continue;
			}
		}
		if (dir.d_stat.st_attr == FIO_SO_IFDIR)
		{
#if USE_CACHE
			if (stricmp(dir.d_name, "cache") == 0) continue;
#endif
			if (stricmp(dir.d_name, "config") == 0) continue;
//			if (stricmp(dir.d_name, "snap") == 0) continue;
#if (EMU_SYSTEM != NCDZ)
			if (stricmp(dir.d_name, "nvram") == 0) continue;
#endif
#ifdef SAVE_STATE
			if (stricmp(dir.d_name, "state") == 0) continue;
#endif
#if PSP_VIDEO_32BPP
			if (stricmp(dir.d_name, "data") == 0) continue;
#endif
#if (EMU_SYSTEM == MVS)
			if (stricmp(dir.d_name, "memcard") == 0) continue;
#elif (EMU_SYSTEM == NCDZ)
			if (stricmp(dir.d_name, "mp3") == 0)
			{
				has_mp3 = 1;
				continue;
			}
#endif
			strcpy(files[nfiles]->name, dir.d_name);
			files[nfiles]->type = FTYPE_DIR;
			set_file_flags(path, nfiles);
			nfiles++;
		}
	}

	sceIoDclose(fd);

	for (i = 0; i < nfiles - 1; i++)
	{
		for (j = i + 1; j < nfiles; j++)
		{
			if (files[i]->type > files[j]->type)
			{
				struct dirent tmp;

				tmp = *files[i];
				*files[i] = *files[j];
				*files[j] = tmp;
			}
		}
	}

	for (type = 1; type < 4; type++)
	{
		int start = nfiles, end = 0;

		for (i = 0; i < nfiles; i++)
		{
			if (files[i]->type == type)
			{
				start = i;
				break;
			}
		}
		for (; i < nfiles; i++)
		{
			if (files[i]->type != type)
			{
				end = i;
				break;
			}
		}

		if (start == nfiles) continue;
		if (end == 0) end = nfiles;

		for (i = start; i < end - 1; i++)
		{
			for (j = i + 1; j < end; j++)
			{
				if (strcmp(files[i]->title, files[j]->title) > 0)
				{
					struct dirent tmp;

					tmp = *files[i];
					*files[i] = *files[j];
					*files[j] = tmp;
				}
			}
		}
	}
}




/******************************************************************************
	・ー・□`・ミ・□vハ□
******************************************************************************/

/*--------------------------------------------------------
	・ユ・。・、・□ホエ贇レ・チ・ァ・テ・ッ
--------------------------------------------------------*/

int file_exist(const char *path)
{
	SceUID fd;

	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	sceIoClose(fd);

	return ((fd >= 0) ? 1 : 0);
}


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


/*--------------------------------------------------------
	ヨクカィ、キ、ソ・ム・ソゥ`・□ホ・ユ・。・、・□□□□
--------------------------------------------------------*/

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


/*--------------------------------------------------------
	・ケ・ニゥ`・ネ・ユ・。・、・□□ハヒ□
--------------------------------------------------------*/

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



/*--------------------------------------------------------
	・ユ・。・、・□ヨ・鬣ヲ・カ携ミミ
--------------------------------------------------------*/

void file_browser(void)
{
   int i;
   for (i = 0; i < MAX_ENTRY; i++)
   files[i] = (struct dirent *)malloc(sizeof(struct dirent));

#if (EMU_SYSTEM != NCDZ)
	memset(zipname, 0, sizeof(zipname));
	zipname_num = 0;
#endif

	strcpy(curr_dir, launchDir);
	strcat(curr_dir, "roms");
	strcpy(startupDir, curr_dir);


#if (EMU_SYSTEM != NCDZ)
	if (!load_zipname())
	{
		fatalerror(TEXT(COULD_NOT_OPEN_ZIPNAME_DAT), EXT);
		goto error;
	}
#endif
	checkStartupDir();
	getDir(curr_dir);

#if (EMU_SYSTEM == NCDZ)
	check_neocd_bios();
#endif

#if (EMU_SYSTEM == NCDZ)
	if (bios_error)
	{
		show_background();
		small_icon_shadow(6, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
		logo(32, 5, UI_COLOR(UI_PAL_TITLE));
		video_flip_screen(1);

		switch (bios_error)
		{
		case 1: messagebox(MB_BIOSNOTFOUND); break;
		case 2: messagebox(MB_BIOSINVALID); break;
		}
	}
#endif


	emu_main();



error:
	for (i = 0; i < MAX_ENTRY; i++)
	{
		if (files[i]) free(files[i]);
	}
	free_zipname();
}
