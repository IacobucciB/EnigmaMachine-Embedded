/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2024/11/28
 * Version: 0.5
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "enigma.h"
#include "sapi.h"
#include "MEF.h"

/*=====[Definition macros of private constants]==============================*/
#define BUTTON_PIN TEC1

/*=====[Definitions of extern global variables]==============================*/
extern pinInitGpioLpc4337_t gpioPinsInit[];

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
volatile uint8_t buttonPressed = 0;

static void set_button_int()
{
	Chip_SCU_PinMux(
			 gpioPinsInit[BUTTON_PIN].pinName.port,
			 gpioPinsInit[BUTTON_PIN].pinName.pin,
			 SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN,
			 SCU_MODE_FUNC0
		  );
	Chip_GPIO_SetDir( LPC_GPIO_PORT, 0, ( 1 << 4 ), 0 );

	// Setup interruption in channel 0
	Chip_SCU_GPIOIntPinSel(1, gpioPinsInit[BUTTON_PIN].gpio.port, gpioPinsInit[BUTTON_PIN].gpio.pin);

	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH(1) ); // Borra posible pending de la IRQ
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH(1) ); // Selecciona activo por flanco (edge-sensitive)
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH(1) ); // Selecciona activo por flanco descendente

	//NVIC_SetPriority(PIN_INT1_IRQn, 7);
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
}

/*=====[Main function, program entry point after power on or reset]==========*/

int main(void) {
	// ----- Setup -----------------------------------
	boardInit();
	set_button_int();
	MEF_Init();

	// ----- Repeat for ever -------------------------
	while ( true) {
		if(buttonPressed)
		{
			MEF_Update();
			buttonPressed = 0;
		}
		MEF_Run();
		gpioToggle(LED);
		delay(500);
	}

	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}

void GPIO1_IRQHandler()
{
	if (Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & PININTCH1) {

		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH1); //Borramos el flag de interrupcion
		buttonPressed = 1;
	}
}