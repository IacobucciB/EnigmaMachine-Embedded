/*=============================================================================
 * Author: Alan Turing
 * Date: 2024/10/23
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "Enigma.h"
#include "sapi.h"

#include "enigmaAPI.h"
#include "rotor.h"


// Definir los pines que usar�s para CLK y DAT
#define PS2_CLK GPIO1  // Ejemplo: Pin para la l�nea de reloj (Clock)
#define PS2_DAT GPIO2  // Ejemplo: Pin para la l�nea de datos (Data)

//Firma de funciones
void ps2_init(void);

void send_ps2_command(uint8_t command);

uint8_t read_ps2_response(void);

uint8_t readKey(void);

uint8_t keycode_to_char(uint8_t keycode);



// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // Inicializar la EDU-CIAA y configurar los GPIOs
    boardConfig();
   
    // Inicializar la EDU-CIAA y los pines para PS/2
    ps2_init();
    
   delayInaccurateUs(60);  // Espera 60 microsegundos

   uint8_t key; // alamacenar la tecla le�da
   
   uint8_t readPin = 1;
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {
        key = readKey();
     
        printf("Valor de PIN: %#04x\r\n", key);

        delay(100);
       //printf("informaci�n recibida: %#04x\r\n", key);
   }   
   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}



// Funci�n para inicializar los pines y la comunicaci�n PS/2
void ps2_init(void) {
    gpioConfig(PS2_CLK, GPIO_INPUT_PULLUP);
    gpioConfig(PS2_DAT, GPIO_INPUT_PULLUP);
}


// Funci�n para enviar un comando al teclado PS/2
void send_ps2_command(uint8_t command) {
    uint8_t parity = 1;
   
    // Poner CLK en bajo para indicar el inicio del env�o
    gpioWrite(PS2_CLK, 0);  // Clock en bajo
    delayInaccurateUs(100);  // Espera 100 microsegundos

    // Enviar los 8 bits del comando
    for (int i = 0; i < 8; i++) {
        uint8_t bit = (command & (1 << i)) ? 1 : 0;

        // Enviar cada bit del comando
        gpioWrite(PS2_DAT, bit);
        
        // Generar pulso de reloj para que el teclado lea el bit
        gpioWrite(PS2_CLK, 1);  // Clock en alto
        delayInaccurateUs(100);  // Espera para que el teclado lea el dato
        gpioWrite(PS2_CLK, 0);  // Clock en bajo
        delayInaccurateUs(100);
        parity ^= bit;
    }

    // Enviar el bit de paridad (ejemplo de paridad impar)
    gpioWrite(PS2_DAT, parity);  // Paridad (puedes ajustarlo si es necesario)
    gpioWrite(PS2_CLK, 1);  // Pulso de clock para enviar la paridad
    delayInaccurateUs(100);  // Espera
    gpioWrite(PS2_CLK, 0);  // Clock en bajo
    delayInaccurateUs(100);  // Espera

    // Finalizar la transmisi�n llevando ambos pines a estado inactivo
    gpioWrite(PS2_DAT, 1);  // Llevamos Data en alto
    gpioWrite(PS2_CLK, 1);  // Llevamos Clock en alto
    delayInaccurateUs(100);  // Espera    
}



//Paridad Impar 
/*
Bit de Start, 
8 bits 
bit de paridad
bit de stop

Envia F0 al finalizar la trama indicando que se solt� una tecla
*/
// Funci�n para leer la respuesta del teclado PS/2 (ej. ACK 0xFA)
uint8_t read_ps2_response(void) {
    uint8_t response = 0;

    // Configurar el pin de datos como entrada
    //gpioConfig(PS2_DAT, GPIO_INPUT);

    // Leer los 8 bits de la respuesta
    for (int i = 0; i < 8; i++) {
        gpioWrite(PS2_CLK, 1);  // Clock en alto para leer el bit
        delayInaccurateUs(100);
        response |= (gpioRead(PS2_DAT) << i);
        gpioWrite(PS2_CLK, 0);  // Clock en bajo
        delayInaccurateUs(100);
    }

    // Leer el bit de paridad y detener el reloj
    gpioWrite(PS2_CLK, 1);
    delayInaccurateUs(100);
    gpioWrite(PS2_CLK, 0);

    // Devolver la respuesta del teclado
    return response;
}


// Funci�n para leer un byte desde el teclado PS/2

/*
uint8_t readKey(void) {
    uint8_t data = 0;
    // Esperar hasta que el reloj est� bajo (inicio de la transmisi�n)
    while (gpioRead(PS2_DAT) == HIGH);
    while (gpioRead(PS2_CLK) == HIGH);
       
    // Ignorar el bit de inicio (se env�a un '0')

    for (int i = 0; i < 8; i++) {
        delayInaccurateMs(60);
        data |= (gpioRead(PS2_DAT) << i);   // Leer el bit de datos
        printf("Informaci�n le�da: %#04x\r\n",data);
    }

    // Leer el bit de paridad (opcional para verificaci�n)
    while (gpioRead(PS2_CLK) == HIGH);  // Esperar hasta que el reloj est� alto
    while (gpioRead(PS2_CLK) == LOW); // Esperar hasta que el reloj est� bajo

    // Leer el bit de parada (se env�a un '1')
    while (gpioRead(PS2_CLK) == HIGH);  // Esperar hasta que el reloj est� alto
    while (gpioRead(PS2_CLK) == LOW); // Esperar hasta que el reloj est� bajo
   

    // Retornar el byte le�do
    return data;
}

*/


uint8_t readKey() {
    uint8_t data = 0;
    int parity = 0;

    // Esperar a que la l�nea de clock baje para el primer bit de start
    while (gpioRead(PS2_CLK) == 1);

    delayInaccurateUs(15);
    
    // Bit de start, que debe ser 0
    if (gpioRead(PS2_DAT) != 0) {
        return 0xFF;  // Error en el bit de start
    }

    // Leer los 8 bits de datos
    for (int i = 0; i < 8; i++) {
        printf("primer dato");
        // Esperar a que el reloj suba y luego baje
        while (gpioRead(PS2_CLK) == 1);
        delayInaccurateUs(15);  // Asegurar estar en el centro del pulso
        data |= (gpioRead(PS2_DAT) << i);  // Leer el bit de datos
        parity ^= gpioRead(PS2_DAT);  // Calcular la paridad
        while (gpioRead(PS2_CLK) == 0);
    }

    // Leer el bit de paridad
    while (gpioRead(PS2_CLK) == 1);
    int parity_bit = gpioRead(PS2_DAT);
    if (parity_bit == parity) {
        return 0xFF;  // Error en la paridad
    }
    while (gpioRead(PS2_CLK) == 0);

    // Leer el bit de stop, que debe ser 1
    while (gpioRead(PS2_CLK) == 1);
    if (gpioRead(PS2_DAT) != 1) {
        return 0xFF;  // Error en el bit de stop
    }
    while (gpioRead(PS2_CLK) == 0);

    return data;  // Retornar el byte de datos le�do
}