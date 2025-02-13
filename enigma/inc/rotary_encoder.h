/**
 * @file rotary_encoder.h
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

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __ROTARY_ENCODER_H__
#define __ROTARY_ENCODER_H__

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
 * @brief Initializes the rotary encoder pins.
 *
 * This function configures the CLK and DATA pins as input with pull-up resistors.
 * The CLK pin configuration depends on the specific pin used (ISP or other).
 */
void RotaryEncoder_Init(void);

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
int8_t RotaryEncoder_Read_Blocking(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __ROTARY_ENCODER_H__ */
