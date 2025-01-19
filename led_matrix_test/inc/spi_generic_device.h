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

/** @addtogroup spi_generic_device SPI generic device
 *  @brief Manager for shared SPI. Supports different SPI device configuration
 *  @{
 */
#ifndef MODULES_LPC4337_M4_DRIVERS_BM_GPIO_HAL_H_
#define MODULES_LPC4337_M4_DRIVERS_BM_GPIO_HAL_H_

/*==================[inclusions]=============================================*/

#include "spi_master_hal.h"

/*==================[typedef]================================================*/
/**
 * @brief SPI generic device configuration structure: ID + SPI configuration
 */
typedef struct {
	uint8_t id; 		/**< Identifier for a single device*/
	spiConfig_t cfg;  	/**< SPI configuration associated with the device*/
} spiDevice_t;

/*==================[external functions declaration]=========================*/

/**
 * @brief Initializes the SPI  and assigns an ID to the device
 *
 * @param[in] 	dev 		Pointer to SPI generic device
 */
void SpiDevInit(spiDevice_t *dev);

/**
 * @brief Releases the identifier and deinitializes the SPI
 *
 * @param[in] 	dev 		Pointer to SPI device
 */
void SpiDevDeInit(spiDevice_t *dev);

/**
 * @brief Writes bytes with the device SPI configuration
 *
 * @param[in] 	dev 		Pointer to SPI device
 * @param[in] 	buffer 		Pointer to data to write
 * @param[in] 	bytes_to_w	Number of bytes to write
 */
void SpiDevWriteBlocking(spiDevice_t *dev,void *buffer,uint32_t bytes_to_w);

/**
 * @brief  Reads bytes with the device SPI configuration
 *
 * @param[in] 	dev 		Pointer to SPI device
 * @param[out]	buffer		Pointer to allocated space to save read data
 * @param[in]	bytes_to_r	Number of bytes to read

 */
void SpiDevReadBlocking(spiDevice_t *dev,uint8_t *buffer,uint32_t bytes_to_r);

/**
 * @brief Reads and writes bytes with the device SPI configuration
 *
 * @param[in] 	dev 		Pointer to SPI device
 * @param[in] 	buffer_tx 	Pointer to data to write
 * @param[out]	buffer_rx 	Pointer to allocated space to save read data
 * @param[in]	bytes_to_rw	Number of bytes to read and write (unique number for both)
 */
void SpiDevRWBlocking(spiDevice_t *dev, void *buffer_tx,uint8_t *buffer_rx, uint32_t bytes_to_rw );

/**
 * @brief Sets pin interrupts (ISR that use SPI) that must be
 * disable when SPI is been used
 *
 * @param[in] 	irq 		Pin interrupt number. Range:0-7
 */
void SpiDevSetIrqList(uint8_t irq);

/**
 * @brief Clears the interrupts from the list
 * of interrupts to disable
 *
 * @param[in] 	irq 		Pin interrupt number. Range:0-7
 */
void SpiDevClearIrqList(uint8_t irq);

/**
 * @brief Makes a 16 bit packet with a byte command(MSB)
 * and a byte data(LSB)
 *
 * @param[in] 	cmd 		Command
 * @param[in] 	data 		Data
 */
uint16_t SpiDevMake2BPacket(uint8_t cmd, uint8_t data);

#endif /* MODULES_LPC4337_M4_DRIVERS_BM_GPIO_HAL_H_ */

/** @} doxygen end group definition */
