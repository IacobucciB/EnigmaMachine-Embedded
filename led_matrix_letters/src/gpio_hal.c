/** @addtogroup gpio_hal GPIO HAL
 *  @{
 */

/*==================[inclusions]=============================================*/

#include "gpio_hal.h"
#include "chip.h"

/*==================[macros and definitions]=================================*/

#define MAX_PIN_INT 8

/*==================[typedef]================================================*/

/**
 * @brief Internal typedef for GPIO pin configuration
 */
typedef struct {
	uint8_t port; 	/**< Hardware Port */
	uint8_t pin;  	/**< Hardware Pin */
	uint16_t func;	/**< Pin Function */
	uint8_t gpio_port; 	/**< GPIO Port */
	uint8_t gpio_pin;  	/**< GPIO Pin */
}gpioCfg_t;

/*==================[internal data definition]===============================*/

static handler_t interrupt_map[MAX_PIN_INT];

static const gpioCfg_t gpio[] ={
	{0x06, 1,SCU_MODE_FUNC0,3,0}, 	/* GPIO_0 */
	{0x06, 4,SCU_MODE_FUNC0,3,3}, 	/* GPIO_1 */
	{0x06, 5,SCU_MODE_FUNC0,3,4}, 	/* GPIO_2 */
	{0x06, 7,SCU_MODE_FUNC4,5,15},	/* GPIO_3 */
	{0x06, 8,SCU_MODE_FUNC4,5,16},	/* GPIO_4 */
	{0x06, 9,SCU_MODE_FUNC0,3,5}, 	/* GPIO_5 */
	{0x06,10,SCU_MODE_FUNC0,3,6}, 	/* GPIO_6 */
	{0x06,11,SCU_MODE_FUNC0,3,7}, 	/* GPIO_7 */
	{0x06,12,SCU_MODE_FUNC0,2,8},  	/* GPIO_8 */
	{0x00,0,SCU_MODE_FUNC0,0,0}  	/* GPIO_MAX */
};

static const uint16_t mode[]={
	SCU_MODE_INACT    | SCU_MODE_INBUFF_EN,      /* GPIO_IN */
	SCU_MODE_INACT    | SCU_MODE_INBUFF_EN,		/* GPIO_OUT */
	SCU_MODE_PULLUP   | SCU_MODE_INBUFF_EN,		/* GPIO_IN_PULLUP */
	SCU_MODE_PULLDOWN | SCU_MODE_INBUFF_EN,		/* GPIO_IN_PULLDOWN */
	SCU_MODE_REPEATER | SCU_MODE_INBUFF_EN		/* GPIO_IN_PULLUP_PULLDOWN */
};

static const LPC43XX_IRQn_Type pin_int[] = {
	PIN_INT0_IRQn,/*!< PIN_INT0 */
	PIN_INT1_IRQn,/*!< PIN_INT1 */
	PIN_INT2_IRQn,/*!< PIN_INT2 */
	PIN_INT3_IRQn,/*!< PIN_INT3 */
	PIN_INT4_IRQn,/*!< PIN_INT4 */
	PIN_INT5_IRQn,/*!< PIN_INT5 */
	PIN_INT6_IRQn,/*!< PIN_INT6 */
	PIN_INT7_IRQn /*!< PIN_INT7 */
};

/*==================[external functions definition]==========================*/

void GpioInit(){
	Chip_GPIO_Init(LPC_GPIO_PORT);
}

void GpioDeInit(){
	Chip_GPIO_DeInit(LPC_GPIO_PORT);
}

void GpioConfig(gpioPin_t * pin){

	Chip_SCU_PinMuxSet(gpio[pin->n].port,gpio[pin->n].pin,(gpio[pin->n].func|mode[pin->dir]));

	if(pin->dir==GPIO_OUT){
		Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT, gpio[pin->n].gpio_port, gpio[pin->n].gpio_pin);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpio[pin->n].gpio_port, gpio[pin->n].gpio_pin,pin->init_st);
	}
	else{
		Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, gpio[pin->n].gpio_port, gpio[pin->n].gpio_pin);
	}
}

void GpioWrite(gpioPin_t * pin, gpioState_t state){
	if(pin->dir==GPIO_OUT){
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpio[pin->n].gpio_port, gpio[pin->n].gpio_pin,state);
	}
}

bool GpioRead(gpioPin_t * pin){
	return (bool) Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, gpio[pin->n].gpio_port, gpio[pin->n].gpio_pin );
}


bool GpioToggle( gpioPin_t * pin ){
	if(pin->dir==GPIO_OUT){
	bool value = !GpioRead(pin);
	GpioWrite( pin, value );
	return value;
	}
	return 0;
}

void GpioInterruptConfig( gpioPin_t * pin, gpioPinIrq_t irq, uint8_t pin_int_num, handler_t func) {
	uint8_t enable_interrupt=true;

	interrupt_map[pin_int_num]=func;

	Chip_PININT_Init ( LPC_GPIO_PIN_INT );
	Chip_SCU_GPIOIntPinSel (pin_int_num, gpio[pin->n].gpio_port, gpio[pin->n].gpio_pin);

	Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT , PININTCH(pin_int_num));

	switch (irq) {
		case GPIO_IRQ_EDGE_RISE:
			Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT , PININTCH(pin_int_num)); /* Edge sensitive */
			Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT , PININTCH(pin_int_num));  /* Rising edge interrupt */
			break;
		case GPIO_IRQ_EDGE_FALL:
			Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT , PININTCH(pin_int_num)); /* Edge sensitive */
			Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT , PININTCH(pin_int_num));   /* Falling edge interrupt */
			break;
		case GPIO_IRQ_LEVEL_HIGH:
			Chip_PININT_SetPinModeLevel(LPC_GPIO_PIN_INT , PININTCH(pin_int_num)); /* Level sensitive */

			LPC_GPIO_PIN_INT->SIENR=PININTCH(pin_int_num); //Enable rising-edge  | Enable level sensitive
			LPC_GPIO_PIN_INT-> SIENF=PININTCH(pin_int_num);//Enable falling-edge | Select HIGH active (for level sensitive mode)

			/* Same behaviour as above two lines - LPCOpen error? No sense what's below */
			//Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT , PININTCH(pin_int_num));
			//Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT , PININTCH(pin_int_num));

			break;
		case GPIO_IRQ_LEVEL_LOW:
			Chip_PININT_SetPinModeLevel(LPC_GPIO_PIN_INT , PININTCH(pin_int_num)); /* Level sensitive */
			LPC_GPIO_PIN_INT->SIENR=PININTCH(pin_int_num); //Enable rising-edge (for edge sensitive mode)  | Enable level sensitive
			LPC_GPIO_PIN_INT->CIENF=PININTCH(pin_int_num); //Diable falling-edge (for edge sensitive mode) | Select LOW active (for level sensitive mode)

			/* Same behaviour as above two lines - LPCOpen error? No sense what's below */
			//Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT , PININTCH(pin_int_num));
			//Chip_PININT_DisableIntLow(LPC_GPIO_PIN_INT , PININTCH(pin_int_num));

			break;
		case GPIO_IRQ_NONE:
			enable_interrupt=false;
			break;
		default:
			//Error
			break;
	}

	NVIC_ClearPendingIRQ( pin_int[pin_int_num]);

	if(enable_interrupt==true){
		NVIC_EnableIRQ( pin_int[pin_int_num]);
	} else {
		NVIC_DisableIRQ(pin_int[pin_int_num]);
	}
}

void GpioInterrupt(uint8_t pin_int_num){
	if(interrupt_map[pin_int_num]!=null){
		interrupt_map[pin_int_num]();
	}
}
/** @} doxygen end group definition */
