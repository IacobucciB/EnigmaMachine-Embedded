/*=============================================================================
 * Author: Alan Turing
 * Date: 2024/10/23
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"

// Definir tiempo de inhibicion para comunicacion host-to-device
// ...

// Definir los pines que usarás para CLK y DAT
#define PS2_CLK_PIN_NAME	GPIO1  	// Nombre del pin usado para la linea de reloj (Clock)
#define PS2_DAT_PIN_NAME  	GPIO2	// Nombre del pin usado para la linea de datos (Data)
#define PS2_CLK_GPIO_PORT	3  		// Numero del puerto GPIO usado para la línea de lectura o escritura
#define PS2_CLK_GPIO_PIN 	4  		// Numero del pin GPIO usado para la línea de lectura o escritura

typedef enum {
	PS2_RESET, PS2_DTH, PS2_HTD
} PS2_STATE_T;

typedef enum {
	PS2_NO_ERRORS, PS2_NO_START_BIT, PS2_FAILED_PARITY_CHECK, PS2_UNRECOGNIZED_COMMAND, PS2_NO_STOP_BIT
} PS2_ERROR_T;

PS2_STATE_T PS2_state = RESET;

typedef struct {
	uint8_t command;
	uint8_t parity_bit;
	uint8_t index;
	uint8_t error;
} PS2_FRAME_T;

PS2_FRAME_T PS2_DTH_frame = { 0 };
PS2_FRAME_T PS2_HTD_frame = { 0 };

uint8_t flag_print = 0;
uint8_t flag_error = 0;
uint8_t buffer =  0;
uint8_t pos_buffer = 0;

//Firma de funciones

void PS2_Init(void);

void PS2_Int_SendCommand(uint8_t command);

void PS2_ReadCommand(void);

void PS2_SendCommand(void);

uint8_t readKey(void);

uint8_t keycode_to_char(uint8_t keycode);

uint8_t checkParity(uint8_t byte);


// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
	// Inicializar la EDU-CIAA y configurar los GPIOs
	boardConfig();

	delay(750);
	// Inicializar los pines para PS/2 y las debidas interrupciones
	PS2_Init();

	gpioWrite( LEDB, 1 );

	// ---------- REPETIR POR SIEMPRE --------------------------
	while( TRUE ) {
//		if(flag_print)
//		{
//			printf("TECLA LEIDA: 0x%X", buffer);
//			switch (flag_error) {
//				case PS2_FAILED_PARITY_CHECK:
//					printf("ERROR DE PARIDAD");
//					break;
//				case PS2_NO_START_BIT:
//					printf("NO HUBO START");
//					break;
//				case PS2_NO_STOP_BIT:
//					printf("NO HUBO STOP");
//					break;
//				default:
//					break;
//			}
//			printf("\r\n");
//			buffer = 0;
//			flag_error = 0;
//			flag_print = 0;
//		}
	}
	// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
	// directamenteno sobre un microcontroladore y no es llamado por ningun
	// Sistema Operativo, como en el caso de un programa para PC.
	return 0;
}


// Función para inicializar los pines y la comunicación PS/2
void PS2_Init(void) {
	//PS2_state = RESET;

    gpioConfig(PS2_CLK_PIN_NAME, GPIO_INPUT);
    gpioConfig(PS2_DAT_PIN_NAME, GPIO_INPUT);

    Chip_SCU_GPIOIntPinSel(0, PS2_CLK_GPIO_PORT, PS2_CLK_GPIO_PIN);
    //Chip_SCU_GPIOIntPinSel(0, 0, 4);

    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) );   // Borra posible pending de la IRQ
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 0 ) );   // Selecciona activo por flanco (edge-sensitive)
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 0 ) );     // Selecciona activo por flanco descendente

    //NVIC_SetPriority(PIN_INT0_IRQn + 0, 7);
    NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_EnableIRQ( PIN_INT0_IRQn );
}

void GPIO0_IRQHandler( void )
{
    //if ( PS2_state == PS2_DTH && ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 ) )
	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 ); //Borramos el flag de interrupcion

        //codigo a ejecutar si ocurrio la interrupcion
        PS2_ReadCommand();
        if(PS2_DTH_frame.index == 11){
//        	if(flag_print == 0)
//        	{
//        		buffer = PS2_DTH_frame.command;
//        		flag_error = PS2_DTH_frame.error;
//        		flag_print = 1;
//        	}
        	PS2_DTH_frame.index = 0;
        	PS2_DTH_frame.command = 0;
        	PS2_DTH_frame.parity_bit = 0;
        	PS2_DTH_frame.error = 0;
        }
    }

	//if ( PS2_state == PS2_HTD && ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH0 ) )
    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH0 )
	{
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 );

		PS2_SendCommand();
	}
}


uint8_t checkOddParity(uint8_t byte)
{
	byte ^= (byte >> 4);
	byte ^= (byte >> 2);
	byte ^= (byte >> 1);

	return byte & 0x01;
}


// Función para enviar un comando al teclado PS/2
void PS2_Int_SendCommand(uint8_t command) {
	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
	NVIC_DisableIRQ( PIN_INT0_IRQn );

	//PS2_state = PS2_HTD;

	// Armar trama a enviar
	PS2_HTD_frame.command = command;
	PS2_HTD_frame.index = 1;
	PS2_HTD_frame.parity_bit = checkOddParity(command);
//	PS2_HTD_frame.error = PS2_NO_ERRORS;

	Chip_PININT_DisableIntLow( LPC_GPIO_PIN_INT, PININTCH( 0 ) );     // Deshabilita activo por flanco descendente
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 0 ) );     // Habilita activo por flanco descendente

    // Poner CLK en bajo para indicar el inicio del envio
    gpioConfig(PS2_CLK_PIN_NAME, GPIO_OUTPUT);
    gpioWrite(PS2_CLK_PIN_NAME, 0);						// Clock en bajo
    delayInaccurateUs(100);								// Espera 100 microsegundos

    gpioConfig(PS2_DAT_PIN_NAME, GPIO_OUTPUT);			// Data como salida
    gpioWrite(PS2_DAT_PIN_NAME, 0);						// Data en bajo

    gpioConfig(PS2_CLK_PIN_NAME, GPIO_INPUT_PULLUP);	// Dejar de administrar linea de clock

    NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_EnableIRQ( PIN_INT0_IRQn );
}

// Función para leer la respuesta del teclado PS/2
void PS2_ReadCommand(void) {
	uint8_t data = gpioRead(PS2_DAT_PIN_NAME);
	if(PS2_DTH_frame.index > 0 && PS2_DTH_frame.index < 9)
	{
		PS2_DTH_frame.command |= (data << (PS2_DTH_frame.index - 1));
	}
	else if(PS2_DTH_frame.index == 0 && data != 0)
	{
		PS2_DTH_frame.error |= 0x01; //PS2_NO_START_BIT
	}
	else if(PS2_DTH_frame.index == 9)
	{
		PS2_DTH_frame.parity_bit = data;
		if(checkOddParity(PS2_DTH_frame.command) == data)
		{
			PS2_DTH_frame.error |= 0x02; //PS2_FAILED_PARITY_CHECK;
		}
	}
	else if(PS2_DTH_frame.index == 10 && data == 0)
	{
		PS2_DTH_frame.error |= 0x04; //PS2_NO_STOP_BIT
	}
//	else
//	{
//		PS2_DTH_frame.error = 0;
//	}
	// else if(END_TRANSMISION)
	// 		CHECK_UNRECOGNIZED_COMMAND
	PS2_DTH_frame.index++;
}

void PS2_SendCommand(void)
{
	uint8_t data;
	if(PS2_HTD_frame.index > 0 && PS2_HTD_frame.index < 10 ){
		if(PS2_HTD_frame.index == 9)
		{
			data = PS2_HTD_frame.parity_bit;
		}
		else
		{
			data = (PS2_HTD_frame.command >> PS2_HTD_frame.index) & 0x01;
		}
		gpioWrite(PS2_DAT_PIN_NAME, data); //FALTA CONSIDERAR STOP BIT Y ACK
	}
	++PS2_HTD_frame.index;
}
