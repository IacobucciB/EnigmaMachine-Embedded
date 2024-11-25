/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2024/10/16
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "plug1.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/
#define NUM_LETTERS 26

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/
char plugboard[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*=====[Definitions of private global variables]=============================*/

// Definir los pines GPIO asociados a cada letra
int pinMap[NUM_LETTERS] = {
    /* Aquí se asignan los pines GPIO que usa cada letra */
	ENET_CRS_DV, 	// A
	ENET_MDIO,		// B
	ENET_TXD0, 		// C
	ENET_TXD1, 		// D
	SPI_MISO, 		// E
	LCD4, 			// E
	LCDRS, 			// E
	LCD3,
	LCD2,
	LCD1,
	GPIO1,
	GPIO3,
	GPIO5,
	GPIO7,
	GPIO8,
	GPIO2,
	GPIO4,
	RS232_RXD,
	RS232_TXD,
	CAN_RD,
	CAN_TD,
	T_COL1,
	T_COL0,
	T_FIL2,
	T_FIL3,
	T_FIL0
};

// Inicializar los pines como GPIO
void initPins() {
    for (uint8_t i = 0; i < NUM_LETTERS; i++) {
    	gpioInit(pinMap[i], GPIO_INPUT_PULLDOWN); // Configurar cada pin como GPIO de entrada
    }

    Chip_SCU_PinMuxSet(0x07,6,(SCU_MODE_FUNC0 | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN));
    Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, 3, 14);
}

// Realizar el barrido por software
void scanPlugboard() {
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
   initPins();

   delay_t printDelay;
   delayConfig( &printDelay, 2000 );

   // ----- Repeat for ever -------------------------
   while( true ) {
	  // Escanear el plugboard
	  scanPlugboard();

	  if(Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, 3, 14)){
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
