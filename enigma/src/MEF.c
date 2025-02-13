/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2025/01/10
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"
#include "plugb.h"
#include "rotary_encoder.h"
#include "enigmaAPI.h"
#include "PS2Keyboard.h"
#include "animation.h"
#include "MEF.h"

/*=====[Definition macros of private constants]==============================*/
#define NUM_ROTORS 3
#define PLUGB_DELAY 500
#define ROTOR_ANIM_DELAY 700

/* Pins used for data and clock from keyboard */
#define IRQ_PIN  T_COL2
#define DATA_PIN T_FIL1

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
typedef enum { ENCRYPT, CONFIG_PB, CONFIG_ROTOR } MEF_state_t;
static MEF_state_t state;

static void MEF_ConfigRotor();
static void MEF_Encrypt();
static void MEF_ConfigPB();
static void (*MEF_Behavior[])(void) = { MEF_Encrypt, MEF_ConfigPB, MEF_ConfigRotor };

static delay_t plugbDelay;
static delay_t rotorAnimDelay;
static bool_t keyPressed = false;
static bool_t pressMsgDone = false;
static uint8_t waitAnimTimes = 3;
static bool_t loadAnimDone = true;
static bool_t rotorAnimDone = false;

static char* const plugbMessage = "PLUG ";
static char* const encryptMessage = "PRESS A KEY ";

static uint8_t rotorIndex = 0;
static uint8_t rotorPos[NUM_ROTORS] = { 0 };

static char out;

void MEF_Init()
{
	PLUGB_Init();

	RotaryEncoder_Init();
	delayInit( &rotorAnimDelay, 500 );

	PS2Keyboard_Init(DATA_PIN, IRQ_PIN, 0);

	Animation_Init();

	state = ENCRYPT;
	out = 0;
	EnigmaAPI_Init( 3, 2, 1, 1, rotorPos[0], rotorPos[1], rotorPos[2] );
	Animation_WaitInput(true);
	pressMsgDone = Animation_ShiftText(encryptMessage, true);
}

void MEF_Update(void) {
	Animation_Loading(true);
	loadAnimDone = false;

	if( state == CONFIG_ROTOR && rotorIndex != (NUM_ROTORS - 1) )
	{
		rotorIndex++;
	}
	else
	{
		if( state == CONFIG_ROTOR ) {
			rotorIndex = 0;
		}
		else if ( state == ENCRYPT )
		{
			PS2Keyboard_DisableInt();
		}
		state++;
		state %= 3;
	}

	switch ( state ) {
		case ENCRYPT:
			out = 0;
			EnigmaAPI_SetPlugboardMapping( PLUGB_GetAllMappings() );
			EnigmaAPI_Init( 3, 2, 1, 1, rotorPos[0], rotorPos[1], rotorPos[2] );
			PS2Keyboard_EnableInt();
			keyPressed = false;
			Animation_WaitInput(true);
			pressMsgDone = Animation_ShiftText(encryptMessage, true);
			printf( "Modo encriptacion \r\n" );
			break;
		case CONFIG_PB:
			Animation_ShiftText(plugbMessage, true);
			delayInit( &plugbDelay, PLUGB_DELAY );
			printf( "Configurando plugboard \r\n" );
			break;
		case CONFIG_ROTOR:
			rotorPos[rotorIndex] = EnigmaAPI_GetRotorValue(rotorIndex);
			delayInit( &rotorAnimDelay, ROTOR_ANIM_DELAY );
			rotorAnimDone = false;
			printf( "Configurando rotor %d \r\n", rotorIndex + 1);
			break;
	}
}

static void MEF_Encrypt()
{
	if ( PS2Keyboard_Available() )
	{
		uint16_t c = PS2Keyboard_Read();
		if (c > 0) {
			printf("Value ");
			if ('A' <= c && c <= 'Z')
			{
				printf("%c", c);
			}
			else {
				printf("%x", c);
			}
			printf(" - Status Bits ");
			printf("%x", c >> 8);
			printf("  Code ");
			printf("%x", c & 0xFF);

			if ('A' <= c && c <= 'Z')
			{
				keyPressed = true;
				out = EnigmaAPI_EncryptChar(c);
				printf(" - out : %c", out);
				Animation_DrawCharacter(out);
			}

			printf("\r\n");
		}
	}
	else if ( !keyPressed )
	{
		if ( !pressMsgDone )
		{
			pressMsgDone = Animation_ShiftText(encryptMessage, false);
		}
		else
		{
			if ( Animation_WaitInput(false) )
			{
				--waitAnimTimes;
				if ( waitAnimTimes == 0 )
				{
					waitAnimTimes = 3;
					Animation_WaitInput(true);
					pressMsgDone = false;
				}
			}
		}
	}
}

static void MEF_ConfigPB()
{
	if ( delayRead( &plugbDelay ) )
	{
		PLUGB_Scan();
		printf( "Plugboard: %s \r\n", PLUGB_GetAllMappings() );
	}
	Animation_ShiftText(encryptMessage, false);
}

static void MEF_ConfigRotor()
{
	if ( !rotorAnimDone && !delayRead(&rotorAnimDelay) ) {
		Animation_DrawRomanNumber(rotorIndex + 1);
		return; // No seguimos si no se mostró lo suficiente el nro de rotor
	}
	else
	{
		rotorAnimDone = true;
		Animation_DrawNumber(rotorPos[rotorIndex] + 1);
	}

    int8_t delta = RotaryEncoder_Read_Blocking();
    if (delta != 0) {
        int8_t newPos = rotorPos[rotorIndex] + delta;

        // Asegurar que el nuevo valor esté dentro de los límites
        if (newPos >= 0 && newPos <= 25) {
            rotorPos[rotorIndex] = newPos;
            Animation_DrawNumber(newPos + 1);
        }
    }
}

void MEF_Run()
{
	if ( loadAnimDone )
		(*MEF_Behavior[ state ])();
	else
		loadAnimDone = Animation_Loading(false);
}
