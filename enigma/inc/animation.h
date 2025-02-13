/**
 * @file animation.h
 * @brief LED matrix animation functions for displaying characters, numbers, and animations.
 *
 * Provides functions to initialize and control animations on an LED matrix
 * using a MAX7219 driver.
 *
 * @author
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 * @date 2025-02-02
 * @version 1.0
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "sapi.h"

/**
 * @brief Initializes the animation system.
 *
 * Sets up the LED matrix and initializes the MAX7219 driver.
 */
void Animation_Init(void);

/**
 * @brief Displays a single character on the LED matrix.
 *
 * @param c Character to display.
 */
void Animation_DrawCharacter(char c);

/**
 * @brief Displays a two-digit number on the LED matrix.
 *
 * @param number The number to display (0-99).
 */
void Animation_DrawNumber(uint8_t number);

/**
 * @brief Displays a Roman numeral representation of a number (1-3).
 *
 * @param number The number to display.
 */
void Animation_DrawRomanNumber(uint8_t number);

/**
 * @brief Scrolls a text message across the LED matrix.
 *
 * @param text The message to display.
 * @param reset If true, resets the scrolling animation.
 * @return true if the text has finished scrolling, false otherwise.
 */
bool_t Animation_ShiftText(char* const text, bool_t reset);

/**
 * @brief Displays a waiting animation.
 *
 * @param reset If true, resets the animation.
 * @return true when the animation cycle completes, false otherwise.
 */
bool_t Animation_WaitInput(bool_t reset);

/**
 * @brief Displays a loading animation.
 *
 * @param reset If true, resets the animation.
 * @return true when the animation cycle completes, false otherwise.
 */
bool_t Animation_Loading(bool_t reset);

#endif /* ANIMATION_H_ */
