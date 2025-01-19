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

/** @addtogroup spi_master_hal SPI master HAL
 *	@brief SPI Hardware abstraction layer for EDU-CIAA NXP
 *  @{
 */
#ifndef MODULES_LPC4337_M4_DRIVERS_BM_INC_SPI_MASTER_HAL_H_
#define MODULES_LPC4337_M4_DRIVERS_BM_INC_SPI_MASTER_HAL_H_

/*==================[inclusions]=============================================*/

#include "drivers_bm_types.h"

/*==================[typedef]================================================*/

/**
 * @brief Synchronous Serial Port (SSP)
 * @note Use it to select the module to use
 */
typedef enum{
   SSP_0,
   SSP_1 /**< Only for completeness purposes. Not available (not wired) on EDU-CIAA */
} spiSsp_t;

/**
 * @brief Frame bits configuration
 */
typedef enum {
	SPI_BITS8, /**< 8 bits frame */
	SPI_BITS16 /**< 16 bits frame */
} spiFrameBits_t;

/**
 * @brief Clock modes
 */
typedef enum {
	SPI_CLK_MODE0,	/**< Clock Phase: First transition 	| Clock Out Polarity: Low */
	SPI_CLK_MODE1,	/**< Clock Phase: First transition 	| Clock Out Polarity: High */
	SPI_CLK_MODE2,	/**< Clock Phase: Second transition | Clock Out Polarity: Low */
	SPI_CLK_MODE3	/**< Clock Phase: Second transition | Clock Out Polarity: High */
} spiClockMode_t;

/**
 * @brief SPI configuration structure
 */
typedef struct {
	spiSsp_t ssp;				/**< SSP number */
	spiFrameBits_t bits;		/**< Format config: frame bits*/
	spiClockMode_t clock_mode;	/**< Format config: clock phase/polarity */
	uint32_t clock_freq;		/**< Clock frequency */
} spiConfig_t;

/*==================[external functions declaration]=========================*/

/**
 * @brief Configures the pin multiplexer and initializes the SSP module
 *
 * @param[in] ssp_n 	SSP module number
 */
void SpiInit( spiSsp_t n);

/**
 * @brief Deinitializes the SSP module
 *
 * @param[in] 	n	 		SSP module number
 */
void SpiDeInit( spiSsp_t n);

/**
 * @brief Configures SPI. Could be used to reconfigure SPI
 * to talk to different devices
 *
 * @param[in] 	cfg 		Pointer to SPI structure configuration
 */
void SpiConfig(spiConfig_t *cfg);

/**
 * @brief Reads on blocking (polling) mode. Writes nothing
 *
 * @param[in] 	n	 		SSP module number
 * @param[out]	buffer 	Pointer to allocated space to save read data
 * @param[in]	bytes_to_r	Number of bytes to read
 */
void SpiReadBlocking(spiSsp_t n,uint8_t *buffer, uint32_t bytes_to_r );

/**
 * @brief Writes on blocking (polling) mode. Discards readings
 *
 * @param[in] 	n	 		SSP module number
 * @param[in] 	buffer 		Pointer to data to write
 * @param[in] 	bytes_to_w	Number of bytes to write
 */
void SpiWriteBlocking(spiSsp_t n, void *buffer, uint32_t bytes_to_w );

/**
 * @brief Reads and writes on blocking (polling) mode
 *
 * @param[in] 	n	 		SSP module number
 * @param[in] 	buffer_tx 	Pointer to data to write
 * @param[out]	buffer_rx 	Pointer to allocated space to save read data
 * @param[in]	bytes_to_rw	Number of bytes to read and write (unique number for both)
 */
void SpiRWBlocking(spiSsp_t n, void *buffer_tx,uint8_t *buffer_rx, uint32_t bytes_to_rw );

/**
 * @brief Enables LoopBack Mode. Data flow: MOSI -> MISO (without wire)
 *
 * @param[in] 	n	 		SSP module number
 * @param[in] 	enable		Enabled with 'true'
 */
void SpiTestLoopBack(spiSsp_t n, bool enable);


#endif /* MODULES_LPC4337_M4_DRIVERS_BM_INC_SPI_MASTER_HAL_H_ */

/** @} doxygen end group definition */
