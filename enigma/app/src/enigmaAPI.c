
#include "board.h"
#include "enigma.h"

#define TICKRATE_HZ (1000)

void SysTick_Handler(void) {
   tick_ct++;
}

void delay(uint32_t tk) {
   uint32_t end = tick_ct + tk;
   while(tick_ct < end)
       __WFI();
}

char output[] = "HELLO WORLD"; 

int main(void) {
   SystemCoreClockUpdate();
   Board_Init();
   SysTick_Config(SystemCoreClock / TICKRATE_HZ);
   
    enigma_init(3, 2, 1, 0, 0, 0);
char out = encrypt_char('a')

   while (1) {
      Board_LED_Toggle(LED_3);
      delay(100);
      printf("Salida cifrada es: ");
      printf("Plain: a Encrypt %c", out);


      printf("\r\n");
   }
}
