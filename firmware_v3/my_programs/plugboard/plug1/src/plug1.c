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
	ENET_RXD1, ENET_TXEN, ENET_MDC, ENET_CRS_DV, ENET_RXD0,
	ENET_MDIO, ENET_TXD0, ENET_TXD1, SPI_MISO, SPI_MOSI,
	LCDEN, LCDRS, LCD1, LCD2, LCD3, LCD4, GPIO0, GPIO1,
	GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, CAN_RD
};

// Inicializar los pines como GPIO
void initPins() {
    for (uint8_t i = 0; i < NUM_LETTERS; i++) {
    	gpioInit(pinMap[i], GPIO_INPUT_PULLDOWN); // Configurar cada pin como GPIO de entrada
    }
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
	  printf("Plugboard: %s \r\n", plugboard);

	  delay(1000);
   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
