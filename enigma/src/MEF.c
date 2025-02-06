/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2025/01/10
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"
#include "plugb.h"
#include "rotary_encoder.h"
#include "enigmaAPI.h"
#include "PS2KeyAdvanced.h"
#include "animation.h"
#include "MEF.h"

/*=====[Definition macros of private constants]==============================*/
#define NUM_ROTORS 3
#define PLUGB_DELAY 500

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

static uint8_t rotorIndex = 0;
static uint8_t rotorPos[NUM_ROTORS] = { 0 };

static char out;

void MEF_Init()
{
	state = ENCRYPT;

	enigma_init(3, 2, 1, 0, 0, 0);

	PLUGB_Init();

	RotaryEncoder_Init();

	PS2KeyAdvanced_begin(DATA_PIN, IRQ_PIN, 0);

	Animation_Init();
}

void MEF_Update(void) {
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
			PS2KeyAdvanced_DisableInt();
		}
		state++;
		state %= 3;
	}

	switch ( state ) {
		case ENCRYPT:
			out = 0;
			enigma_init( 3, 2, 1, rotorPos[0], rotorPos[1], rotorPos[2] );
			PS2KeyAdvanced_EnableInt();
			Animation_DrawCharacter(0x05);
			printf( "Modo encriptacion \r\n" );
			break;
		case CONFIG_PB:
			Animation_DrawCharacter(0x06);
			delayInit( &plugbDelay, PLUGB_DELAY );
			printf( "Configurando plugboard \r\n" );
			break;
		case CONFIG_ROTOR:
			Animation_DrawNumber(rotorPos[rotorIndex] + 1);
			printf( "Configurando rotor %d \r\n", rotorIndex + 1);
			break;
		default:
			break;
	}
}

static void MEF_Encrypt()
{
	if ( PS2KeyAdvanced_available() )
	{
		uint16_t c = PS2KeyAdvanced_read();
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
				out = encrypt_char( PLUGB_GetMapping(c) );
				out = PLUGB_GetMapping(out);
				printf(" - out : %c", out);
				Animation_DrawCharacter(out);
			}

			printf("\r\n");
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
}

static void MEF_ConfigRotor()
{
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
	(*MEF_Behavior[ state ])();
}
