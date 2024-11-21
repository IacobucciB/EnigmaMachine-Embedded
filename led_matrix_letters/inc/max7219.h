/* Copyright (c) 2018 Julián Botello <jlnbotello@gmail.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** @addtogroup max7219 MAX7219
 *  @brief Basic driver for MAX7219 with led matrix attached
 *  @{
 */

#ifndef PROJECTS_SPI_TEST_INC_MATRIX_LED_H_
#define PROJECTS_SPI_TEST_INC_MATRIX_LED_H_

/*==================[inclusions]=============================================*/

#include "spi_generic_device.h"
#include "gpio_hal.h"

/*==================[macros and definitions]=================================*/

/* Register address map */
#define NO_OP			0x00 	/* For cascading */
#define DIGIT_0			0x01
#define DIGIT_1			0x02
#define DIGIT_2			0x03
#define DIGIT_3			0x04
#define DIGIT_4			0x05
#define DIGIT_5			0x06
#define DIGIT_6			0x07
#define DIGIT_7			0x08
#define DECODE_MODE		0x09
#define INTENSITY		0x0A
#define SCAN_LIMIT		0x0B
#define SHUTDOWN		0x0C	/* Power */
#define DISPLAY_TEST	0x0F

/* Shutdown register */
#define SHUTDOWN_MODE  		0x00
#define NORMAL_OPERATION	0x01

/* Decode mode */
#define NO_DECODE 	0x00

/* Intensity register format */
#define	MIN_INTENSITY		1
#define MAX_INTENSITY 		15
#define SET_INTENSITY(x) 	(x & 0x0F)

/* Scan limit register format */
#define EIGHT 	0x07 /* Eight lines or digits */

/* Display test register format */
#define DISPLAY_TEST_OFF 	0x00
#define DISPLAY_TEST_ON		0x01

/* Side size (number of leds) of a led matrix */
#define MAX7219_SIZE 8

/*==================[typedef]================================================*/

/**
 * @brief Point(led) status codes
 */
typedef enum {
	RST_POINT,
	SET_POINT,
	INVALID	/**< For example if 'x' or 'y' are out of range */
} ledStatus_t;

/**
 * @brief Matrix data
 */
typedef uint8_t max7219Data_t[MAX7219_SIZE];

/**
 * @brief MAX7219 configuration structure
 */
typedef struct {
	spiDevice_t spi;	/**< SPI: Device */
	gpioPin_t cs;		/**< GPIO: Chip Select */
	max7219Data_t data; /**< Matrix data buffer. Reflects state of the led matrix */
} max7219_t;

/*==================[external data declaration]==============================*/

extern spiConfig_t max7219_spi_default_cfg; /**< Used on initialization function */

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize MAX7219 module attached with a led matrix
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration structure
 * @param[in] gpio_n 	GPIO number
 * @param[in] cfg	 	SPI configuration
 *
 */
void Max7219Init(max7219_t *max7219, gpioNumber_t gpio_n, spiConfig_t cfg);

/**
 * @brief Transfer the data buffer to led matrix. Updates matrix
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 *
 */
void Max7219Update(max7219_t *max7219);

/**
 * @brief Set data buffer to zero
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration structure
 *
 */
void Max7219Blank(max7219_t *max7219);

/**
 * @brief Sets a specific matrix point
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 * @param[in] x			x position of led
 * @param[in] y			y position of led
 *
 * @return Point status. See 'Status point codes'
 *
 */
ledStatus_t Max7219SetPoint(max7219_t *max7219, uint8_t x, uint8_t y);

/**
 * @brief Resets a specific matrix point
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 * @param[in] x			x position of led
 * @param[in] y			y position of led
 *
 * @return Point status. See 'Status point codes'
 *
 */
ledStatus_t Max7219ResetPoint(max7219_t *max7219, uint8_t x, uint8_t y);

/**
 * @brief Toggles a specific matrix point
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 * @param[in] x			x position of led
 * @param[in] y			y position of led
 *
 * @return Point status. See 'Status point codes'
 *
 */
ledStatus_t Max7219TogglePoint(max7219_t *max7219, uint8_t x, uint8_t y);

/**
 * @brief Gets a specific matrix point status
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 * @param[in] x			x position of led
 * @param[in] y			y position of led
 *
 * @return Point status. See 'Status point codes'
 *
 */
ledStatus_t Max7219GetPoint(max7219_t *max7219, uint8_t x, uint8_t y);

/**
 * @brief Sets the matrix data buffer
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 * @param[in] src_img	Source data buffer of type max7219Data_t
 *
 */
void Max7219SetImage(max7219_t *max7219, max7219Data_t src_img);

/**
 * @brief Gets a copy of matrix data buffer
 *
 * @param[in] max7219 	Pointer to MAX7219 configuration struct
 * @param[out] tar_img	Target data buffer of type max7219Data_t
 *
 */
void Max7219GetImage(max7219_t *max7219, max7219Data_t tar_img);


#endif /* PROJECTS_SPI_TEST_INC_MATRIX_LED_H_ */
/** @} doxygen end group definition */
