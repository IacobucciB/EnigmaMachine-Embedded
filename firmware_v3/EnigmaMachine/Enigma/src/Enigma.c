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

//Firma de funciones
void ps2_init(void);

void send_ps2_command(uint8_t command);

uint8_t read_ps2_response(void);

uint8_t ps2_read_byte(void);

uint8_t keycode_to_char(uint8_t keycode);



// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // Inicializar la EDU-CIAA y configurar los GPIOs
    boardConfig();
   
    // Inicializar la EDU-CIAA y los pines para PS/2
    ps2_init();
   
   uint8_t key; // alamacenar la tecla leída

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {
        // Leer la tecla
        key = ps2_read_byte();
        if (key != 0) {
           printf("información recibida: %c\r\n", key);
        }
       delay(100); 
   }   
   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}



// Función para inicializar los pines y la comunicación PS/2
void ps2_init(void) {
    gpioConfig(PS2_CLK, GPIO_INPUT_PULLUP);
    gpioConfig(PS2_DAT, GPIO_INPUT_PULLUP);
}


// Función para enviar un comando al teclado PS/2
void send_ps2_command(uint8_t command) {
    uint8_t parity = 1;
   
    // Poner CLK en bajo para indicar el inicio del envío
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

    // Finalizar la transmisión llevando ambos pines a estado inactivo
    gpioWrite(PS2_DAT, 1);  // Llevamos Data en alto
    gpioWrite(PS2_CLK, 1);  // Llevamos Clock en alto
    delayInaccurateUs(100);  // Espera    
}

// Función para leer la respuesta del teclado PS/2 (ej. ACK 0xFA)
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


// Función para leer un byte desde el teclado PS/2
uint8_t ps2_read_byte(void) {
    uint8_t data = 0;

    // Esperar hasta que el reloj esté bajo (inicio de la transmisión)
    while (gpioRead(PS2_CLK) == 1);
    
    // Ignorar el bit de inicio (se envía un '0')
    while (gpioRead(PS2_CLK) == 0);  // Esperar hasta que el reloj esté alto
    while (gpioRead(PS2_CLK) == 1); // Esperar hasta que el reloj esté bajo

    // Leer los 8 bits de datos
    for (int i = 0; i < 8; i++) {
        while (gpioRead(PS2_CLK) == 0);  // Esperar hasta que el reloj esté alto
        data |= (gpioRead(PS2_DAT) << i);   // Leer el bit de datos
        while (gpioRead(PS2_CLK) == 1); // Esperar hasta que el reloj esté bajo
    }

    // Leer el bit de paridad (opcional para verificación)
    while (gpioRead(PS2_CLK) == 0);  // Esperar hasta que el reloj esté alto
    while (gpioRead(PS2_CLK) == 1); // Esperar hasta que el reloj esté bajo

    // Leer el bit de parada (se envía un '1')
    while (gpioRead(PS2_CLK) == 0);  // Esperar hasta que el reloj esté alto
    while (gpioRead(PS2_CLK) == 1); // Esperar hasta que el reloj esté bajo
   

    // Retornar el byte leído
    return data;
}

