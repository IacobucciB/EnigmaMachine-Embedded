/**
 * @file plugb.h
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

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __PLUGB_H__
#define __PLUGB_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

/**
 * @brief Initializes the plugboard pins.
 *
 * This function configures the GPIO pins associated with each letter as input with pull-down resistors.
 */
void Plugb_Init(void);

/**
 * @brief Scans the plugboard connections.
 *
 * This function performs a software scan of the plugboard connections to determine the mappings between letters.
 * It updates the internal mapping table based on the detected connections.
 */
void Plugb_Scan(void);

/**
 * @brief Gets the mapping for a given input letter.
 *
 * This function returns the mapped letter for a given input letter based on the current plugboard configuration.
 *
 * @param input The input letter (A-Z).
 * @return char The mapped letter, or '\0' if the input is invalid.
 */
char Plugb_GetMapping(char input);

/**
 * @brief Gets all the current plugboard mappings.
 *
 * This function returns a pointer to the internal mapping table containing the current plugboard mappings.
 *
 * @return const char* A pointer to the internal mapping table.
 */
const char* Plugb_GetAllMappings(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __PLUGB_H__ */
