/*=============================================================================
 * Author: Alan Turing
 * Date: 2024/10/23
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "Enigma.h"
#include "sapi.h"

#include "enigmaAPI.h"
#include "rotor.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
   // ----- Setup -----------------------------------
   boardInit();
   
   enigma_init(3,2,1,0,0,0);
   char out = encrypt_char('a');
   
   RotaryEncoder_Init();
   int rotor = 20;

   // ----- Repeat for ever -------------------------
   while( true ) {
      gpioToggle(LED);
      delay(10);
      // Detectar cambios en A y B para depuración
      if (RotaryEncoder_ChangeDetectedA()) {
         printf("Cambio detectado en A\r\n");
         rotor++;
      }
      if (RotaryEncoder_ChangeDetectedB()) {
         printf("Cambio detectado en B\r\n");
         rotor--;
      }
    
      EncoderState_t state = RotaryEncoder_Read();
      /*
      if (state == ENCODER_CLOCKWISE) {
         // Acción para el giro en sentido horario
         rotor++;
      } else if (state == ENCODER_COUNTERCLOCKWISE) {
         // Acción para el giro en sentido antihorario
         rotor--;
      }
      */
      //printf("Direccion: %d \r\n",rotor);

      if (RotaryEncoder_ButtonPressed()) {
         //printf("Letra encriptada: %c\n",out);
      }
   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
