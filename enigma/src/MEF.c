/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2025/01/10
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "MEF.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/
#define NUM_ROTORS 3
uint8_t rotor_index = 0;

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
typedef enum { ENCRYPT, CONFIG_PB, CONFIG_ROTOR } MEF_state_t;
MEF_state_t state;

void MEF_Init()
{
	state = ENCRYPT;
}

void MEF_Update(void) {
	if(state != CONFIG_ROTOR || rotor_index == NUM_ROTORS)
	{
		state++;
		state %= 3;
	}
	switch (state) {
		case ENCRYPT:
			rotor_index = 0;
			printf("Modo encriptacion");
			break;
		case CONFIG_PB:
			printf("Configurando plugboard");
			break;
		case CONFIG_ROTOR:
			rotor_index %= NUM_ROTORS;
			rotor_index++;
			printf("Configurando rotor %d", rotor_index);
			break;
		default:
			break;
	}
	printf("\r\n");
}

void MEF_Run()
{
	switch (state) {
		case ENCRYPT:
			//Do something
			break;
		case CONFIG_PB:
			//Do something
			break;
		case CONFIG_ROTOR:
			//Do something
			break;
		default:
			break;
	}
}
