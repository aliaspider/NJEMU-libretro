/******************************************************************************

	ui_text.c

	・讖`・カ・、・□ソ・ユ・ァゥ`・ケ・ニ・ュ・ケ・ネケワタ□

******************************************************************************/

#include "psp.h"
#include <psputility_sysparam.h>

#define FONT_UPARROW       ""
#define FONT_DOWNARROW       ""
#define FONT_LEFTARROW       ""
#define FONT_RIGHTARROW       ""
#define FONT_CIRCLE       ""
#define FONT_CROSS       ""
#define FONT_SQUARE       ""
#define FONT_TRIANGLE       ""
#define FONT_LTRIGGER       ""
#define FONT_RTRIGGER       ""
#define FONT_UPTRIANGLE       ""
#define FONT_DOWNTRIANGLE       ""
#define FONT_LEFTTRIANGLE       ""
#define FONT_RIGHTTRIANGLE       ""

const char *ui_text[UI_TEXT_MAX] =
{
   "\0",
   "\n",

   /* psp/filer.c */
   "Please wait...",
   "Could not open zipname.%s",
#ifdef ADHOC
   "Please turn on the WLAN switch.",
   "Failed to load AdHoc modules.",
#endif

   /* psp/sound.c */
   "Could not reserve audio channel for sound.",
   "Could not start sound thread.",

   /* psp/ui.c */
   "Warning: Battery is low (%d%%). Please charge battery!",

   FONT_CIRCLE " to launch, " FONT_CROSS " to cancel",
   FONT_CIRCLE " to confirm, " FONT_CROSS " to cancel",
#ifdef SAVE_STATE
   FONT_CIRCLE " return to game, " FONT_CROSS " return to menu",
#endif
   "Press any button.",
   "Start emulation.",
#ifdef ADHOC
   "Start emulation. (AdHoc)",
#endif
   "Exit emulation.",
   "Reset emulation.",
   "Need to restart emulation.",
#if (EMU_SYSTEM != NCDZ)
   "THIS GAME DOESN'T WORK.",
   "You won't be able to make it work correctly.",
   "Don't bother.",
#endif
   "Do you make this directory the startup directory?",
#ifdef PSP_SLIM
   "This program requires PSP-2000 + FW 3.71 M33 or later.",
#endif
#ifdef SAVE_STATE
   "Start save state.",
   "Start load state.",
#endif
#if defined(SAVE_STATE) || defined(COMMAND_LIST)
   "Complete.",
#endif
#ifdef SAVE_STATE
   "Delete state file.",
#endif
#if (EMU_SYSTEM == NCDZ)
   "MP3 files not found. CDDA cannot play.",
   "IPL.TXT not found.",
   "Boot NEO。、GEO CDZ BIOS.",
   "NEO。、GEO CDZ BIOS (neocd.bin) not found.",
   "Invalid NEO。、GEO CDZ BIOS found.",
   "Cannot launch game.",
#endif

   "Help - %s",
   "Not use",
   "Press any button to return to menu.",
   "File browser",
   "Scroll",
   "Scroll 1 page",
   "Launch game",
   "Launch game (AdHoc)",
   "Exit emulator",
   "Show BIOS select menu",
   "Open color settings menu",
   "Show this help",
   "Boot BIOS",
   "Set selected directory as startup",
   "Change value",
   "Select",
   "Select slot",
   "Change function",
   "Execute function",
   "Select item",
   "Select item / Change value",
   "RGB value +5",
   "Return to file browser",
   "RGB value -5",
   "Restore all values to default",
   "Scroll / Select item",
   "Open / Close item menu",
   "Return to main menu / game",

   /* psp/ui_menu.c */
   "On",
   "Off",
   "Yes",
   "No",
   "Enable",
   "Disable",
   "Return to main menu",

   "Game configuration menu",
   "Raster Effects",
   "Stretch Screen",
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
   "360x270 (4:3)",
   "384x270 (24:17)",
   "466x272 (12:7)",
   "480x270 (16:9)",
   "Rotate Screen",
#else
   "320x224 (4:3)",
   "360x270 (4:3)",
   "366x270 (19:14)",
   "420x270 (14:9)",
   "480x270 (16:9)",
#endif
   "Video Sync",
   "Auto Frameskip",
   "Frameskip",
   "level 1",
   "level 2",
   "level 3",
   "level 4",
   "level 5",
   "level 6",
   "level 7",
   "level 8",
   "level 9",
   "level 10",
   "level 11",
   "Show FPS",
   "60fps Limit",
   "Enable Sound",
   "Sample Rate",
   "11025Hz",
   "22050Hz",
   "44100Hz",
   "Sound Volume",
   "0%",
   "10%",
   "20%",
   "30%",
   "40%",
   "50%",
   "60%",
   "70%",
   "80%",
   "90%",
   "100%",
   "Controller",
   "Player 1",
   "Player 2",
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
   "Player 3",
   "Player 4",
#endif
   "PSP clock",
   "222MHz",
   "266MHz",
   "300MHz",
   "333MHz",
#if (EMU_SYSTEM == MVS)
   "Default",
#endif
#if (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
   "Machine Region",
   "Japan",
   "USA",
   "Europe",
#endif
#if (EMU_SYSTEM == MVS)
   "Machine Mode",
   "Console (AES)",
   "Arcade (MVS)",
#endif
#if (EMU_SYSTEM == NCDZ)
   "Emulate Load Screen",
   "CD-ROM Speed Limit",
   "Enable CDDA",
   "CDDA Volume",
#endif

   "Key configuration menu",
   "Not use",
   FONT_UPARROW,
   FONT_DOWNARROW,
   FONT_LEFTARROW,
   FONT_RIGHTARROW,
   FONT_CIRCLE,
   FONT_CROSS,
   FONT_SQUARE,
   FONT_TRIANGLE,
   "L TRIGGER",
   "R TRIGGER",
   "START",
   "SELECT",
   "Low",
   "Normal",
   "High",
#if (EMU_SYSTEM == CPS2)
   "Player1 Start",
   "Player2 Start",
#endif
   "%d frame",
   "%d frames",
   FONT_UPARROW,
   FONT_DOWNARROW,
   FONT_LEFTARROW,
   FONT_RIGHTARROW,
#if (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
   "Button A",
   "Button B",
   "Button C",
   "Button D",
#else
   "Button 1",
   "Button 2",
   "Button 3",
   "Button 4",
   "Button 5",
   "Button 6",
#endif
   "Start",
#if (EMU_SYSTEM == NCDZ)
   "Select",
#else
   "Coin",
   "Service Coin",
#endif
#if (EMU_SYSTEM == MVS)
   "Test Switch",
#elif (EMU_SYSTEM != NCDZ)
   "Service Switch",
#endif
#if (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
   "Autofire A",
   "Autofire B",
   "Autofire C",
   "Autofire D",
#else
   "Autofire 1",
   "Autofire 2",
   "Autofire 3",
   "Autofire 4",
   "Autofire 5",
   "Autofire 6",
#endif
   "Autofire Interval",
#if (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
   "Hotkey A+B",
   "Hotkey A+C",
   "Hotkey A+D",
   "Hotkey B+C",
   "Hotkey B+D",
   "Hotkey C+D",
   "Hotkey A+B+C",
   "Hotkey A+B+D",
   "Hotkey A+C+D",
   "Hotkey B+C+D",
   "Hotkey A+B+C+D",
#else
   "Hotkey 1+2",
   "Hotkey 1+3",
   "Hotkey 1+4",
   "Hotkey 2+3",
   "Hotkey 2+4",
   "Hotkey 3+4",
   "Hotkey 1+2+3",
   "Hotkey 1+2+4",
   "Hotkey 1+3+4",
   "Hotkey 2+3+4",
   "Hotkey 1+2+3+4",
   "Hotkey 4+5+6",
#endif
#if (EMU_SYSTEM != NCDZ)
   "Analog Sensitivity",
#endif
#if (EMU_SYSTEM == CPS1)
   "Dial (Left)",
   "Dial (Right)",
   "Pause",
#endif
#if (EMU_SYSTEM == CPS2)
   "Paddle (Left)",
   "Paddle (Right)",
#endif
#if (EMU_SYSTEM == MVS)
   "Big",
   "Small",
   "Double Up",
   "Start/Collect",
   "Payout",
   "Cancel",
   "Bet",
   "Bet/Cancel All",
   "Operator Menu",
   "Clear Credit",
   "Hopper Out",
#endif
   "Save Screenshot",
   "Switch Player",
#if (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
   "Button Layout",
   "Type 1 (NEOGEO PAD)",
   "Type 2 (MVS)",
   "Type 3 (PlayStation)",
   "User Define",
#endif

   "DIP switch settings menu",
   "This game has no DIP switches.",

   "Save/Load State",
   "Slot %d:",
   "Empty",
   "Load",
   "Delete",
   "Save",
   "Play Date",
   "Save Time",
   "Version",
   "Faild to delete file \"%s\".",

   "Main menu",
   "Game configuration",
   "Change game settings.",
   "Key configuration",
   "Change buttons and autofire/hotkey settings.",
#if (EMU_SYSTEM != NCDZ)
   "DIP switch settings",
   "Change hardware DIP switch settings.",
#endif
   "Reset emulation",
   "Reset " SYSTEM_NAME " emulation.",
   "Return to file browser",
   "Stop emulation and return to file browser.",
   "Return to game",
   "Return to game emulation.",
   "Exit emulator",
   "Exit emulation and return to PSP menu.",
#ifdef COMMAND_LIST
   "Show command list",
   "Show command list. (MAME Plus! format \"command.dat\")",
#endif

#if PSP_VIDEO_32BPP
   "Color settings menu",
   "Background image type",
   "Background image blightness",
   "Title bar text",
   "Selected text",
   "Normal text",
   "Information message text",
   "Warning message text",
   "File select bar (start)",
   "File select bar (end)",
   "Title bar/Message box",
   "Title bar/ Message box frame",
   "Background",
   "Red",
   "Green",
   "Blue",
   "Default image",
   "User's image",
   "logo.png only",
   "File select bar",
#endif

   "Command list",

   /* psp/mp3.c */
#if (EMU_SYSTEM == NCDZ)
   "Counld not reopen mp3 file \"%s\".",
   "MP3 decode error.",
   "Could not reserve audio channel for MP3.",
   "Could not start MP3 thread.",
#endif

   /* psp/adhoc.c*/
#ifdef ADHOC
   "lobby",
   "server",
   "crient",
   "Waiting for another PSP to join.\n",
   "Connecting to the %s.",
   "Connected.",
   "Disconnecting from the %s.",
   "Disconnected.",
   "failed.",
   "Select a server to connect to, or " FONT_TRIANGLE " to return.\n",
   "Waiting for %s to accept the connection.\n",
   "To cancel press " FONT_CROSS ".\n",
   "%s has requested a connection.\n",
   "To accept the connection press " FONT_CIRCLE ", to cancel press " FONT_CROSS ".\n",
   "Wainting for synchronization.",
#endif

   /* psp/png.c */
   "Could not allocate memory for PNG.",
   "Could not enecode PNG image.",
#if PSP_VIDEO_32BPP || (EMU_SYSTEM == NCDZ)
   "Could not decode PNG image.",
#endif
   "%d bit color PNG image not supported.",

   /* emumain.c */
   "Fatal error",
   "Snapshot saved as \"%s_%02d.png\".",

#if USE_CACHE
   /* common/cache.c */
   "Could not open cache file.\n",
   "Memory not enough.\n",
   "Could not allocate cache memory.\n",
   "%dKB cache allocated.\n",
   "Cache load error!!!\n",
   "Loading cache information data...\n",
   "Unsupported version of cache file \"V%c%c\".\n",
   "Current required version is \"" CACHE_VERSION "\".\n",
   "Please rebuild cache file.\n",
#if (EMU_SYSTEM == CPS2)
   "Could not open sprite block %03x\n",
#elif (EMU_SYSTEM == MVS)
   "PCM cache enabled.\n",
#endif
#endif

   /* common/loadrom.c */
   "Press any button.\n",
   "Could not allocate %s memory.\n",
   "CRC32 not correct. \"%s\"\n",
   "File not found. \"%s\"\n",

#ifdef SAVE_STATE
   /* common/state.c */
   "Could not allocate state buffer.",
   "Could not create file \"%s.sv%d\"",
   "Could not open file \"%s.sv%d\"",
   "Saving \"%s.sv%d\"",
   "Loading \"%s.sv%d\"",
#if (EMU_SYSTEM == MVS)
   "Could not reload BIOS. Press any button to exit.",
#elif (EMU_SYSTEM == NCDZ)
   "Could not compress state data.",
   "Could not uncompress state data.",
#endif
#endif

#ifdef COMMAND_LIST
   /* common/cmdlist.c */
   "Command list for this game not found.",
   "Command list - %s",
   "%d/%d items",
   "COMMAND.DAT size reduction",
   "This processing removes the command list of the games not being\n",
   "supported by this emulator from COMMAND.DAT.\n",
   "If you want to reduce the size of command.dat, press " FONT_CIRCLE " button.\n",
   "Otherwise, press " FONT_CROSS " button to return to file browser.\n",
   "Checking COMMAND.DAT format...\n",
   "ERROR: Unknown format.\n",
   "ERROR: Empty file.\n",
   "ERROR: Could not allocate memory.\n",
   "ERROR: Could not rename file.\n",
   "ERROR: Could create output file.\n",
   "Copying \"%s\"...\n",
   "Original size:%dbytes, Result:%dbytes (-%.1f%%)\n",
#endif

   /* emulation core */
   "Done.\n",
   "Exit emulation",
   "Please wait.\n",

   /* inptport.c */
   "Controller: Player 1",
   "Controller: Player %d",
#ifdef ADHOC
   "Lost sync.\n",
   "Paused by %s",
   "Return to Game",
   "Disconnect",
#endif

   /* memintrf.c */
   "Loading \"%s\"\n",
   "Load ROM",
#if (EMU_SYSTEM != NCDZ)
   "Checking ROM info...\n",
   "This game not supported.\n",
   "ROM not found. (zip file name incorrect)\n",
   "Driver for \"%s\" not found.\n",
   "ROM set \"%s\" (parent: %s)\n",
   "ROM set \"%s\"\n",
#endif

#if (EMU_SYSTEM == CPS1)

   /* memintrf.c */
   "rominfo.cps1 not found.\n",
   "Could not allocate memory. (0x8000 bytes)",

#elif (EMU_SYSTEM == CPS2)

   /* cps2crpt.c */
   "Decrypting %d%%\r",
   "Decrypting 100%%\n",

   /* memintrf.c */
   "rominfo.cps2 not found.\n",

#ifdef PSP_SLIM
   /* vidhrdw.c */
   "Decoding GFX...\n",
#endif

#elif (EMU_SYSTEM == MVS)

   /* biosmenu.c */
   "BIOS select menu",
   "BIOS not found.",
   "All NVRAM files are removed.\n",

   /* memintrf.c */
   "rominfo.mvs not found.\n",
   "Loading \"%s (%s)\"\n",
   "Decrypting ROM...\n",
   "Loading decrypted GFX2 ROM...\n",
   "Loading decrypted SOUND1 ROM...\n",
   "Could not allocate memory for decrypt ROM.\n",
   "Could not allocate memory for sprite data.\n",
   "Try to use sprite cache...\n",
   "Checking BIOS...\n",

#elif (EMU_SYSTEM == NCDZ)

   /* ncdz.c */
   "Error while processing IPL.TXT.",
   "Reset caused by watchdog counter.",

   /* cdrom.c */
   "Could not open file \"%s\".",
   "Could not insert CD-ROM state data.",

   /* vidhrdw.c */
   "CD-ROM speed limit: Off",
   "CD-ROM speed limit: On",

   /* memintrf.c */
   "Checking game ID...\n",
#endif
   "Cheat menu",
   "Cheat",
   "Select cheat",
   "Cheat for this game not found.",
/*
   "Memory free",
   "Mem free",*/
   NULL
};




