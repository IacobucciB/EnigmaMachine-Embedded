/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2024/11/11
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "led_matrix_letters.h"
#include "led_matrix.h"
#include "switch.h"
#include "font8x8_basic.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

#define LETTER_ROW(ROW,VALUE) ( ((uint64_t) VALUE) << ((7 - ROW) * 8))

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main(void) {
	// ----- Setup -----------------------------------
	boardInit();

	max7219_t max7219;
	Max7219Init(&max7219, GPIO_MAX, max7219_spi_default_cfg);

	ledMatrix_t mat;
	MatrixInit(&mat, max7219, ROT_270_CW);

	//arrow to pointing (x,y)=(8,8) led
	MatrixSetImage(&mat, 0xf0c0a09008040201);
	MatrixUpdate(&mat);

	uint8_t key = 0;
	uint8_t i;
	uint8_t letter_index = 0;
	uint64_t letter;

	// ----- Repeat for ever -------------------------
	while ( true) {
		key = Read_Switches();

		if (key == 1) {
			//letter = 0x3f66663e66663f00;
			letter = 0;
			for (i = 0; i < 8; ++i)
			{
				letter |= LETTER_ROW(i,font8x8_ib8x8u[letter_index][i]);
			}
			letter_index++;
			letter_index %= 128;
			MatrixSetImage(&mat, letter);
			MatrixUpdate(&mat);
		}

		gpioToggle(LED);
		delay(500);
	}

	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}
