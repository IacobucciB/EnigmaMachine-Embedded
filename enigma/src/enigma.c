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
#define LED_PERIOD 1000

#define CHECK_MSEC 10     // Cada cuanto se lee el boton
#define PRESS_MSEC 50     // Tiempo estable previo a registrar "presionar"
#define RELEASE_MSEC 50   // Tiempo estable previo a registrar "soltar"

/*=====[Definitions of extern global variables]==============================*/
extern pinInitGpioLpc4337_t gpioPinsInit[];

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

void debounceSwitch(bool_t *key_changed, bool_t *key_pressed)
{
	static uint8_t count = RELEASE_MSEC / CHECK_MSEC;
	static bool_t debouncedKeyPress = false;

	bool_t rawState;

	*key_changed = false;
	*key_pressed = debouncedKeyPress;

	rawState = gpioRead(BUTTON_PIN);

	if (rawState == debouncedKeyPress)
	{
		// setear el timer que permite cambiar el estado actual.
		if (debouncedKeyPress)
			count = RELEASE_MSEC / CHECK_MSEC;
		else
			count = PRESS_MSEC / CHECK_MSEC;
	}
	else
	{
		// 'key' cambió - espera un nuevo estado para volver a estable.
		if (--count == 0)
		{
			// el timer expiró - se realiza el cambio.
			debouncedKeyPress = rawState;
			*key_changed = true;              // se avisa que se cambio el estado
			*key_pressed = debouncedKeyPress; // se pasa el estado al que cambio
			// y ahora se resetea el timer.
			if (debouncedKeyPress)
				count = RELEASE_MSEC / CHECK_MSEC;
			else
				count = PRESS_MSEC / CHECK_MSEC;
		}
	}
}

static void configButton()
{
	Chip_SCU_PinMux(
			gpioPinsInit[BUTTON_PIN].pinName.port,
			gpioPinsInit[BUTTON_PIN].pinName.pin,
			SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN,
			SCU_MODE_FUNC0
		);
	Chip_GPIO_SetDir( LPC_GPIO_PORT, 0, ( 1 << 4 ), 0 );
}

/*=====[Main function, program entry point after power on or reset]==========*/

int main(void) {
	// ----- Setup -----------------------------------
	boardInit();
	configButton();
	MEF_Init();

	delay_t ledDelay;
	delayInit( &ledDelay, LED_PERIOD/2 );

	tick_t checkTime = tickRead(); 	// Ultimo tick en el que fue observado el pulsador
	bool_t keyChanged; 	// Almacena el estado actual sin rebote del pulsador.
	bool_t keyPressed; 	// Indica si el pulsador cambió de un estado a otro.

	// ----- Repeat for ever -------------------------
	while ( true ) {
		if ( tickRead() - checkTime > CHECK_MSEC )
		{
			checkTime = tickRead();
			debounceSwitch(&keyChanged, &keyPressed);
			if (keyChanged == true && keyPressed == false)
			{
				// Hacer tarea cuando se presiono y solto el boton
				MEF_Update();
			}
		}

		if( delayRead( &ledDelay ) )
		{
			gpioToggle(LED);
		}

		MEF_Run();
	}

	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}
