#include "board.h"
#include "font8x8_basic.h" // from https://github.com/dhepper/font8x8/blob/master/README

#define TICKRATE_HZ (1000)

#define MAX7219_REG_NOOP         0x0
#define MAX7219_REG_DIGIT0       0x1
#define MAX7219_REG_DIGIT1       0x2
#define MAX7219_REG_DIGIT2       0x3
#define MAX7219_REG_DIGIT3       0x4
#define MAX7219_REG_DIGIT4       0x5
#define MAX7219_REG_DIGIT5       0x6
#define MAX7219_REG_DIGIT6       0x7
#define MAX7219_REG_DIGIT7       0x8
#define MAX7219_REG_DECODEMODE   0x9
#define MAX7219_REG_INTENSITY    0xA
#define MAX7219_REG_SCANLIMIT    0xB
#define MAX7219_REG_SHUTDOWN     0xC
#define MAX7219_REG_DISPLAYTEST  0xF

#define CS_LOW()    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, FSSDC_CS_PORT, FSSDC_CS_PIN)
#define CS_HIGH()   Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, FSSDC_CS_PORT, FSSDC_CS_PIN)

static volatile uint32_t tick_ct = 0;

void SysTick_Handler(void) {
   tick_ct++;
}

void delay(uint32_t tk) {
   uint32_t end = tick_ct + tk;
   while(tick_ct < end)
       __WFI();
}

void CS_low()
{
   Chip_GPIO_SetPinState(LPC_GPIO_PORT, 1, 8, 0);
}

void CS_high()
{
   Chip_GPIO_SetPinState(LPC_GPIO_PORT, 1, 8, 1);
}

int main(void) {
   SystemCoreClockUpdate();
   Board_Init();
   SysTick_Config(SystemCoreClock / TICKRATE_HZ);
   
   Chip_SCU_PinMux      (0xF, 4, SCU_MODE_INACT, SCU_MODE_FUNC0); // SSP1_SCK
   Chip_SCU_PinMux      (0x1, 4, SCU_MODE_INACT, SCU_MODE_FUNC5); // SSP1_MOSI
   Chip_SCU_PinMux      (0x1, 5, SCU_MODE_INACT, SCU_MODE_FUNC5); // SSP1_SSEL
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 8), 1);
   
   // Initialize SSP Peripheral
   Chip_SSP_Init        (LPC_SSP1);
   Chip_SSP_Set_Mode    (LPC_SSP1, SSP_MODE_MASTER);
   Chip_SSP_SetFormat   (LPC_SSP1, SSP_BITS_16, SSP_FRAMEFORMAT_SPI,
                        SSP_CLOCK_CPHA0_CPOL0);
   Chip_SSP_SetBitRate  (LPC_SSP1, 100000); // 100 kbits/s --> SSP_CLK = 100 kHz aprox.?? Se puede usar un CLK más rapido < 10 MHz
   Chip_SSP_Enable      (LPC_SSP1);
   
   // Initialize MAX7219 display driver
   uint8_t buffer[] = {
      MAX7219_REG_DECODEMODE,  0,             // Using an 8x8 LED matrix, not 7-segment numeric LED displays
      MAX7219_REG_SCANLIMIT,   3,             // Show all 8 digits (matrix rows)
      MAX7219_REG_DISPLAYTEST, 1,             // Display test
      
      // Clear display
      MAX7219_REG_DIGIT0, 0,
      MAX7219_REG_DIGIT1, 0,
      MAX7219_REG_DIGIT2, 0,
      MAX7219_REG_DIGIT3, 0,
      MAX7219_REG_DIGIT4, 0,
      MAX7219_REG_DIGIT5, 0,
      MAX7219_REG_DIGIT6, 0,
      MAX7219_REG_DIGIT7, 0,
      
      MAX7219_REG_INTENSITY,   7,            // Set medium intensity (range: 0 to 15)
      MAX7219_REG_SHUTDOWN,    1,            // Not in shutdown mode (i.e. start it up)
   };
   CS_low();
   Chip_SSP_WriteFrames_Blocking(LPC_SSP1, buffer, sizeof(buffer)); // Send configuration using blocking code
   CS_high();
   
   //Test display during 3 s
   /* delay(3000);
   uint8_t data_unit[2] = {MAX7219_REG_DISPLAYTEST, 0};
   Chip_SSP_WriteFrames_Blocking(LPC_SSP1, data_unit, sizeof(data_unit)); // End display test
   
   delay(100); */
   
   // Display letter 'a'
   /* for(uint8_t i = 0; i < 8; i++)
   {
      data_unit[0] = MAX7219_REG_DIGIT0 + i;
      data_unit[1] = font8x8_basic[0x61][i];
      
      Chip_SSP_WriteFrames_Blocking(LPC_SSP1, data_unit, sizeof(data_unit)); // End display test
   } */
   
   uint8_t letter = 'A';
   while (1) {
       Board_LED_Toggle(LED_1);
      
       //Display each letter, both uppercase and lowercase, with a 10 ms delay between them
       // ....
      
       delay(100);
       printf("Hola mundo at %d\r\n", tick_ct);
   }
}
