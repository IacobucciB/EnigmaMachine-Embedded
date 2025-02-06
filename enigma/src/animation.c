/*
 * animation.c
 *
 *  Created on: 2 feb. 2025
 *      Author: Lisandro
 */

#include "led_matrix.h"
#include "font8x8_basic.h"

#define LETTER_ROW( ROW, VALUE ) ( ((uint64_t) VALUE) << ((7 - ROW) * 8))
#define BITMAP8X8_TO_IMAGE64x1( ROW, VALUE ) ( ((uint64_t) VALUE) << ((7 - ROW) * 8) )

const uint8_t font5x3_numbers[10][8] = {
		{0x00, 0xe0, 0xa0, 0xa0, 0xa0, 0xe0, 0x00, 0x00}, // zero
		{0x00, 0xc0, 0x40, 0x40, 0x40, 0xe0, 0x00, 0x00}, // one
		{0x00, 0xe0, 0x20, 0xe0, 0x80, 0xe0, 0x00, 0x00}, // two
		{0x00, 0xe0, 0x20, 0x60, 0x20, 0xe0, 0x00, 0x00}, // three
		{0x00, 0xa0, 0xa0, 0xe0, 0x20, 0x20, 0x00, 0x00}, // four
		{0x00, 0xe0, 0x80, 0xe0, 0x20, 0xe0, 0x00, 0x00}, // five
		{0x00, 0xe0, 0x80, 0xe0, 0xa0, 0xe0, 0x00, 0x00}, // six
		{0x00, 0xe0, 0x20, 0x60, 0x20, 0x20, 0x00, 0x00}, // seven
		{0x00, 0xe0, 0xa0, 0xe0, 0xa0, 0xe0, 0x00, 0x00}, // eight
		{0x00, 0xe0, 0xa0, 0xe0, 0x20, 0x20, 0x00, 0x00}  // nine
};

const uint8_t RomanNumbers[3][8] = {
		{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // one
		{0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66}, // two
		{0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB}  // three
};

static ledMatrix_t mat;

void Animation_Init()
{
	max7219_t max7219;
	Max7219Init( &max7219, ENET_RXD1, max7219_spi_default_cfg );
	MatrixInit( &mat, max7219, ROT_270_CW );

	//smile face
	MatrixSetImage( &mat, 0x7e81a581bd99817e );
	MatrixUpdate( &mat );
}

void Animation_DrawCharacter(char c)
{
	uint64_t image = 0x00;
	for (uint8_t i = 0; i < 8; ++i) {
		image |= BITMAP8X8_TO_IMAGE64x1(i, font8x8_ib8x8u[c][i]);
	}

	MatrixSetImage(&mat, image);
	MatrixUpdate(&mat);
}

void Animation_DrawNumber(uint8_t number)
{
	uint8_t digit0 = number % 10;
	uint8_t digit1 = number / 10;
	uint64_t image = 0x00;

	for (uint8_t i = 1; i < 6; ++i) {
		image |= BITMAP8X8_TO_IMAGE64x1(i, font5x3_numbers[digit1][i]);
		image |= BITMAP8X8_TO_IMAGE64x1(i, font5x3_numbers[digit0][i]) >> 4;
	}

	MatrixSetImage(&mat, image);
	MatrixUpdate(&mat);
}

void Animation_DrawRomanNumber(uint8_t number)
{
	number %= 4;
	if(number != 0)
	{
		uint64_t image = 0x00;
		for (uint8_t i = 0; i < 8; ++i) {
			image |= BITMAP8X8_TO_IMAGE64x1(i, RomanNumbers[number][i]);
		}

		MatrixSetImage(&mat, image);
		MatrixUpdate(&mat);
	}
}

void Animation_WaitInput()
{
	static uint8_t frame = 0;
	static uint64_t image = 0x00;
	static delay_t frameDelay;

	if( frame == 0 )
	{
		image = 0x1054381000000038;
		delayInit( &frameDelay, 200 );
	}
	if( delayRead(&frameDelay) )
	{
		if ( frame > 0 )
		{
			if ( frame < 3 )
			{
				image >>= 8;
				image |= 0x1000000000000038;
			}
			else if ( frame == 3 )
			{
				image |= 0x7c;
			}
			else if ( frame == 4 )
			{
				image |= 0xfe;
			}
		}

		MatrixSetImage(&mat, image);
		MatrixUpdate(&mat);

		frame++;
		frame %= 5;
	}
}

bool_t Animation_Loading(bool reset)
{
	static uint8_t frame = 0;
	static uint8_t i = 2;
	static delay_t frameDelay;

	if ( reset )
	{
		frame = 0;
		i = 2;
		delayInit(&frameDelay, 200);
	}

	if ( delayRead(&frameDelay) )
	{
		switch (frame) {
			case 0:
				Animation_DrawCharacter('|');
				break;
			case 1:
				Animation_DrawCharacter('/');
				break;
			case 2:
				Animation_DrawCharacter('-');
				break;
			case 3:
				Animation_DrawCharacter('\\');
				i--;
				break;
			default:
				break;
		}

		frame++;
		frame %= 4;

		return i == 0;  // Retorna true cuando la animación finaliza
	}

	return false;  // Retorna false si la animación sigue en curso
}
