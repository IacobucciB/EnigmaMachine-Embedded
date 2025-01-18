/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2025/01/10
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"
#include "MEF.h"
#include "../inc/plugb.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/
#define NUM_ROTORS 3
#define PLUGB_DELAY 500

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
typedef enum { ENCRYPT, CONFIG_PB, CONFIG_ROTOR } MEF_state_t;
static MEF_state_t state;

static uint8_t rotor_index = 0;

static delay_t plugbDelay;

void MEF_Init()
{
	state = ENCRYPT;

	PLUGB_Init();
}

void MEF_Update(void) {
	if( state != CONFIG_ROTOR || rotor_index == NUM_ROTORS )
	{
		state++;
		state %= 3;
	}
	switch ( state ) {
		case ENCRYPT:
			rotor_index = 0;
			printf( "Modo encriptacion" );
			break;
		case CONFIG_PB:
			delayInit( &plugbDelay, PLUGB_DELAY );
			printf( "Configurando plugboard" );
			break;
		case CONFIG_ROTOR:
			rotor_index %= NUM_ROTORS;
			rotor_index++;
			printf( "Configurando rotor %d", rotor_index );
			break;
		default:
			break;
	}
	printf( "\r\n" );
}

void MEF_Run()
{
	switch ( state ) {
		case ENCRYPT:
			//Do something
			break;
		case CONFIG_PB:
			if( delayRead( &plugbDelay ) )
			{
				PLUGB_Scan();
				printf( "Plugboard: %s \r\n", PLUGB_GetAllMappings() );
			}
			break;
		case CONFIG_ROTOR:
			//Do something
			break;
		default:
			break;
	}
}
