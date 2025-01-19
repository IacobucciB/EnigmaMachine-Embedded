/*=============================================================================
 * Author: Martinez Lisandro <lisandromartz@gmail.com>
 * Date: 2025/01/10
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"
#include "MEF.h"
#include "plugb.h"
#include "rotary_encoder.h"
#include "led_matrix.h"

/*=====[Definition macros of private constants]==============================*/
#define NUM_ROTORS 3
#define PLUGB_DELAY 500

#define BITMAP8X8_TO_IMAGE64x1( ROW, VALUE ) ( ((uint64_t) VALUE) << ((7 - ROW) * 8) )

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/
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

/*=====[Definitions of private global variables]=============================*/
typedef enum { ENCRYPT, CONFIG_PB, CONFIG_ROTOR } MEF_state_t;
static MEF_state_t state;

static delay_t plugbDelay;

static uint8_t rotorIndex = 0;
static uint8_t rotorPos[NUM_ROTORS];
static int8_t delta;

static ledMatrix_t mat;

static void drawRotorPos()
{
	uint8_t digit0 = rotorPos[rotorIndex] % 10;
	uint8_t digit1 = rotorPos[rotorIndex] / 10;
	uint64_t image = 0x00;

	for (uint8_t i = 1; i < 6; ++i) {
		image |= BITMAP8X8_TO_IMAGE64x1(i, font5x3_numbers[digit1][i]);
		image |= BITMAP8X8_TO_IMAGE64x1(i, font5x3_numbers[digit0][i]) >> 4;
	}

	MatrixSetImage(&mat, image);
	MatrixUpdate(&mat);
}

void MEF_Init()
{
	state = ENCRYPT;

	PLUGB_Init();

	RotaryEncoder_Init();
	for (uint8_t i = 0; i < NUM_ROTORS; ++i) {
		rotorPos[i] = 1;
	}

	max7219_t max7219;
	Max7219Init(&max7219, ENET_RXD1, max7219_spi_default_cfg);
	MatrixInit(&mat, max7219, ROT_270_CW);

	//smile face
	MatrixSetImage(&mat, 0x7e81a581bd99817e);
	MatrixUpdate(&mat);
}

void MEF_Update(void) {
	if( state == CONFIG_ROTOR && rotorIndex != (NUM_ROTORS - 1) )
	{
		rotorIndex++;
	}
	else
	{
		if( state == CONFIG_ROTOR ) {
			rotorIndex = 0;
		}
		state++;
		state %= 3;
	}
	switch ( state ) {
		case ENCRYPT:
			printf( "Modo encriptacion" );
			break;
		case CONFIG_PB:
			delayInit( &plugbDelay, PLUGB_DELAY );
			printf( "Configurando plugboard" );
			break;
		case CONFIG_ROTOR:
			drawRotorPos();
			printf( "Configurando rotor %d", rotorIndex + 1);
			break;
		default:
			break;
	}
	printf( "\r\n" );
}

void MEF_Run()
{
	switch ( state ) {
		case ENCRYPT:
			//Do something
			break;
		case CONFIG_PB:
			if( delayRead( &plugbDelay ) )
			{
				PLUGB_Scan();
				printf( "Plugboard: %s \r\n", PLUGB_GetAllMappings() );
			}
			break;
		case CONFIG_ROTOR:
			delta = RotaryEncoder_Read_Blocking();
			if (delta != 0) {
			    bool_t canIncrease = (delta > 0) && (rotorPos[rotorIndex] < 26);
			    bool_t canDecrease = (delta < 0) && (rotorPos[rotorIndex] > 1);

			    if (canIncrease || canDecrease) {
			        rotorPos[rotorIndex] += delta;
			        drawRotorPos();
			    }
			}
			break;
		default:
			break;
	}
}
