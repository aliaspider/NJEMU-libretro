/******************************************************************************

	inptport.c

	CPS2 ネ□ヲ・ンゥ`・ネ・ィ・゜・螂□`・キ・逾□

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	・ー・□`・ミ・□萍□
******************************************************************************/

int option_controller;
UINT16 cps2_port_value[CPS2_PORT_MAX];

int input_map[MAX_INPUTS];
int input_max_players;
int input_max_buttons;
int input_coin_chuter;
int analog_sensitivity;
int af_interval = 1;


/******************************************************************************
	・□`・ォ・□萍□
******************************************************************************/

static UINT8 ALIGN_DATA input_flag[MAX_INPUTS];
static int input_analog_value[2];
static int input_ui_wait;
static int service_switch;
static int p12_start_pressed;

static UINT8 max_players[COIN_MAX] =
{
	2,	// COIN_NONE: 2P 2・キ・蟀`・ソゥ`ケフカィ (・チ・ァ・テ・ッアリメェ、ハ、キ)

	2,	// COIN_2P1C: 2P 1・キ・蟀`・ソゥ`
	2,	// COIN_2P2C: 2P 2・キ・蟀`・ソゥ`

	3,	// COIN_3P1C: 3P 1・キ・蟀`・ソゥ`
	3,	// COIN_3P2C: 3P 2・キ・蟀`・ソゥ`
	3,	// COIN_3P3C: 3P 3・キ・蟀`・ソゥ`

	4,	// COIN_4P1C: 4P 1・キ・蟀`・ソゥ`
	4,	// COIN_4P2C: 4P 2・キ・蟀`・ソゥ`
	4	// COIN_4P4C: 4P 4・キ・蟀`・ソゥ`
};

static UINT8 coin_chuter[COIN_MAX][4] =
{
	{ 1, 2, 0, 0 },	// COIN_NONE: 2P 2・キ・蟀`・ソゥ`ケフカィ (・チ・ァ・テ・ッアリメェ、ハ、キ)

	{ 1, 1, 0, 0 },	// COIN_2P1C: 2P 1・キ・蟀`・ソゥ`
	{ 1, 2, 0, 0 },	// COIN_2P2C: 2P 2・キ・蟀`・ソゥ`

	{ 1, 1, 1, 0 },	// COIN_3P1C: 3P 1・キ・蟀`・ソゥ`
	{ 1, 1, 2, 0 },	// COIN_3P2C: 3P 2・キ・蟀`・ソゥ`
	{ 1, 2, 3, 0 },	// COIN_3P3C: 3P 3・キ・蟀`・ソゥ`

	{ 1, 1, 1, 1 },	// COIN_4P1C: 4P 1・キ・蟀`・ソゥ`
	{ 1, 1, 2, 2 },	// COIN_3P2C: 4P 2・キ・蟀`・ソゥ`
	{ 1, 2, 3, 4 }	// COIN_3P3C: 4P 4・キ・蟀`・ソゥ`
};


/******************************************************************************
	・□`・ォ・□vハ□
******************************************************************************/

/*------------------------------------------------------
	EEPROM、ホ・ウ・、・□Oカィ、□チ・ァ・テ・ッ
------------------------------------------------------*/

static void check_eeprom_settings(int popup)
{
	UINT8 eeprom_value = EEPROM_read_data(driver->inp_eeprom);
	UINT8 coin_type = driver->inp_eeprom_value[eeprom_value];

	if (input_coin_chuter != coin_type)
	{
		input_coin_chuter = coin_type;
		input_max_players = max_players[coin_type];

		if (option_controller >= input_max_players)
		{
			option_controller = INPUT_PLAYER1;
			if (popup) msg_printf(TEXT(CONTROLLER_PLAYER1));
		}
	}
}


/*------------------------------------------------------
	CPS2 ・ンゥ`・ネ0 (・ウ・□ネ・□`・□ / 2)
------------------------------------------------------*/

static void update_inputport0(void)
{
	UINT16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_19xx:
	case INPTYPE_batcir:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_12])		 value &= ~0x0030;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_12])		 value &= ~0x3000;
		}
		break;

	case INPTYPE_cps2:
	case INPTYPE_cybots:
	case INPTYPE_ssf2:
	case INPTYPE_avsp:
	case INPTYPE_sgemf:
	case INPTYPE_daimahoo:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
			if (input_flag[P1_12])		 value &= ~0x0030;
			if (input_flag[P1_13])		 value &= ~0x0050;
			if (input_flag[P1_23])		 value &= ~0x0060;
			if (input_flag[P1_123])     value &= ~0x0070;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
			if (input_flag[P1_12])		 value &= ~0x3000;
			if (input_flag[P1_13])		 value &= ~0x5000;
			if (input_flag[P1_23])		 value &= ~0x6000;
			if (input_flag[P1_123])     value &= ~0x7000;
		}
		break;

	case INPTYPE_ddtod:
	case INPTYPE_pzloop2:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
			if (input_flag[P1_BUTTON4]) value &= ~0x0080;
			if (input_flag[P1_12])		 value &= ~0x0030;
			if (input_flag[P1_13])		 value &= ~0x0050;
			if (input_flag[P1_23])		 value &= ~0x0060;
			if (input_flag[P1_24])		 value &= ~0x00a0;
			if (input_flag[P1_34])		 value &= ~0x00c0;
			if (input_flag[P1_123])	 value &= ~0x0070;
			if (input_flag[P1_124])	 value &= ~0x00b0;
			if (input_flag[P1_134])	 value &= ~0x00d0;
			if (input_flag[P1_234])	 value &= ~0x00e0;
			if (input_flag[P1_1234])	 value &= ~0x00f0;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
			if (input_flag[P1_BUTTON4]) value &= ~0x8000;
			if (input_flag[P1_123])	 value &= ~0x7000;
			if (input_flag[P1_124])	 value &= ~0xb000;
			if (input_flag[P1_134])	 value &= ~0xd000;
			if (input_flag[P1_234])	 value &= ~0xe000;
			if (input_flag[P1_1234])	 value &= ~0xf000;
		}
		break;

	case INPTYPE_qndream:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0001;
			if (input_flag[P1_BUTTON3]) value &= ~0x0002;
			if (input_flag[P1_BUTTON2]) value &= ~0x0004;
			if (input_flag[P1_BUTTON1]) value &= ~0x0008;
			if (input_flag[P1_12])		 value &= ~0x000c;
			if (input_flag[P1_13])		 value &= ~0x000a;
			if (input_flag[P1_23])		 value &= ~0x0006;
			if (input_flag[P1_24])		 value &= ~0x0005;
			if (input_flag[P1_34])		 value &= ~0x0003;
			if (input_flag[P1_123])	 value &= ~0x000e;
			if (input_flag[P1_124])	 value &= ~0x000d;
			if (input_flag[P1_134])	 value &= ~0x000b;
			if (input_flag[P1_234])	 value &= ~0x0007;
			if (input_flag[P1_1234])	 value &= ~0x000f;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0100;
			if (input_flag[P1_BUTTON3]) value &= ~0x0200;
			if (input_flag[P1_BUTTON2]) value &= ~0x0400;
			if (input_flag[P1_BUTTON1]) value &= ~0x0800;
			if (input_flag[P1_12])		 value &= ~0x0c00;
			if (input_flag[P1_13])		 value &= ~0x0a00;
			if (input_flag[P1_23])		 value &= ~0x0600;
			if (input_flag[P1_24])		 value &= ~0x0500;
			if (input_flag[P1_34])		 value &= ~0x0300;
			if (input_flag[P1_123])	 value &= ~0x0e00;
			if (input_flag[P1_124])	 value &= ~0x0d00;
			if (input_flag[P1_134])	 value &= ~0x0b00;
			if (input_flag[P1_234])	 value &= ~0x0700;
			if (input_flag[P1_1234])	 value &= ~0x0f00;
		}
		break;
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[0] = value;
	else
#endif
		cps2_port_value[0] = value;
}


/*------------------------------------------------------
	CPS2 ・ンゥ`・ネ1 (・ウ・□ネ・□`・□ / 4 / ラキシモ・ワ・ソ・□
------------------------------------------------------*/

static void update_inputport1(void)
{
	UINT16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_cybots:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0001;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0100;
		}
		break;

	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0001;
			if (input_flag[P1_BUTTON5]) value &= ~0x0002;
			if (input_flag[P1_BUTTON6]) value &= ~0x0004;
			if (input_flag[P1_456])     value &= ~0x0007;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0010;
			if (input_flag[P1_BUTTON5]) value &= ~0x0020;
			if (input_flag[P1_456])     value &= ~0x0030;
		}
		break;

	case INPTYPE_batcir:
		if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_12])		 value &= ~0x0030;
		}
		else if (option_controller == INPUT_PLAYER4)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_12])		 value &= ~0x3000;
		}
		break;

	case INPTYPE_avsp:
		if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
			if (input_flag[P1_12])		 value &= ~0x0030;
			if (input_flag[P1_13])		 value &= ~0x0050;
			if (input_flag[P1_23])		 value &= ~0x0060;
		}
		break;

	case INPTYPE_ddtod:
		if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
			if (input_flag[P1_BUTTON4]) value &= ~0x0080;
			if (input_flag[P1_12])		 value &= ~0x0030;
			if (input_flag[P1_13])		 value &= ~0x0050;
			if (input_flag[P1_23])		 value &= ~0x0060;
			if (input_flag[P1_24])		 value &= ~0x00a0;
			if (input_flag[P1_34])		 value &= ~0x00c0;
			if (input_flag[P1_123])	 value &= ~0x0070;
			if (input_flag[P1_124])	 value &= ~0x00b0;
			if (input_flag[P1_134])	 value &= ~0x00d0;
			if (input_flag[P1_234])	 value &= ~0x00e0;
			if (input_flag[P1_1234])	 value &= ~0x00f0;
		}
		else if (option_controller == INPUT_PLAYER4)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
			if (input_flag[P1_BUTTON4]) value &= ~0x8000;
			if (input_flag[P1_12])		 value &= ~0x3000;
			if (input_flag[P1_13])		 value &= ~0x5000;
			if (input_flag[P1_23])		 value &= ~0x6000;
			if (input_flag[P1_24])		 value &= ~0xa000;
			if (input_flag[P1_34])		 value &= ~0xc000;
			if (input_flag[P1_123])	 value &= ~0x7000;
			if (input_flag[P1_124])	 value &= ~0xb000;
			if (input_flag[P1_134])	 value &= ~0xd000;
			if (input_flag[P1_234])	 value &= ~0xe000;
			if (input_flag[P1_1234])	 value &= ~0xf000;
		}
		break;
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[1] = value;
	else
#endif
		cps2_port_value[1] = value;
}


/*------------------------------------------------------
	CPS2 ・ンゥ`・ネ2 (START / COIN)
------------------------------------------------------*/

static void update_inputport2(void)
{
	UINT16 value = 0xffff;

	if (input_flag[SERV_SWITCH] || service_switch)
	{
		value &= ~0x0002;
	}
	if (input_flag[SERV_COIN])
	{
		value &= ~0x0004;
	}
	if (p12_start_pressed)
	{
		value &= ~(0x0100 | 0x0200);
	}

	if (input_flag[P1_COIN])
	{
		switch (coin_chuter[input_coin_chuter][option_controller])
		{
		case 1: value &= ~0x1000; break;
		case 2: value &= ~0x2000; break;
		case 3: value &= ~0x4000; break;
		case 4: value &= ~0x8000; break;
		}
	}

	if (option_controller == INPUT_PLAYER1)
	{
		if (input_flag[P1_START]) value &= ~0x0100;
		if (input_flag[P2_START]) value &= ~0x0200;
	}
	else if (option_controller == INPUT_PLAYER2)
	{
		if (input_flag[P1_START]) value &= ~0x0200;
		if (input_flag[P2_START]) value &= ~0x0100;
	}

	switch (machine_input_type)
	{
	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		if (option_controller == INPUT_PLAYER2)
		{
			// Player 2 button 6
			if (input_flag[P1_BUTTON6]) value &= ~0x4000;
			if (input_flag[P1_456])     value &= ~0x4000;
		}
		break;

	case INPTYPE_avsp:
		if (option_controller == INPUT_PLAYER3)
		{
			// Player 3 start
			if (input_flag[P1_START]) value &= ~0x0400;
		}
		break;

	case INPTYPE_ddtod:
	case INPTYPE_batcir:
		if (option_controller == INPUT_PLAYER3)
		{
			// Player 3 start
			if (input_flag[P1_START]) value &= ~0x0400;
		}
		else if (option_controller == INPUT_PLAYER4)
		{
			// Player 4 start
			if (input_flag[P1_START]) value &= ~0x0800;
		}
		break;
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[2] = value;
	else
#endif
		cps2_port_value[2] = value;
}


/*------------------------------------------------------
	pzloop2 ・「・ハ・□ーネ□ヲ・ンゥ`・ネ
------------------------------------------------------*/

static void update_inputport3(void)
{
	int delta = 0;

	if (input_flag[P1_DIAL_L])
	{
		switch (analog_sensitivity)
		{
		case 0: delta -= 10; break;
		case 1: delta -= 15; break;
		case 2: delta -= 20; break;
		}
	}
	if (input_flag[P1_DIAL_R])
	{
		switch (analog_sensitivity)
		{
		case 0: delta += 10; break;
		case 1: delta += 15; break;
		case 2: delta += 20; break;
		}
	}
	input_analog_value[option_controller] = (input_analog_value[option_controller] + delta) & 0xff;

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[0] = input_analog_value[0] | (input_analog_value[1] << 8);
	else
#endif
		cps2_port_value[3] = input_analog_value[0] | (input_analog_value[1] << 8);
}


/*------------------------------------------------------
	ネ□ヲ・ワ・ソ・□□ュテ豺スマ□ヒコマ、□サ、ニユ{ユ□
------------------------------------------------------*/

static UINT32 adjust_input(UINT32 buttons)
{
	UINT32 buttons2;

	if (!cps_flip_screen && machine_screen_type != SCREEN_VERTICAL)
		return buttons;

	if (!machine_screen_type)
	{
		if (cps_flip_screen)
		{
			buttons2 = buttons & (PSP_CTRL_START | PSP_CTRL_SELECT);

			if (buttons & PSP_CTRL_UP)       buttons2 |= PSP_CTRL_DOWN;
			if (buttons & PSP_CTRL_DOWN)     buttons2 |= PSP_CTRL_UP;
			if (buttons & PSP_CTRL_RIGHT)    buttons2 |= PSP_CTRL_LEFT;
			if (buttons & PSP_CTRL_LEFT)     buttons2 |= PSP_CTRL_RIGHT;
			if (buttons & PSP_CTRL_SQUARE)   buttons2 |= PSP_CTRL_CIRCLE;
			if (buttons & PSP_CTRL_CIRCLE)   buttons2 |= PSP_CTRL_SQUARE;
			if (buttons & PSP_CTRL_TRIANGLE) buttons2 |= PSP_CTRL_CROSS;
			if (buttons & PSP_CTRL_CROSS)    buttons2 |= PSP_CTRL_TRIANGLE;
			if (buttons & PSP_CTRL_RTRIGGER) buttons2 |= PSP_CTRL_LTRIGGER;
			if (buttons & PSP_CTRL_LTRIGGER) buttons2 |= PSP_CTRL_RTRIGGER;

			buttons = buttons2;
		}
	}
	else
	{
		if (!cps_rotate_screen)
		{
			buttons2 = buttons & (PSP_CTRL_START | PSP_CTRL_SELECT | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER);

			if (buttons & PSP_CTRL_UP)       buttons2 |= PSP_CTRL_LEFT;
			if (buttons & PSP_CTRL_DOWN)     buttons2 |= PSP_CTRL_RIGHT;
			if (buttons & PSP_CTRL_RIGHT)    buttons2 |= PSP_CTRL_UP;
			if (buttons & PSP_CTRL_LEFT)     buttons2 |= PSP_CTRL_DOWN;
			if (buttons & PSP_CTRL_TRIANGLE) buttons2 |= PSP_CTRL_SQUARE;
			if (buttons & PSP_CTRL_CIRCLE)   buttons2 |= PSP_CTRL_TRIANGLE;
			if (buttons & PSP_CTRL_SQUARE)   buttons2 |= PSP_CTRL_CROSS;
			if (buttons & PSP_CTRL_CROSS)    buttons2 |= PSP_CTRL_CIRCLE;

			buttons = buttons2;
		}

		if (cps_flip_screen)
		{
			buttons2 = buttons & (PSP_CTRL_START | PSP_CTRL_SELECT);

			if (buttons & PSP_CTRL_UP)       buttons2 |= PSP_CTRL_DOWN;
			if (buttons & PSP_CTRL_DOWN)     buttons2 |= PSP_CTRL_UP;
			if (buttons & PSP_CTRL_RIGHT)    buttons2 |= PSP_CTRL_LEFT;
			if (buttons & PSP_CTRL_LEFT)     buttons2 |= PSP_CTRL_RIGHT;
			if (buttons & PSP_CTRL_SQUARE)   buttons2 |= PSP_CTRL_CIRCLE;
			if (buttons & PSP_CTRL_CIRCLE)   buttons2 |= PSP_CTRL_SQUARE;
			if (buttons & PSP_CTRL_TRIANGLE) buttons2 |= PSP_CTRL_CROSS;
			if (buttons & PSP_CTRL_CROSS)    buttons2 |= PSP_CTRL_TRIANGLE;
			if (buttons & PSP_CTRL_RTRIGGER) buttons2 |= PSP_CTRL_LTRIGGER;
			if (buttons & PSP_CTRL_LTRIGGER) buttons2 |= PSP_CTRL_RTRIGGER;

			buttons = buttons2;
		}
	}

	return buttons;
}


/******************************************************************************
	ネ□ヲ・ンゥ`・ネ・、・□ソ・ユ・ァゥ`・ケ騅ハ□
******************************************************************************/

/*------------------------------------------------------
	ネ□ヲ・ンゥ`・ネ、ホウ□レサッ
------------------------------------------------------*/

int input_init(void)
{
	input_ui_wait = 0;
	service_switch = 0;
	p12_start_pressed = 0;

	memset(cps2_port_value, 0xff, sizeof(cps2_port_value));	
	memset(input_flag, 0, sizeof(input_flag));

	input_analog_value[0] = 0;
	input_analog_value[1] = 0;

	switch (machine_input_type)
	{
	case INPTYPE_avsp:
		input_max_players = 3;
		break;

	case INPTYPE_ddtod:
	case INPTYPE_batcir:
		input_max_players = 4;
		break;

	default:
		input_max_players = 2;
		break;
	}

	switch (machine_input_type)
	{
	case INPTYPE_19xx:
	case INPTYPE_batcir:
		input_max_buttons = 2;
		break;

	case INPTYPE_cybots:
	case INPTYPE_ddtod:
	case INPTYPE_qndream:
	case INPTYPE_pzloop2:
		input_max_buttons = 4;
		break;

	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		input_max_buttons = 6;
		break;

	default:
		input_max_buttons = 3;
		break;
	}

	input_coin_chuter = COIN_NONE;


   input_map[P1_UP]=PSP_CTRL_UP;
	input_map[P1_DOWN]=PSP_CTRL_DOWN;
	input_map[P1_LEFT]=PSP_CTRL_LEFT;
	input_map[P1_RIGHT]=PSP_CTRL_RIGHT;
	input_map[P1_BUTTON1]=PSP_CTRL_CROSS;
	input_map[P1_BUTTON2]=PSP_CTRL_LTRIGGER;
	input_map[P1_BUTTON3]=PSP_CTRL_SQUARE;
	input_map[P1_BUTTON4]=PSP_CTRL_CIRCLE;
	input_map[P1_BUTTON5]=PSP_CTRL_RTRIGGER;
	input_map[P1_BUTTON6]=PSP_CTRL_TRIANGLE;
	input_map[P1_START]=PSP_CTRL_START;
	input_map[P1_COIN]=PSP_CTRL_SELECT;

	return 1;
}



/*------------------------------------------------------
	ネ□ヲ・ンゥ`・ネ、□□サ・テ・ネ
------------------------------------------------------*/

void input_reset(void)
{
	memset(cps2_port_value, 0xff, sizeof(cps2_port_value));
	input_analog_value[0] = 0;
	input_analog_value[1] = 0;
	service_switch = 0;
	p12_start_pressed = 0;


	if (driver->inp_eeprom) check_eeprom_settings(0);

}



/*------------------------------------------------------
	ネ□ヲ・ンゥ`・ネ、□□ツ
------------------------------------------------------*/

void update_inputport(void)
{
	int i;
	UINT32 buttons;

   service_switch = 0;
   p12_start_pressed = 0;

   if (driver->inp_eeprom) check_eeprom_settings(1);

   buttons = poll_gamepad();

   if ((buttons & PSP_CTRL_LTRIGGER) && (buttons & PSP_CTRL_RTRIGGER))
   {
      if (buttons & PSP_CTRL_SELECT)
      {
         buttons &= ~(PSP_CTRL_SELECT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER);
         service_switch = 1;
      }
      else if (buttons & PSP_CTRL_START)
      {
         buttons &= ~(PSP_CTRL_START | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER);
         p12_start_pressed = 1;
      }
   }

   buttons = adjust_input(buttons);

   for (i = 0; i < MAX_INPUTS; i++)
      input_flag[i] = (buttons & input_map[i]) != 0;

   update_inputport0();
   update_inputport1();
   update_inputport2();
   if (machine_input_type == INPTYPE_pzloop2) update_inputport3();

   if (input_flag[SWPLAYER])
   {
      if (!input_ui_wait)
      {
         option_controller++;
         if (option_controller == input_max_players)
            option_controller = INPUT_PLAYER1;
         msg_printf(TEXT(CONTROLLER_PLAYERx), option_controller + 1);
         input_ui_wait = 30;
      }
   }


   if (input_ui_wait > 0) input_ui_wait--;

}


/******************************************************************************
	・サゥ`・ヨ/・□`・ノ ・ケ・ニゥ`・ネ
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( input )
{
	state_save_long(&option_controller, 1);
	state_save_long(&input_analog_value[0], 1);
	state_save_long(&input_analog_value[1], 1);
}

STATE_LOAD( input )
{
	state_load_long(&option_controller, 1);
	state_load_long(&input_analog_value[0], 1);
	state_load_long(&input_analog_value[1], 1);

	setup_autofire();
	input_ui_wait = 0;
	p12_start_pressed = 0;
	service_switch = 0;
}

#endif /* SAVE_STATE */
