/**
 * @file enigmaAPI.c
 * @brief Implementation of the Enigma machine simulation API.
 *
 * This file contains the implementation of the Enigma machine simulation API.
 * It provides functions to initialize and operate an Enigma machine simulation,
 * including rotor configuration, plugboard mapping, and character encryption.
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

#include "sapi.h"
#include "enigmaAPI.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ROTATE 26

const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* plugboardMappings = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char *rotor_ciphers[] = {
    "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
    "AJDKSIRUXBLHWTMCQGZNPYFVOE",
    "BDFHJLCPRTXVZNYEIWGAKMUSQO",
    "ESOVPZJAYQUIRHXLNFTGKDCMWB",
    "VZBRGITYUPSDNHLXAWMJQOFECK",
    "JPGVOUMFYQBENHZRDKASXLICTW",
    "NZJHGRCXMYSWBOUFAIVLPEKQDT",
    "FKQHTLXOCBJSPDZRAMEWNIUYGV"
};

const char *rotor_notches[] = {"Q", "E", "V", "J", "Z", "ZM", "ZM", "ZM"};

const char *rotor_turnovers[] = {"R", "F", "W", "K", "A", "AN", "AN", "AN"};

const char *reflectors[] = {
    "EJMZALYXVBWFCRQUONTSPIKHGD",
    "YRUHQSLDPXNGOKMIEBFZCWVJAT",
    "FVPJIAOYEDRZXWGCTKUQSBNMHL"
};

/**
 * @brief Structure representing a rotor in the Enigma machine.
 */
struct Rotor {
    int             offset;     /**< Current offset of the rotor */
    int             turnnext;   /**< Flag indicating if the next rotor should turn */
    const char      *cipher;    /**< Cipher string for the rotor */
    const char      *turnover;  /**< Turnover positions for the rotor */
    const char      *notch;     /**< Notch positions for the rotor */
};

/**
 * @brief Structure representing the Enigma machine.
 */
struct Enigma {
    int             numrotors;  /**< Number of rotors in the machine */
    const char      *reflector; /**< Reflector string */
    struct Rotor    rotors[8];  /**< Array of rotors */
};

/**
 * @brief Creates a new rotor with the specified configuration.
 *
 * @param machine Pointer to the Enigma machine structure.
 * @param rotornumber The rotor number (1-8).
 * @param offset The initial offset of the rotor (0-25).
 * @return struct Rotor The configured rotor.
 */
struct Rotor new_rotor(struct Enigma *machine, int rotornumber, int offset) {
    struct Rotor r;
    r.offset = offset;
    r.turnnext = 0;
    r.cipher = rotor_ciphers[rotornumber - 1];
    r.turnover = rotor_turnovers[rotornumber - 1];
    r.notch = rotor_notches[rotornumber - 1];

    return r;
}

/**
 * @brief Finds the index of a character in a string.
 *
 * @param str The string to search.
 * @param character The character to find.
 * @return int The index of the character in the string, or -1 if not found.
 */
int str_index(const char *str, int character) {
    char *pos;
    int index;
    pos = strchr(str, character);

    // pointer arithmetic
    if (pos){
        index = (int) (pos - str);
    } else {
        index = -1;
    }

    return index;
}

/**
 * @brief Advances the rotor by one position.
 *
 * @param rotor Pointer to the rotor structure.
 */
void rotor_cycle(struct Rotor *rotor) {
    rotor->offset++;
    rotor->offset = rotor->offset % ROTATE;

    // Check if the notch is active, if so trigger the turnnext
    if(str_index(rotor->turnover, alpha[rotor->offset]) >= 0) {
        rotor->turnnext = 1;
    }
}

/**
 * @brief Performs the forward encryption through a rotor.
 *
 * @param rotor Pointer to the rotor structure.
 * @param index The input index.
 * @return int The output index after encryption.
 */
int rotor_forward(struct Rotor *rotor, int index) {

    // In the cipher side, out the alpha side
    index = (index + rotor->offset) % ROTATE;
    index = str_index(alpha, rotor->cipher[index]);
    index = (ROTATE + index - rotor->offset) % ROTATE;

    return index;
}

/**
 * @brief Performs the reverse encryption through a rotor.
 *
 * @param rotor Pointer to the rotor structure.
 * @param index The input index.
 * @return int The output index after encryption.
 */
int rotor_reverse(struct Rotor *rotor, int index) {

    // In the cipher side, out the alpha side
    index = (index + rotor->offset) % ROTATE;
    index = str_index(rotor->cipher, alpha[index]);
    index = (ROTATE + index - rotor->offset) % ROTATE;

    return index;

}

/**
 * @brief Global Enigma machine instance.
 */
static struct Enigma machine = {};

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
void EnigmaAPI_Init(int rotor1 ,int rotor2 ,int rotor3, int reflector, int offset1, int offset2, int offset3)
{
    // Configure Enigma
    machine.numrotors = 3;
    machine.reflector = reflectors[reflector];
    machine.rotors[0] = new_rotor(&machine, rotor1, offset1);
    machine.rotors[1] = new_rotor(&machine, rotor2, offset2);
    machine.rotors[2] = new_rotor(&machine, rotor3, offset3);

}

/**
 * @brief Gets the current rotor position.
 *
 * This function returns the current position of the specified rotor.
 *
 * @param rotor The rotor number (0-2).
 * @return unsigned int The current position of the rotor (0-25).
 */
unsigned int EnigmaAPI_GetRotorValue(unsigned int rotor)
{
    return machine.rotors[rotor].offset;
}

/**
 * @brief Sets the plugboard mapping.
 *
 * This function sets the plugboard mapping for the Enigma machine. The plugboard
 * allows for letter substitutions before and after the rotor operations.
 *
 * @param mapping A string representing the plugboard mapping (26 characters).
 */
void EnigmaAPI_SetPlugboardMapping(const char* mapping)
{
    plugboardMappings = mapping;
}

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
char EnigmaAPI_EncryptChar(char character)
{
    int i, index;

    if (!isalpha(character)) {
        printf("%c", character);
    }

    character = toupper(character);
    character = plugboardMappings[character - 'A'];
    index = str_index(alpha, character);

    // Cycle the first rotor before continuing
    rotor_cycle(&machine.rotors[0]);
    // Double step the rotor
    if (str_index(machine.rotors[1].notch, alpha[machine.rotors[1].offset]) >= 0) {
        rotor_cycle(&machine.rotors[1]);
    }

    // Cycle the rotors
    for (i = 0; i < machine.numrotors - 1; i++) {
        if (machine.rotors[i].turnnext) {
            machine.rotors[i].turnnext = 0;
            rotor_cycle(&machine.rotors[i + 1]);
        }
    }

    // Pass through the rotors (forward)
    for (i = 0; i < machine.numrotors; i++) {
        index = rotor_forward(&machine.rotors[i], index);
    }

    // Pass through the reflector
    character = machine.reflector[index];
    index = str_index(alpha, character);

    // Pass back through the rotors (reverse)
    for (i = machine.numrotors - 1; i >= 0; i--) {
        index = rotor_reverse(&machine.rotors[i], index);
    }

    index = alpha[index] - 'A';

    // Output the encrypted character
    return plugboardMappings[index];
}
