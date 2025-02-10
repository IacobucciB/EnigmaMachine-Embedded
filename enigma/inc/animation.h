/*
 * animation.h
 *
 *  Created on: 2 feb. 2025
 *      Author: Lisandro
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "sapi.h"

void Animation_Init();
void Animation_DrawCharacter(char c);
void Animation_DrawNumber(uint8_t number);
void Animation_DrawRomanNumber(uint8_t number);
bool_t Animation_ShiftText(char* const text, bool_t reset);
bool_t Animation_WaitInput();
bool_t Animation_Loading(bool_t reset);


#endif /* ANIMATION_H_ */
