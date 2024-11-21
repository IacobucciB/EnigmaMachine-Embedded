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

/** @addtogroup gpio_hal GPIO HAL
 *  @brief GPIO Hardware abstraction layer for EDU-CIAA NXP
 *  @{
 */

#ifndef MODULES_LPC4337_M4_DRIVERS_BM_INC_GPIO_HAL_H_
#define MODULES_LPC4337_M4_DRIVERS_BM_INC_GPIO_HAL_H_

/*==================[inclusions]=============================================*/

#include "drivers_bm_types.h"

/*==================[typedef]================================================*/

/**
 * @brief Pin direction configuration
 */
typedef enum {
	GPIO_IN, GPIO_OUT, GPIO_IN_PULLUP, GPIO_IN_PULLDOWN, GPIO_IN_PULLUP_PULLDOWN
} gpioPinDir_t;

/**
 * @brief Pin interrupt mode
 */
typedef enum {
	GPIO_IRQ_NONE,
	GPIO_IRQ_EDGE_RISE,
	GPIO_IRQ_EDGE_FALL,
	GPIO_IRQ_LEVEL_HIGH,
	GPIO_IRQ_LEVEL_LOW
} gpioPinIrq_t;

/**
 * @brief Output GPIO state
 */
typedef enum {
	GPIO_LOW, GPIO_HIGH
} gpioState_t;

/**
 * @brief Selects GPIO number as is on EDU-CIAA board silkscreen
 */
typedef enum {
	GPIO_0, GPIO_1, GPIO_2, GPIO_3, GPIO_4, GPIO_5, GPIO_6, GPIO_7, GPIO_8, GPIO_MAX
} gpioNumber_t;

/**
 * @brief GPIO pin configuration structure
 */
typedef struct {
	gpioNumber_t n; 	/**< Number of GPIO as on EDU-CIAA board*/
	gpioPinDir_t dir;	/**< Pin direction configuration */
	gpioState_t init_st;/**< Initial state when is output */
} gpioPin_t;

/**
 * @brief Function pointer for interrupt service routine
 */
typedef void (*handler_t)(void);

/*==================[external functions declaration]=========================*/

/**
 * @brief Initializes the GPIO
 */
void GpioInit();

/**
 * @brief Calls the GPIO DeInit function
 */
void GpioDeInit();

/**
 * @brief Configures the mux and pin direction based on GPIO number
 *
 * @param[in] pin	Pointer to pin configuration
 */
void GpioConfig(gpioPin_t * pin);

/**
 * @brief Write the pin (output) with the specified state
 *
 * @param[in] pin	Pointer to pin configuration
 * @param[in] state	State to write
 */
void GpioWrite(gpioPin_t * pin, gpioState_t state);

/**
 * @brief Reads the pin
 *
 * @param[in] pin	Pointer to pin configuration
 */
bool GpioRead(gpioPin_t * pin);

/**
 * @brief Toggles the state of the pin (output)
 *
 * @param[in] pin	Pointer to pin configuration
 */
bool GpioToggle(gpioPin_t * pin);

/**
 * @brief Associates a GPIO pin with a pin interrupt
 *
 * @param[in] pin			Pointer to pin configuration
 * @param[in] irq 			Sets the interrupt mode(level/edge|high/low). See \ref gpioPinIrq_t
 * @param[in] pin_int_num 	Number of pin interruption to associated with the pin. There are only 8
 * @param[in] func 			Interrupt handler passed from upper layer
 */
void GpioInterruptConfig(gpioPin_t * pin, gpioPinIrq_t irq, uint8_t pin_int_num,
		handler_t func);

/**
 * @brief Executes the handler based on interrupt number
 *
 * @param[in] pin_int_num Pin interrupt number. Range: 0 to 7 (8 pin IRQs)
 *
 * @note Call it within the system interrupt handler. For example: GPIO0_IRQHandler()
 */
void GpioInterrupt(uint8_t pin_int_num);

#endif /* MODULES_LPC4337_M4_DRIVERS_BM_INC_GPIO_HAL_H_ */

/** @} doxygen end group definition */
