/*=============================================================================
 * Author: Alan Turing
 * Date: 2024/10/23
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "Enigma.h"
#include "sapi.h"

#include "enigmaAPI.h"
#include "rotor.h"


// Definir los pines que usarás para CLK y DAT
#define PS2_CLK GPIO1  // Ejemplo: Pin para la línea de reloj (Clock)
#define PS2_DAT GPIO2  // Ejemplo: Pin para la línea de datos (Data)

#define TIMOVER_ms 100
#define DELAYED_READ(a) //delay_fast(a)

uint16_t data_read=0;
uint8_t flag_read_data=0;

// prototipos
void delay_fast(uint8_t comparaciones);
void scan_keyboad_lock();

//Firma de funciones
void ps2_init(void);



// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    boardConfig();
    gpioConfig(PS2_CLK, GPIO_INPUT);
    gpioConfig(PS2_DAT, GPIO_INPUT);


	while(1){
		scan_keyboad_lock();
		if(flag_read_data){
			printf("0x%x\r\n",data_read);
			flag_read_data=0;
		}
      delay(500);
	}
    return 0;
}



// Función para inicializar los pines y la comunicación PS/2
void ps2_init(void) {
    gpioConfig(PS2_CLK, GPIO_INPUT_PULLUP);
    gpioConfig(PS2_DAT, GPIO_INPUT_PULLUP);
}




//Paridad Impar 
/*
Bit de Start, 
8 bits 
bit de paridad
bit de stop

Envia F0 al finalizar la trama indicando que se soltó una tecla
*/


void delay_fast(uint8_t comparaciones){
	for(uint8_t i=0; i<comparaciones ; i++);	
}   

void scan_keyboad_lock(){
	static uint8_t index_bit=0;
	uint32_t tim_over=tickRead();
	while(gpioRead(PS2_CLK) && tickRead()<tim_over+TIMOVER_ms );
	if(tickRead()<tim_over+TIMOVER_ms) { // si no se preciona nada durante un tiempo
		index_bit=0;			// finaliza la lectura del dato
		flag_read_data=1;
		return;
	}
	DELAYED_READ(3);
	data_read += gpioRead(PS2_DAT)<<index_bit; // acumula el dato
	index_bit++;
}

