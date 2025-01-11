/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2024/10/28
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "rotary_encoder.h"
#include "sapi.h"

// Robust Rotary encoder reading
//
// Copyright John Main - best-microcontroller-projects.com

#define CLK ISP //PINA
#define DATA ENET_MDC //PINB

static uint8_t prevNextCode = 0;
static uint16_t store = 0;

void update();

int main( void )
{
   // ----- Setup -----------------------------------
   boardInit();

   RotaryEncoder_Init();

   // ----- Repeat for ever -------------------------
   while( true ) {
	   update();
   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}

void RotaryEncoder_Init() {
	// Configurar CLK pin como entrada con pullup
#if CLK == ISP
	//ya que el pin ISP no es configurable mediante sAPI, se usa LPC_Open
	Chip_SCU_PinMuxSet( 0x07, 6, (SCU_MODE_FUNC0 | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN) );
	Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, 3, 14 );
#else
	gpioConfig( CLK, GPIO_INPUT_PULLUP );
#endif

	// Configurar CLK pin como entrada con pullup
	gpioConfig( DATA, GPIO_INPUT_PULLUP );
}

void update() {
   static int8_t c, val;

   val = RotaryEncoder_Read_Blocking();
   if( val ) {
      c +=val;
      printf("%d ", c);

      if ( prevNextCode==0x0b || prevNextCode==0x04 ) {
    	  if(prevNextCode==0x0b)
    	  {
    		 printf("eleven ");
    	  }
    	  else
    	  {
    		 printf("four ");
    	  }

         printf("%x\r\n", store);
      }

      if ( prevNextCode==0x08 || prevNextCode==0x07 ) {
    	 if(prevNextCode==0x07)
    	 {
    		 printf("seven ");
    	 }
    	 else
    	 {
    		 printf("eight ");
    	 }
         printf("%x\r\n", store);
      }
   }
}

// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t RotaryEncoder_Read_Blocking() {
	static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

	prevNextCode <<= 2;
	if ( gpioRead(DATA) ) prevNextCode |= 0x02;
	if ( Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, 3, 14 ) ) prevNextCode |= 0x01;
	prevNextCode &= 0x0f;

	// If valid then store as 16 bit data.
	if ( rot_enc_table[prevNextCode] ) {
		store <<= 4;
		store |= prevNextCode;
		if ( (store&0xff)==0xd4 || (store&0xff)==0x2b ) return -1;
		if ( (store&0xff)==0xe8 || (store&0xff)==0x17 ) return 1;
	}
	return 0;
}