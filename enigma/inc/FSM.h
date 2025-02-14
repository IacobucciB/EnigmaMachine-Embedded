/**
 * @file FSM.h
 * @brief Finite State Machine (FSM) implementation for the Enigma machine simulation.
 *
 * This module handles the state transitions and behavior of the FSM that controls
 * the encryption process, plugboard configuration, and rotor configuration.
 *
 * The FSM interacts with multiple hardware components, including:
 * - A PS/2 keyboard for input
 * - A plugboard for letter substitutions
 * - Rotors that modify the encryption process
 * - An LED matrix for visual feedback
 * - A rotary encoder for rotor configuration
 *
 * The FSM has three states:
 * 1. **ENCRYPT**: Encrypts user input using the Enigma machine.
 * 2. **CONFIG_PB**: Configures the plugboard connections.
 * 3. **CONFIG_ROTOR**: Adjusts rotor positions using a rotary encoder.
 *
 * @author
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 * @date 2025/01/10¨
 * 
 * @copyright
 * Released under the MIT License.
 */

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __FSM_H__
#define __FSM_H__

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
 * @brief Initializes the Finite State Machine (FSM).
 *
 * This function initializes the FSM, setting up the necessary hardware and software components.
 * It configures the plugboard, rotary encoder, PS/2 keyboard, and animation modules.
 */
void FSM_Init(void);

/**
 * @brief Updates the FSM state.
 *
 * This function updates the FSM state based on the current state and input conditions.
 * It handles transitions between different states such as encryption, plugboard configuration,
 * and rotor configuration.
 */
void FSM_Update(void);

/**
 * @brief Runs the FSM behavior for the current state.
 *
 * This function executes the behavior associated with the current FSM state.
 * It processes input, performs actions, and updates the state as needed.
 */
void FSM_Run(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __FSM_H__ */
