/******************************************************************************

	inptport.c

	CPS1 ネ□ヲ・ンゥ`・ネ・ィ・゜・螂□`・キ・逾□

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	・ー・□`・ミ・□萍□
******************************************************************************/

int option_controller;
int cps1_dipswitch[3];
UINT16 ALIGN_DATA cps1_port_value[CPS1_PORT_MAX];
int ALIGN_DATA input_map[MAX_INPUTS];
int input_max_players;
int input_max_buttons;
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

/******************************************************************************
	・□`・ォ・□vハ□
******************************************************************************/

/*------------------------------------------------------
	CPS1 ・ンゥ`・ネ0 (START / COIN)
------------------------------------------------------*/

static void update_inputport0(void)
{
	UINT16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_sfzch:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON5]) value &= ~0x0101;
			if (input_flag[P1_COIN])    value &= ~0x0404;
			if (input_flag[P1_START])   value &= ~0x1010;
			if (input_flag[P1_BUTTON6]) value &= ~0x4040;
			if (input_flag[P1_456])     value &= ~0x4141;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON5]) value &= ~0x0202;
			if (input_flag[P1_COIN])    value &= ~0x0808;
			if (input_flag[P1_START])   value &= ~0x2020;
			if (input_flag[P1_BUTTON6]) value &= ~0x8080;
			if (input_flag[P1_456])     value &= ~0x8282;
		}
		break;

	case INPTYPE_wofch:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON5]) value &= ~0x0101;
			if (input_flag[P1_COIN])    value &= ~0x0404;
			if (input_flag[P1_START])   value &= ~0x1010;
			if (input_flag[P1_BUTTON6]) value &= ~0x4040;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON5]) value &= ~0x0202;
			if (input_flag[P1_COIN])    value &= ~0x0808;
			if (input_flag[P1_START])   value &= ~0x2020;
			if (input_flag[P1_BUTTON6]) value &= ~0x8080;
		}
		break;

#if !RELEASE
	case INPTYPE_wofh:
	case INPTYPE_wofsj:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_COIN])    value &= ~0x0100;
			if (input_flag[P1_START])   value &= ~0x1000;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_COIN])    value &= ~0x0200;
			if (input_flag[P1_START])   value &= ~0x2000;
		}
		else if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_COIN])    value &= ~0x0040;
			if (input_flag[P1_START])   value &= ~0x0080;
			if (input_flag[P1_12])		 value &= ~0x0030;
		}
		if (input_flag[SERV_COIN])
		{
			value &= ~0x0400;
		}
		if (p12_start_pressed)
		{
		value &= ~(0x1000 | 0x2000);
		}
		break;
#endif

	default:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_COIN])  value &= ~0x0101;
			if (input_flag[P1_START]) value &= ~0x1010;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_COIN])  value &= ~0x0202;
			if (input_flag[P1_START]) value &= ~0x2020;
		}
		if (input_flag[SERV_COIN])
		{
			value &= ~0x0400;
		}
		if (p12_start_pressed)
		{
		value &= ~(0x1010 | 0x2020);
		}
		break;
	}

	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
	case INPTYPE_mercs:
	case INPTYPE_sfzch:
#if !RELEASE
	case INPTYPE_wofh:
	case INPTYPE_wofsj:
#endif
		break;

	case INPTYPE_ghouls:
	case INPTYPE_ghoulsu:
	case INPTYPE_daimakai:
		if (input_flag[SERV_SWITCH])
		{
			if (!input_ui_wait)
			{
				service_switch ^= 0x4040;
				if (service_switch == 0)
					Loop = LOOP_RESTART;
				else
					Loop = LOOP_RESET;
				input_ui_wait = 30;
			}
		}
		value &= ~service_switch;
		break;

	case INPTYPE_strider:
	case INPTYPE_stridrua:
	case INPTYPE_dynwar:
	case INPTYPE_ffight:
	case INPTYPE_1941:
		if (input_flag[SERV_SWITCH])
		{
			if (!input_ui_wait)
			{
				service_switch ^= 0x4040;
				Loop = LOOP_RESET;
				input_ui_wait = 30;
			}
		}
		value &= ~service_switch;
		break;

	default:
		if (input_flag[SERV_SWITCH])
		{
			value &= ~0x4040;
		}
		break;
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[0] = value;
	else
#endif
		cps1_port_value[0] = value;
}


/*------------------------------------------------------
	CPS1 ・ンゥ`・ネ1 (・ウ・□ネ・□`・□/2)
------------------------------------------------------*/

static void update_inputport1(void)
{
	UINT16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_dynwar:
	case INPTYPE_ffight:	// button 3 (cheat)
	case INPTYPE_mtwins:
//	case INPTYPE_3wonders:// button 3 (cheat)
	case INPTYPE_pnickj:
	case INPTYPE_pang3:
	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
	case INPTYPE_sf2:
	case INPTYPE_sf2j:
#if !RELEASE
	case INPTYPE_kodh:
	case INPTYPE_knightsh:
	case INPTYPE_wofh:
	case INPTYPE_wof3js:
	case INPTYPE_dinoh:
	case INPTYPE_punisherbz:
#endif
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
		}
		break;

	case INPTYPE_cworld2j:
	case INPTYPE_qad:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
		if (option_controller == INPUT_PLAYER1)
		{
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

	case INPTYPE_forgottn:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
		}
		break;

	case INPTYPE_slammast:
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
		}
		else if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_BUTTON3]) value &= ~0x0080;
		}
		else if (option_controller == INPUT_PLAYER4)
		{
			if (input_flag[P1_BUTTON3]) value &= ~0x8000;
		}
		break;

	case INPTYPE_sfzch:
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
			if (input_flag[P1_456])     value &= ~0x0080;
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
			if (input_flag[P1_456])     value &= ~0x8000;
		}

	case INPTYPE_wofch:
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
		}
		break;

	default:
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
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[1] = value;
	else
#endif
		cps1_port_value[1] = value;
}


/*------------------------------------------------------
	CPS1 ・ンゥ`・ネ2 (・ウ・□ネ・□`・□ / ラキシモ・ワ・ソ・□
------------------------------------------------------*/

static void update_inputport2(void)
{
	UINT16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_mercs:
	case INPTYPE_kod:
	case INPTYPE_kodj:
	case INPTYPE_knights:
	case INPTYPE_wof:
	case INPTYPE_dino:
	case INPTYPE_captcomm:
	case INPTYPE_slammast:
#if !RELEASE
	case INPTYPE_kodh:
	case INPTYPE_knightsh:
	case INPTYPE_wof3js:
	case INPTYPE_dinoh:
#endif
		if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0101;
			if (input_flag[P1_LEFT])    value &= ~0x0202;
			if (input_flag[P1_DOWN])    value &= ~0x0404;
			if (input_flag[P1_UP])      value &= ~0x0808;
			if (input_flag[P1_BUTTON1]) value &= ~0x1010;
			if (input_flag[P1_BUTTON2]) value &= ~0x2020;
			if (input_flag[P1_COIN])    value &= ~0x4040;
			if (input_flag[P1_START])   value &= ~0x8080;
			if (input_flag[P1_12])		 value &= ~0x3030;
		}
		break;

	case INPTYPE_sf2:
	case INPTYPE_sf2j:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0101;
			if (input_flag[P1_BUTTON5]) value &= ~0x0202;
			if (input_flag[P1_BUTTON6]) value &= ~0x0404;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x1010;
			if (input_flag[P1_BUTTON5]) value &= ~0x2020;
			if (input_flag[P1_BUTTON6]) value &= ~0x4040;
		}
		break;
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[2] = value;
	else
#endif
		cps1_port_value[2] = value;
}


/*------------------------------------------------------
	CPS1 ・ンゥ`・ネ3 (・ウ・□ネ・□`・□)
------------------------------------------------------*/

static void update_inputport3(void)
{
	UINT16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_captcomm:
	case INPTYPE_slammast:
		if (option_controller == INPUT_PLAYER4)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0101;
			if (input_flag[P1_LEFT])    value &= ~0x0202;
			if (input_flag[P1_DOWN])    value &= ~0x0404;
			if (input_flag[P1_UP])      value &= ~0x0808;
			if (input_flag[P1_BUTTON1]) value &= ~0x1010;
			if (input_flag[P1_BUTTON2]) value &= ~0x2020;
			if (input_flag[P1_COIN])    value &= ~0x4040;
			if (input_flag[P1_START])   value &= ~0x8080;
			if (input_flag[P1_12])		 value &= ~0x3030;
		}
		break;
	}

#ifdef ADHOC
	if (adhoc_enable)
		send_data.port_value[3] = value;
	else
#endif
		cps1_port_value[3] = value;
}


/*------------------------------------------------------
	forgottn ・「・ハ・□ーネ□ヲ・ンゥ`・ネ
------------------------------------------------------*/

static void forgottn_update_dial(void)
{
	int delta = 0;

	if (input_flag[P1_DIAL_L])
	{
		switch (analog_sensitivity)
		{
		case 0: delta -= 10; break;
		case 1: delta -= 20; break;
		case 2: delta -= 30; break;
		}
	}
	if (input_flag[P1_DIAL_R])
	{
		switch (analog_sensitivity)
		{
		case 0: delta += 10; break;
		case 1: delta += 20; break;
		case 2: delta += 30; break;
		}
	}
#ifdef ADHOC
	if (adhoc_enable)
	{
		delta *= 2;
		send_data.port_value[4 + option_controller] = (input_analog_value[option_controller] + delta) & 0xfff;
	}
	else
#endif
		input_analog_value[option_controller] = (input_analog_value[option_controller] + delta) & 0xfff;
}


UINT16 forgottn_read_dial0(void)
{
	return input_analog_value[0];
}

UINT16 forgottn_read_dial1(void)
{
	return input_analog_value[1];
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
	
	memset(cps1_port_value, 0xff, sizeof(cps1_port_value));	

	input_analog_value[0] = 0;
	input_analog_value[1] = 0;

	switch (machine_input_type)
	{
	case INPTYPE_mercs:
	case INPTYPE_kod:
	case INPTYPE_kodj:
	case INPTYPE_knights:
	case INPTYPE_wof:
	case INPTYPE_dino:
#if !RELEASE
	case INPTYPE_kodh:
	case INPTYPE_knightsh:
	case INPTYPE_wofh:
	case INPTYPE_wof3js:
	case INPTYPE_wofsj:
	case INPTYPE_dinoh:
#endif
		input_max_players = 3;
		break;

	case INPTYPE_captcomm:
	case INPTYPE_slammast:
		input_max_players = 4;
		break;

	default:
		input_max_players = 2;
		break;
	}

	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
		input_max_buttons = 1;
		break;

	case INPTYPE_dynwar:
	case INPTYPE_ffight:	// button 3 (cheat)
	case INPTYPE_mtwins:
//	case INPTYPE_3wonders:// button 3 (cheat)
	case INPTYPE_pnickj:
	case INPTYPE_pang3:
	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
	case INPTYPE_slammast:
#if !RELEASE
	case INPTYPE_kodh:
	case INPTYPE_knightsh:
	case INPTYPE_wofh:
	case INPTYPE_wof3js:
	case INPTYPE_wofsj:
	case INPTYPE_dinoh:
	case INPTYPE_punisherbz:
#endif
		input_max_buttons = 3;
		break;

	case INPTYPE_cworld2j:
	case INPTYPE_qad:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
		input_max_buttons = 4;
		break;

	case INPTYPE_sf2:
	case INPTYPE_sf2j:
	case INPTYPE_sfzch:
		input_max_buttons = 6;
		break;

	default:
		input_max_buttons = 2;
		break;
	}

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
	memset(cps1_port_value, 0xff, sizeof(cps1_port_value));
	input_analog_value[0] = 0;
	input_analog_value[1] = 0;
	p12_start_pressed = 0;

}


/*------------------------------------------------------
	ネ□ヲ・ンゥ`・ネ、□□ツ
------------------------------------------------------*/

void update_inputport(void)
{
	int i, serv_switch = 0;
	UINT32 buttons;


   service_switch = 0;
   p12_start_pressed = 0;

   buttons = poll_gamepad();

   if ((buttons & PSP_CTRL_LTRIGGER) && (buttons & PSP_CTRL_RTRIGGER))
   {
      if (buttons & PSP_CTRL_SELECT)
      {
         buttons &= ~(PSP_CTRL_SELECT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER);
         serv_switch = 1;
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

   if (serv_switch) input_flag[SERV_SWITCH] = 1;

   update_inputport0();
   update_inputport1();
   update_inputport2();
   update_inputport3();
   if (machine_input_type == INPTYPE_forgottn) forgottn_update_dial();

   if (input_flag[SWPLAYER])
   {
      if (!input_ui_wait)
      {
         option_controller++;
         if (option_controller == input_max_players)
            option_controller = INPUT_PLAYER1;
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
	state_save_long(&cps1_dipswitch[0], 1);
	state_save_long(&cps1_dipswitch[1], 1);
	state_save_long(&cps1_dipswitch[2], 1);
	state_save_byte(&service_switch, 1);
}

STATE_LOAD( input )
{
	state_load_long(&option_controller, 1);
	state_load_long(&input_analog_value[0], 1);
	state_load_long(&input_analog_value[1], 1);
	state_load_long(&cps1_dipswitch[0], 1);
	state_load_long(&cps1_dipswitch[1], 1);
	state_load_long(&cps1_dipswitch[2], 1);
	state_load_byte(&service_switch, 1);

	input_ui_wait = 0;
	p12_start_pressed = 0;
	service_switch = 0;
}

#endif /* SAVE_STATE */
