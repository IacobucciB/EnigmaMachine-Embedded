/** @addtogroup max7219 MAX7219
 *  @{
 */

/*==================[inclusions]=============================================*/

#include "max7219.h"

/*==================[macros and definitions]=================================*/

#define PACKET(HB,LB) HB<<8|LB

/*==================[external data definition]===============================*/

spiConfig_t max7219_spi_default_cfg = { SSP_1, SPI_BITS16, SPI_CLK_MODE0,
		1000000 };

/*==================[internal data definition]===============================*/

static const uint16_t default_init_seq[] = {
	PACKET(SHUTDOWN, NORMAL_OPERATION),
	PACKET(SCAN_LIMIT, EIGHT),
	PACKET(INTENSITY, SET_INTENSITY(5)),
	PACKET(DECODE_MODE, NO_DECODE),
	PACKET(DISPLAY_TEST, DISPLAY_TEST_OFF)
};

/*==================[internal functions definition]==========================*/

static void CsLow(max7219_t *max7219) {
	GpioWrite(&max7219->cs, GPIO_LOW);
}

static void CsHigh(max7219_t *max7219) {
	GpioWrite(&max7219->cs, GPIO_HIGH);
}

static void SpiWrite(max7219_t *max7219, const uint16_t *buffer,
		uint32_t n_commands) {
	uint32_t cnt = 0;
	void * ptr;
	while (cnt < n_commands) {
		ptr = ((void *) ((uint32_t) buffer + cnt));
		CsLow(max7219);
		SpiDevWriteBlocking(&max7219->spi, ptr, sizeof(uint16_t));
		CsHigh(max7219);
		cnt++;
	}
}

static bool ValidPoint(uint8_t x, uint8_t y) {
	if (x > 0 && x < 9 && y > 0 && y < 9)
		return true;
	else
		return false;
}

/*==================[external functions definition]==========================*/

void Max7219Init(max7219_t *max7219, gpioNumber_t gpio_n, spiConfig_t cfg) {
	max7219->spi.cfg = cfg;
	SpiDevInit(&max7219->spi);

	max7219->cs.n = gpio_n;
	max7219->cs.dir = GPIO_OUT;
	max7219->cs.init_st = GPIO_HIGH;  // IS ACTIVE LOW
	GpioConfig(&max7219->cs);

	SpiWrite(max7219, default_init_seq, 5); // 5 Initialization commands
	Max7219Blank(max7219);
	Max7219Update(max7219);
}

void Max7219Update(max7219_t *max7219) {
	for (uint8_t i = DIGIT_0; i <= DIGIT_7; i++) {
		uint16_t packet = SpiDevMake2BPacket(i, max7219->data[i - 1]);
		SpiWrite(max7219, &packet, 1);
	}
}

void Max7219Blank(max7219_t *max7219) {
	for (uint8_t i = 0; i < MAX7219_SIZE; i++) {
		max7219->data[i] = 0x00;
	}
}

ledStatus_t Max7219SetPoint(max7219_t *max7219, uint8_t x, uint8_t y) {
	if (!ValidPoint(x, y)) {
		return INVALID;
	}
	max7219->data[y - 1] |= 1 << (MAX7219_SIZE - x);
	return SET_POINT;
}

ledStatus_t Max7219ResetPoint(max7219_t *max7219, uint8_t x, uint8_t y) {
	if (!ValidPoint(x, y)) {
		return INVALID;
	}
	max7219->data[y - 1] &= ~(1 << (MAX7219_SIZE - x));
	return RST_POINT;
}

ledStatus_t Max7219TogglePoint(max7219_t *max7219, uint8_t x, uint8_t y) {
	if (!ValidPoint(x, y)) {
		return INVALID;
	}
	ledStatus_t status = Max7219GetPoint(max7219, x, y);
	if (status == SET_POINT) {
		Max7219ResetPoint(max7219, x, y);
		return RST_POINT;
	} else {
		Max7219SetPoint(max7219, x, y);
		return SET_POINT;
	}
}

ledStatus_t Max7219GetPoint(max7219_t *max7219, uint8_t x, uint8_t y) {
	if (!ValidPoint(x, y)) {
		return INVALID;
	}
	uint8_t row = max7219->data[y - 1];
	ledStatus_t status = (row >> (MAX7219_SIZE - x)) & (0x01);
	return status;
}

void Max7219SetImage(max7219_t *max7219, max7219Data_t src_img) {
	for (uint8_t i = 0; i < MAX7219_SIZE; i++) {
		max7219->data[i] = src_img[i];
	}
}

void Max7219GetImage(max7219_t *max7219, max7219Data_t tar_img) {
	for (uint8_t i = 0; i < MAX7219_SIZE; i++) {
		tar_img[i] = max7219->data[i];
	}
}

/** @} doxygen end group definition */
