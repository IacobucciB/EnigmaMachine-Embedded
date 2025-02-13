/**
 * @file enigmaAPI.h
 * @brief API for the Enigma machine simulation.
 *
 * This API provides functions to initialize and operate an Enigma machine simulation.
 * It supports rotor configuration, plugboard mapping, and character encryption.
 *
 * The Enigma machine is a cipher device used for encrypting and decrypting messages.
 * This implementation simulates the behavior of the Enigma machine, including rotor
 * stepping, plugboard substitutions, and reflector operations.
 *
 * @note This implementation is designed for educational purposes and may not be
 *       suitable for production use.
 *
 * @version 1.0
 * @date 2025-01-10
 *
 * @author
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 *
 * @copyright
 * Released under the MIT License.
 */

#ifndef __ENIGMA_H_
#define __ENIGMA_H_

/**
 * @brief Initializes the Enigma machine.
 *
 * This function initializes the Enigma machine with the specified rotor and reflector
 * configurations. It sets the initial rotor positions and prepares the machine for
 * encryption operations.
 *
 * @param rotor1 The first rotor number (1-8).
 * @param rotor2 The second rotor number (1-8).
 * @param rotor3 The third rotor number (1-8).
 * @param reflector The reflector number (0-2).
 * @param offset1 The initial position of the first rotor (0-25).
 * @param offset2 The initial position of the second rotor (0-25).
 * @param offset3 The initial position of the third rotor (0-25).
 */
void EnigmaAPI_Init(int rotor1, int rotor2, int rotor3, int reflector, int offset1, int offset2, int offset3);

/**
 * @brief Encrypts a character using the Enigma machine.
 *
 * This function encrypts a single character using the current configuration of the
 * Enigma machine. It performs rotor stepping, plugboard substitution, and reflector
 * operations to produce the encrypted character.
 *
 * @param character The character to encrypt.
 * @return char The encrypted character.
 */
char EnigmaAPI_EncryptChar(char character);

/**
 * @brief Sets the plugboard mapping.
 *
 * This function sets the plugboard mapping for the Enigma machine. The plugboard
 * allows for letter substitutions before and after the rotor operations.
 *
 * @param mapping A string representing the plugboard mapping (26 characters).
 */
void EnigmaAPI_SetPlugboardMapping(const char* mapping);

/**
 * @brief Gets the current rotor position.
 *
 * This function returns the current position of the specified rotor.
 *
 * @param rotor The rotor number (0-2).
 * @return unsigned int The current position of the rotor (0-25).
 */
unsigned int EnigmaAPI_GetRotorValue(unsigned int rotor);

#endif /* __ENIGMA_H_ */
