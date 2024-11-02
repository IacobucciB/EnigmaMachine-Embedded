#include "board.h"

#define TICKRATE_HZ (1000)

// Robust Rotary encoder reading
//
// Copyright John Main - best-microcontroller-projects.com
//

static volatile uint32_t tick_ct = 0;

static uint8_t prevNextCode = 0;
static uint16_t store = 0;

void rotary_init();
int8_t read_rotary();

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
   
   int8_t c, val;
   rotary_init();
   
   while (1) {
      Board_LED_Toggle(LED_2);
      delay(5);

      val = read_rotary();
      if( val ) {
         c += val;
         printf("%d", c);
         printf("\r\n");

         if ( prevNextCode==0x0b) {
            printf("eleven %x \r\n", store);
            Board_LED_Set(LED_1, 0);
            Board_LED_Set(LED_2, 1);
         }

         if ( prevNextCode==0x07) {
            printf("seven %x \r\n", store);
            Board_LED_Set(LED_1, 1);
            Board_LED_Set(LED_2, 0);
         }
      }
   }
}

void rotary_init() {
   Chip_SCU_PinMux(6, 5, SCU_MODE_INACT | SCU_MODE_INBUFF_EN, SCU_MODE_FUNC0); //CLK ->GPIO1
   Chip_SCU_PinMux(6, 4, SCU_MODE_INACT | SCU_MODE_INBUFF_EN, SCU_MODE_FUNC0); //DT ->GPIO2
   Chip_SCU_PinMux(6, 1, SCU_MODE_INACT | SCU_MODE_INBUFF_EN, SCU_MODE_FUNC0); //SW ->GPIO0
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1 << 3), 0);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1 << 4), 0);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1 << 0), 0);
   printf("KY-040 Start:\r\n");
}

// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prevNextCode <<= 2;
  if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, 3, 4)) prevNextCode |= 0x02;
  if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, 3, 3)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

   // If valid then store as 16 bit data.
   if  (rot_enc_table[prevNextCode] ) {
      store <<= 4;
      store |= prevNextCode;
      //if (store==0xd42b) return 1;
      //if (store==0xe817) return -1;
      if ((store&0xff)==0x2b) return -1;
      if ((store&0xff)==0x17) return 1;
   }
   return 0;
}