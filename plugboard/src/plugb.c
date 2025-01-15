/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2024/10/16
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "plugb.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/
#define NUM_LETTERS 26

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
char plugboard[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Definir los pines GPIO asociados a cada letra
int pinMap[NUM_LETTERS] = {
    /* Aqui se asignan los pines GPIO que usa cada letra */
	LCD2, 			// A
	LCD3,			// B
	LCDRS, 			// C
	LCD4, 			// D
	SPI_MISO, 		// E
	ENET_TXD1, 		// F
	ENET_TXD0, 		// G
	ENET_MDIO,		// H
	ENET_CRS_DV,	// I
	GPIO4,			// J
	GPIO2,			// K
	GPIO8,			// L
	GPIO7,			// M
	GPIO5,			// N
	GPIO3,			// O
	GPIO1,			// P
	LCD1,			// Q
	T_FIL0,			// R
	T_FIL3,			// S
	T_FIL2,			// T
	T_COL0,			// U
	T_COL1,			// V
	CAN_TD,			// W
	CAN_RD,			// X
	RS232_TXD,		// Y
	RS232_RXD		// Z
};

// Inicializar los pines como GPIO
void PLUGB_Init() {
    for (uint8_t i = 0; i < NUM_LETTERS; i++) {
    	gpioInit(pinMap[i], GPIO_INPUT_PULLDOWN); // Configurar cada pin como GPIO de entrada
    }
}

// Realizar el barrido por software
void PLUGB_Scan() {
	for (uint8_t i = 0; i < NUM_LETTERS; i++) {
	  gpioInit(pinMap[i], GPIO_OUTPUT);
	  gpioWrite(pinMap[i], TRUE); // Coloca en alto este pin

	  bool_t state = FALSE;

	  for(uint8_t j = 0; j < NUM_LETTERS; j++)
	  {
		  if(i != j)
		  {
			  state = gpioRead(pinMap[j]);
			  if(state)
			  {
				plugboard[i] = 'A' + j;
				plugboard[j] = 'A' + i;
				break;
			  }
			  else
			  {
				plugboard[i] = 'A' + i;
			  }
		  }
	  }

	  gpioWrite(pinMap[i], FALSE);
	  gpioInit(pinMap[i], GPIO_INPUT_PULLDOWN);
	}
}

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
   // ----- Setup -----------------------------------
   boardInit();

   // Inicializar los pines
   PLUGB_Init();

   delay_t printDelay;
   delayConfig( &printDelay, 2000 );

   // ----- Repeat for ever -------------------------
   while( true ) {
	  // Escanear el plugboard
	  PLUGB_Scan();

	  if( Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, 3, 14 ) ){
		  printf("Prendido \r\n");
	  }
	  printf("Plugboard: %s \r\n", plugboard);

	  delay(1000);
   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
