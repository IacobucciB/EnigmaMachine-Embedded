w/**
 * @file enigma.c
 * @brief Main program for the Enigma machine simulation.
 *
 * This file contains the main function and supporting functions for the Enigma machine simulation.
 * It initializes the hardware, configures the FSM, and handles the main loop for processing input
 * and updating the FSM state.
 *
 * The program interacts with multiple hardware components, including:
 * - A PS/2 keyboard for input
 * - A plugboard for letter substitutions
 * - Rotors that modify the encryption process
 * - An LED matrix for visual feedback
 * - A rotary encoder for rotor configuration
 *
 * The main loop handles button debouncing, FSM updates, and LED toggling.
 *
 * @version 1.0
 * @date 2025/02/10
 *
 * @note This implementation is designed for educational purposes and may not be
 *       suitable for production use.
 *
 * @author
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 *
 * @copyright
 * Released under the MIT License.
 */

/*=====[Inclusions of function dependencies]=================================*/

#include "enigma.h"
#include "sapi.h"
#include "FSM.h"

/*=====[Definition macros of private constants]==============================*/

#define BUTTON_PIN TEC1       /**< Pin for the button */
#define LED_PERIOD 1000       /**< LED toggle period in milliseconds */

#define CHECK_MSEC 10         /**< Button check interval in milliseconds */
#define PRESS_MSEC 50         /**< Debounce time for button press in milliseconds */
#define RELEASE_MSEC 50       /**< Debounce time for button release in milliseconds */

/*=====[Definitions of extern global variables]==============================*/
extern pinInitGpioLpc4337_t gpioPinsInit[];

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/**
 * @brief Debounces the button input.
 *
 * This function debounces the button input to ensure stable state changes.
 * It uses a timer to filter out noise and short presses/releases.
 *
 * @param key_changed Pointer to a boolean that indicates if the key state has changed.
 * @param key_pressed Pointer to a boolean that indicates if the key is currently pressed.
 */
void debounceSwitch(bool_t *key_changed, bool_t *key_pressed) {
    static uint8_t count = RELEASE_MSEC / CHECK_MSEC;
    static bool_t debouncedKeyPress = false;

    bool_t rawState;

    *key_changed = false;
    *key_pressed = debouncedKeyPress;

    rawState = gpioRead(BUTTON_PIN);

    if (rawState == debouncedKeyPress) {
        // Reset the timer for stable state
        if (debouncedKeyPress)
            count = RELEASE_MSEC / CHECK_MSEC;
        else
            count = PRESS_MSEC / CHECK_MSEC;
    } else {
        // Key state changed - wait for stable state
        if (--count == 0) {
            // Timer expired - change state
            debouncedKeyPress = rawState;
            *key_changed = true;              // Notify state change
            *key_pressed = debouncedKeyPress; // Update key state
            // Reset the timer
            if (debouncedKeyPress)
                count = RELEASE_MSEC / CHECK_MSEC;
            else
                count = PRESS_MSEC / CHECK_MSEC;
        }
    }
}

/**
 * @brief Configures the button pin.
 *
 * This function configures the button pin as an input with a pull-up resistor.
 */
static void configButton() {
    Chip_SCU_PinMux(
        gpioPinsInit[BUTTON_PIN].pinName.port,
        gpioPinsInit[BUTTON_PIN].pinName.pin,
        SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN,
        SCU_MODE_FUNC0
    );
    Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 4), 0);
}

/*=====[Main function, program entry point after power on or reset]==========*/

/**
 * @brief Main function for the Enigma machine simulation.
 *
 * This function initializes the hardware, configures the FSM, and handles the main loop
 * for processing input and updating the FSM state. It debounces the button input, updates
 * the FSM state, and toggles the LED.
 *
 * @return int This function does not return.
 */
int main(void) {
    // ----- Setup -----------------------------------
    boardInit();
    configButton();
    FSM_Init();

    delay_t ledDelay;
    delayInit(&ledDelay, LED_PERIOD / 2);

    tick_t checkTime = tickRead();  // Last tick when the button was checked
    bool_t keyChanged;  // Stores the current debounced state of the button
    bool_t keyPressed;  // Indicates if the button state has changed

    // ----- Repeat forever -------------------------
    while (true) {
        if (tickRead() - checkTime > CHECK_MSEC) {
            checkTime = tickRead();
            debounceSwitch(&keyChanged, &keyPressed);
            if (keyChanged == true && keyPressed == false) {
                // Perform task when the button is pressed and released
                FSM_Update();
            }
        }

        if (delayRead(&ledDelay)) {
            gpioToggle(LED);
        }

        FSM_Run();
    }

	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}
