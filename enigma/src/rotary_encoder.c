/**
 * @file rotary_encoder.c
 * @brief Library for handling rotary encoders on EDU-CIAA.
 *
 * This library provides functions to initialize and read a rotary encoder
 * using GPIO pins. It implements a state machine to decode the quadrature
 * signals and determine the direction of rotation.
 *
 * @author 
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 * @version 1.0
 * @date 2024-10-28
 * 
 * @details
 * Based on the code by John Main from best-microcontroller-projects.com.
 * The original article can be found at:
 * https://www.best-microcontroller-projects.com/rotary-encoder.html
 *
 * @note This implementation uses the LPC4337 microcontroller and the 
 *       EDU-CIAA platform. It requires the sAPI library for GPIO handling.
 *
 * @copyright
 * This software is released under the MIT License.
 */

#include "rotary_encoder.h"
#include "sapi.h"

#define CLK ISP // PINA
#define DATA ENET_MDC // PINB

#define ROTARY_CCW_PATTERN_1  0xD4
#define ROTARY_CCW_PATTERN_2  0x2B
#define ROTARY_CW_PATTERN_1   0xE8
#define ROTARY_CW_PATTERN_2   0x17

volatile static uint8_t prevNextCode = 0;
volatile static uint16_t store = 0;

/**
 * @brief Reads the current state of the rotary encoder.
 * @return uint8_t The encoded 2-bit state (0bXY where X=CLK, Y=DATA)
 */
static uint8_t RotaryEncoder_ReadState() {
    uint8_t state = 0;
    if (gpioRead(DATA)) state |= 0x02;  // Read DATA pin
    if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, 3, 14)) state |= 0x01;  // Read CLK pin
    return state;
}

/**
 * @brief Initializes the rotary encoder pins.
 *
 * This function configures the CLK and DATA pins as input with pull-up resistors.
 * The CLK pin configuration depends on the specific pin used (ISP or other).
 */
void RotaryEncoder_Init() {
    // Configure CLK pin as input with pull-up
#if CLK == ISP
    // Since ISP pin is not configurable using sAPI, use LPC_Open
    Chip_SCU_PinMuxSet( 0x07, 6, (SCU_MODE_FUNC0 | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN) );
    Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, 3, 14 );
#else
    gpioConfig( CLK, GPIO_INPUT_PULLUP );
#endif

    // Configure DATA pin as input with pull-up
    gpioConfig(DATA, GPIO_INPUT_PULLUP);
}

/**
 * @brief Reads the rotary encoder state in a blocking manner.
 *
 * This function reads the state of the rotary encoder and determines the direction
 * of rotation. It uses a state machine to decode the quadrature signals from the
 * encoder and returns the direction of rotation.
 *
 * @return int8_t
 *   - 1 if the encoder was rotated clockwise (CW).
 *   - -1 if the encoder was rotated counterclockwise (CCW).
 *   - 0 if the rotation was invalid or no movement was detected.
 * 
 * @note This function should be called frequently for accurate detection.
 *
 * @example
 * ```c
 * int8_t rotation = RotaryEncoder_Read_Blocking();
 * if (rotation == 1) {
 *     printf("Clockwise rotation detected\n");
 * } else if (rotation == -1) {
 *     printf("Counterclockwise rotation detected\n");
 * }
 * ```
 */
int8_t RotaryEncoder_Read_Blocking() {
    // Lookup table for valid rotary encoder states
    static const int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    // Shift previous state and charge new data
    prevNextCode = (prevNextCode << 2) | RotaryEncoder_ReadState();
    
    // Mask the last 4 bits to keep only relevant state
    prevNextCode &= 0x0f;

    // If the state transition is valid, update the store variable
    if (rot_enc_table[prevNextCode]) {
        store <<= 4; // Shift previous stored state
        store |= prevNextCode; // Store new state

        // Check for valid state sequences indicating rotation
		if ((store & 0xff) == ROTARY_CCW_PATTERN_1 || (store & 0xff) == ROTARY_CCW_PATTERN_2) return -1; 	// Counterclockwise rotation
		if ((store & 0xff) == ROTARY_CW_PATTERN_1 || (store & 0xff) == ROTARY_CW_PATTERN_2) return 1; 	 	// Clockwise rotation
    }
    return 0; // No valid rotation detected
}
