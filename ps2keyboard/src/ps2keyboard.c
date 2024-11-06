/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2024/11/06
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "ps2keyboard.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

/* Keyboard constants  Change to suit your Arduino
   define pins used for data and clock from keyboard */
#define DATAPIN T_FIL0
#define IRQPIN  T_COL1

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

uint16_t c;

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	// ----- Setup -----------------------------------
	boardInit();

	// Configure the keyboard library
	PS2KeyAdvanced_begin( DATAPIN, IRQPIN );
	printf( "PS2 Advanced Key Simple Test:" );

	// ----- Repeat for ever -------------------------
	while( true ) {
		if( PS2KeyAdvanced_available( ) )
		{
			// read the next key
			c = PS2KeyAdvanced_read( );
			if( c > 0 )
			{
				printf( "Value " );
				printf( "%x", c );
				printf( " - Status Bits " );
				printf( "%x", c >> 8 );
				printf( "  Code " );
				printf( "%x\r\n", c & 0xFF );
			}
		}
	}

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
