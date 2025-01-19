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

/** @addtogroup led_mat Led Matrix Utils
 *  @brief Matrix led utilities: Images, rotation, stage point (blinks)
 *  @{
 */

#ifndef MODULES_LPC4337_M4_DRIVERS_BM_INC_LED_MATRIX_H_
#define MODULES_LPC4337_M4_DRIVERS_BM_INC_LED_MATRIX_H_

/*==================[inclusions]=============================================*/

#include "max7219.h"

/*==================[macros and definitions]=================================*/

#define MATRIX_SIZE MAX7219_SIZE

/*==================[typedef]================================================*/

/**
 * @brief Matrix orientation tags
 */
typedef enum {
	ROT_0_CW, ROT_90_CW, ROT_180_CW, ROT_270_CW,

} matrixOrientation_t;

/**
 * @brief Stage point. Used for selection tasks
 */
typedef struct {
	uint8_t x;
	uint8_t y;
	ledStatus_t status; /**< To save current status of staged point */
	bool staged;
} stagedPoint_t;

/**
 * @brief Stage point movement tags
 */
typedef enum {
	X_PLUS_1, X_MINUS_1, Y_PLUS_1, Y_MINUS_1
} moveDir_t;

/**
 * @brief Led Matrix configuration structure
 */
typedef struct {
	max7219_t dev;
	matrixOrientation_t ori;
	stagedPoint_t st_pt;
} ledMatrix_t;

/*==================[external functions declaration]=========================*/

/**
 * @brief Initializes a led matrix
 *
 * @param[in] mat 	Pointer to led matrix configuration structure Will be initialize here
 * @param[in] dev	Already initialize MAX7219 configuration structure
 * @param[in] ori 	Orientation of matrix. See 'MATRIX ORIENTATION TAGS'
 *
 * @note (x,y)  = (1,1) on 0 degree rotation is the nearest to pin 1 of MAX7219
 * (Version with IC beside the matrix led )
 *
 */
void MatrixInit(ledMatrix_t *mat, max7219_t dev, matrixOrientation_t ori);

/**
 * @brief Updates the led matrix. Call it to reflect changes on MAX7219 matrix
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 *
 */
void MatrixUpdate(ledMatrix_t *mat);

/**
 * @brief Clears the matrix data buffer.
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 *
 */
void MatrixBlank(ledMatrix_t *mat);

/**
 * @brief Rotates the matrix data buffer as indicates second parameter
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] ori 	Orientation. See 'MATRIX ORIENTATION TAGS'
 *
 */
void MatrixRotate(ledMatrix_t *mat, matrixOrientation_t ori);

/**
 * @brief Sets a point in the matrix data buffer with the current orientation
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] x		Point x coordinate
 * @param[in] y		Point y coordinate
 *
 */
void MatrixSetPoint(ledMatrix_t *mat, uint8_t x, uint8_t y);

/**
 * @brief Resets a point in the matrix data buffer with the current orientation
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] x		Point x coordinate
 * @param[in] y		Point y coordinate
 *
 */
void MatrixRstPoint(ledMatrix_t *mat, uint8_t x, uint8_t y);

/**
 * @brief Toggles a point in the matrix data buffer with the current orientation
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] x		Point x coordinate
 * @param[in] y		Point y coordinate
 *
 */
ledStatus_t MatrixTogPoint(ledMatrix_t *mat, uint8_t x, uint8_t y);

/**
 * @brief Gets point status in the matrix data buffer with the current orientation
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] x		Point x coordinate
 * @param[in] y		Point y coordinate
 *
 */
ledStatus_t MatrixGetPoint(ledMatrix_t *mat, uint8_t x, uint8_t y);

/**
 * @brief Sets an image in the data buffer with the current orientation
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] img	Image of uint64_t format
 *
 * @note Generate uint64_t images with next web page:
 * https://xantorohara.github.io/led-matrix-editor/#
 *
 */
void MatrixSetImage(ledMatrix_t *mat, uint64_t img);

/**
 * @brief Gets the image in the data buffer with the current orientation. TODO: Implementation
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[out] img	Pointer to uint64_t to save the image
 *
 */
void MatrixGetImage(ledMatrix_t *mat, uint64_t *img);

/* -----  STAGED POINT FUNCTIONS ----- */

/**
 * @brief Stages the indicated point
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] x	 Staged point x coordinate
 * @param[in] y	 Staged point y coordinate
 *
 */
void MatrixStagePoint(ledMatrix_t *mat, uint8_t x, uint8_t y);

/**
 * @brief Unstages the staged point
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 *
 */
void MatrixUnstagePoint(ledMatrix_t *mat);

/**
 * @brief Toggle the status of the staged point
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 *
 */
ledStatus_t MatrixTogStagePoint(ledMatrix_t *mat);

/**
 * @brief Moves the staged point
 *
 * @param[in] mat 	Pointer to led matrix configuration structure
 * @param[in] dir	Direction. See 'STAGE POINT MOVEMENTS TAGS'
 *
 */
void MatrixMoveStagedPoint(ledMatrix_t *mat, moveDir_t dir);

/**
 * @brief Ticks to toggle on the staged point. Call it on timer ISR
 *
 */
void MatrixTick();

/**
 * @brief Update staged point on tick. Call on main program loop
 */
void MatrixOnTickUpdateSP(ledMatrix_t *mat);

#endif /* MODULES_LPC4337_M4_DRIVERS_BM_INC_LED_MATRIX_H_ */

/** @} doxygen end group definition */
