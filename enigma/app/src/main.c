#include "board.h"

#define TICKRATE_HZ (1000)

#include "enigma.h"
#include "rotor.h"

static volatile uint32_t tick_ct = 0;

void SysTick_Handler(void) {
   tick_ct++;
}

void delay(uint32_t tk) {
   uint32_t end = tick_ct + tk;
   while(tick_ct < end)
       __WFI();
}

int main(void) {
   SystemCoreClockUpdate();
   Board_Init();
   
   SysTick_Config(SystemCoreClock / TICKRATE_HZ);
   enigma_init(3,2,1,0,0,0);
   char out = encrypt_char('a');
   
   RotaryEncoder_Init();
   int rotor = 20;
   
   while (1) {
       Board_LED_Toggle(LED_2);
       delay(10);
       EncoderState_t state = RotaryEncoder_Read();
        if (state == ENCODER_CLOCKWISE) {
            // Acción para el giro en sentido horario
           rotor++;
        } else if (state == ENCODER_COUNTERCLOCKWISE) {
            // Acción para el giro en sentido antihorario
            rotor--;
        }
      
      printf("Hola mundo at %d\r\n", tick_ct);
      printf("Direccion: %d\n",rotor);

      if (RotaryEncoder_ButtonPressed()) {
            // Acción cuando se presiona el botón
      
            printf("Letra encriptada: %c\n",out);
      }
   }
}
