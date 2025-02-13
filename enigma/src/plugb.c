/**
 * @file plugb.c
 * @brief Library for handling the plugboard (Steckerbrett) of the Enigma machine.
 *
 * This library provides functions to initialize and manage the plugboard, allowing 
 * letter substitutions based on detected physical connections. It implements a 
 * software scan to dynamically update letter mappings.
 *
 * @author 
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 * @version 1.0
 * @date 2024-10-16
 *
 * @details
 * The plugboard is a key component of the Enigma machine, enabling letter swapping 
 * before and after passing through the rotor mechanism. This implementation uses 
 * GPIO inputs and outputs on the EDU-CIAA platform to simulate plugboard connections.
 *
 * @note 
 * - The mapping is updated dynamically through a software scanning routine.
 * - The EDU-CIAA hardware constraints determine the GPIO pin assignments.
 *
 * @copyright
 * Released under the MIT License.
 */

/*=====[Inclusions of function dependencies]=================================*/

#include "plugb.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/
#define NUM_LETTERS 26

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static char plugboardMappings[NUM_LETTERS + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Define the GPIO pins associated with each letter
static const int pinMapping[NUM_LETTERS] = {
    /* Assign the GPIO pins used by each letter */
    LCD2,           // A
    LCD3,           // B
    LCDRS,          // C
    LCD4,           // D
    SPI_MISO,       // E
    ENET_TXD1,      // F
    ENET_TXD0,      // G
    ENET_MDIO,      // H
    ENET_CRS_DV,    // I
    GPIO4,          // J
    GPIO2,          // K
    GPIO8,          // L
    GPIO7,          // M
    GPIO5,          // N
    GPIO3,          // O
    GPIO1,          // P
    LCD1,           // Q
    T_FIL0,         // R
    T_FIL3,         // S
    T_FIL2,         // T
    T_COL0,         // U
    T_COL1,         // V
    CAN_TD,         // W
    CAN_RD,         // X
    RS232_TXD,      // Y
    RS232_RXD       // Z
};

/**
 * @brief Initializes the plugboard pins.
 *
 * This function configures the GPIO pins associated with each letter as input with pull-down resistors.
 */
void Plugb_Init() {
    for (uint8_t i = 0; i < NUM_LETTERS; i++) {
        gpioInit(pinMapping[i], GPIO_INPUT_PULLDOWN); // Configure each pin as GPIO input with pull-down
    }
}

/**
 * @brief Scans the plugboard connections.
 *
 * This function performs a software scan of the plugboard connections to determine the mappings between letters.
 * It sequentially sets each pin as an output and checks which other pins detect a HIGH state.
 * The detected connections update the plugboardMappings table.
 *
 * @note If no connection is detected for a letter, it maps to itself (A → A, B → B, etc.).
 */
void Plugb_Scan() {
    for (uint8_t i = 0; i < NUM_LETTERS; i++) {
        gpioInit(pinMapping[i], GPIO_OUTPUT);
        gpioWrite(pinMapping[i], TRUE); // Set this pin high

        bool_t state = FALSE; // Flag to track if a connection is detected

        for (uint8_t j = 0; j < NUM_LETTERS; j++) {
            if (i != j) { // Avoid self-detection
                state = gpioRead(pinMapping[j]);
                if (state) { // If another pin detects HIGH, store mapping
                    plugboardMappings[i] = 'A' + j;
                    plugboardMappings[j] = 'A' + i;
                    break;
                } else {
                    plugboardMappings[i] = 'A' + i; // Default to self-mapping
                }
            }
        }

        gpioWrite(pinMapping[i], FALSE); // Reset pin to LOW state
        gpioInit(pinMapping[i], GPIO_INPUT_PULLDOWN); // Restore pin mode
    }
}

/**
 * @brief Gets the mapping for a given input letter.
 *
 * This function returns the mapped letter for a given input letter based on the current plugboard configuration.
 *
 * @param input The input letter (A-Z).
 * @return char The mapped letter, or '\0' if the input is invalid.
 */
char Plugb_GetMapping(char input) {
    if (input >= 'A' && input <= 'Z') {
        return plugboardMappings[input - 'A'];
    }
    return '\0'; // Return null character if input is invalid
}

/**
 * @brief Gets all the current plugboard mappings.
 *
 * This function returns a pointer to the internal mapping table containing the current plugboard mappings.
 *
 * @return const char* A pointer to the internal mapping table.
 */
const char* Plugb_GetAllMappings() {
    return plugboardMappings;
}