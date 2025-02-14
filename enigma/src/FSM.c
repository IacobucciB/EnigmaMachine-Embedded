/**
 * @file FSM.c
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
 * @date 2025/01/10
 *
 * @copyright
 * Released under the MIT License.
 */

#include "sapi.h"
#include "plugb.h"
#include "rotary_encoder.h"
#include "enigmaAPI.h"
#include "PS2Keyboard.h"
#include "animation.h"
#include "FSM.h"

/*=====[Definition macros of private constants]==============================*/

#define NUM_ROTORS 3          /**< Number of rotors in the Enigma machine */
#define PLUGB_DELAY 500       /**< Delay in milliseconds for plugboard scanning */
#define ROTOR_ANIM_DELAY 700  /**< Delay in milliseconds for rotor animation */

/** Pins used for data and clock from the keyboard */
#define IRQ_PIN  T_COL2
#define DATA_PIN T_FIL1

/*=====[Definition of private types]=========================================*/

/**
 * @brief Enumeration of FSM states.
 */
typedef enum {
    ENCRYPT,       /**< State for encryption */
    CONFIG_PB,     /**< State for plugboard configuration */
    CONFIG_ROTOR   /**< State for rotor configuration */
} FSM_state_t;

/*=====[Definition of private global variables]=============================*/

static FSM_state_t state;  /**< Current state of the FSM */

/** Function pointers to FSM behaviors */
static void FSM_Encrypt(void);
static void FSM_ConfigPB(void);
static void FSM_ConfigRotor(void);
static void (*FSM_Behavior[])(void) = { FSM_Encrypt, FSM_ConfigPB, FSM_ConfigRotor };

static delay_t plugbDelay;      		/**< Delay handler for plugboard scanning */
static delay_t rotorAnimDelay;			/**< Delay handler for rotor animation */
static bool_t keyPressed = false;  		/**< Indicates if a key was pressed */
static bool displayChar = false;  		/**< Indicates if the encrypted character should be displayed */
static bool_t pressMsgDone = false; 	/**< Indicates if the "press key" message has been displayed */
static uint8_t waitAnimTimes = 3;  		/**< Counter for animation waiting cycles */
static bool_t loadAnimDone = true; 		/**< Indicates if the loading animation is complete */
static bool_t rotorAnimDone = false;	/**< Indicates if the rotor animation is complete */

static char* const plugbMessage = "PLUG ";   /**< Plugboard configuration message */
static char* const encryptMessage = "PRESS A KEY ";  /**< Encryption prompt message */

static uint8_t rotorIndex = 0;  /**< Index of the rotor currently being configured */
static uint8_t rotorPos[NUM_ROTORS] = { 0 }; /**< Rotor positions */

static char out;  /**< Stores the encrypted output character */

/*=====[Function Implementations]============================================*/

/**
 * @brief Initializes the Finite State Machine (FSM).
 *
 * This function initializes the FSM, setting up the necessary hardware and software components.
 * It configures the plugboard, rotary encoder, PS/2 keyboard, and animation modules.
 */
void FSM_Init(void) {
    Plugb_Init();
    RotaryEncoder_Init();
    delayInit(&rotorAnimDelay, 500);
    PS2Keyboard_Init(DATA_PIN, IRQ_PIN);
    Animation_Init();

    state = ENCRYPT;
    out = 0;
    EnigmaAPI_Init(3, 2, 1, 1, rotorPos[0], rotorPos[1], rotorPos[2]);
    Animation_WaitInput(true);
    pressMsgDone = Animation_ShiftText(encryptMessage, true);
}

/**
 * @brief Updates the FSM state.
 *
 * This function updates the FSM state based on the current state and input conditions.
 * It handles transitions between different states such as encryption, plugboard configuration,
 * and rotor configuration.
 */
void FSM_Update(void) {
    Animation_Loading(true);
    loadAnimDone = false;

    if (state == CONFIG_ROTOR && rotorIndex != (NUM_ROTORS - 1)) {
        rotorIndex++;
    } else {
        if (state == CONFIG_ROTOR) {
            rotorIndex = 0;
        } else if (state == ENCRYPT) {
            PS2Keyboard_DisableInt();
        }
        state++;
        state %= 3;
    }

    switch (state) {
        case ENCRYPT:
            out = 0;
            EnigmaAPI_SetPlugboardMapping(Plugb_GetAllMappings());
            EnigmaAPI_Init(3, 2, 1, 1, rotorPos[0], rotorPos[1], rotorPos[2]);
            PS2Keyboard_EnableInt();
            keyPressed = false;
            Animation_WaitInput(true);
            pressMsgDone = Animation_ShiftText(encryptMessage, true);
            printf("Encryption mode\r\n");
            break;
        case CONFIG_PB:
            Animation_ShiftText(plugbMessage, true);
            delayInit(&plugbDelay, PLUGB_DELAY);
            printf("Configuring plugboard\r\n");
            break;
        case CONFIG_ROTOR:
            rotorPos[rotorIndex] = EnigmaAPI_GetRotorValue(rotorIndex);
            delayInit(&rotorAnimDelay, ROTOR_ANIM_DELAY);
            rotorAnimDone = false;
            printf("Configuring rotor %d\r\n", rotorIndex + 1);
            break;
    }
}

/**
 * @brief Encrypts the input using the Enigma machine.
 *
 * This function reads input from the PS/2 keyboard, encrypts it using the Enigma machine,
 * and displays the encrypted output using the animation module.
 */
static void FSM_Encrypt(void) {
    if (PS2Keyboard_Available()) {
        uint16_t c = PS2Keyboard_Read();
        if (c > 0) {
            printf("Value ");
            if ('A' <= c && c <= 'Z') {
                printf("%c", c);
            } else {
                printf("%x", c);
            }
            printf(" - Status Bits ");
            printf("%x", c >> 8);
            printf("  Code ");
            printf("%x", c & 0xFF);

            if ('A' <= c && c <= 'Z') {
                keyPressed = true;
                loadAnimDone = false;
                Animation_Loading(true);

                out = EnigmaAPI_EncryptChar(c);
                printf(" - out : %c", out);
                displayChar = true;
            }

            printf("\r\n");
        }
    }
    if (!keyPressed) {
        if (!pressMsgDone) {
            pressMsgDone = Animation_ShiftText(encryptMessage, false);
        } else {
            if (Animation_WaitInput(false)) {
                --waitAnimTimes;
                if (waitAnimTimes == 0) {
                    waitAnimTimes = 3;
                    Animation_WaitInput(true);
                    pressMsgDone = false;
                }
            }
        }
    }
    else if (displayChar && loadAnimDone) {
        displayChar = false;
        Animation_DrawCharacter(out);
    }
}

/**
 * @brief Configures the plugboard.
 *
 * This function scans the plugboard connections and updates the internal mapping table.
 * It also provides visual feedback on the LED matrix.
 */
static void FSM_ConfigPB(void) {
    if (delayRead(&plugbDelay)) {
        Plugb_Scan();
        printf("Plugboard: %s\r\n", Plugb_GetAllMappings());
    }
    Animation_ShiftText(encryptMessage, false);
}

/**
 * @brief Configures the rotor positions.
 *
 * This function reads the rotary encoder input to adjust the rotor positions.
 * It updates the animation module to display the current rotor position.
 */
static void FSM_ConfigRotor(void) {
    if (!rotorAnimDone && !delayRead(&rotorAnimDelay)) {
        Animation_DrawRomanNumber(rotorIndex + 1);
        return; // Do not proceed if the rotor number has not been displayed long enough
    } else {
        rotorAnimDone = true;
        Animation_DrawNumber(rotorPos[rotorIndex] + 1);
    }

    int8_t delta = RotaryEncoder_Read_Blocking();
    if (delta != 0) {
        int8_t newPos = rotorPos[rotorIndex] + delta;

        // Ensure the new value is within limits
        if (newPos >= 0 && newPos <= 25) {
            rotorPos[rotorIndex] = newPos;
            Animation_DrawNumber(newPos + 1);
        }
    }
}

/**
 * @brief Runs the FSM behavior for the current state.
 *
 * This function executes the behavior associated with the current FSM state.
 * It processes input, performs actions, and updates the state as needed.
 */
void FSM_Run(void) {
    if (loadAnimDone) {
        (*FSM_Behavior[state])();
    } else {
        loadAnimDone = Animation_Loading(false);
    }
}