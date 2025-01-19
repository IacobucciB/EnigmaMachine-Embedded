/** @addtogroup led_mat Led Matrix Utils
 *  @{
 */

/*==================[inclusions]=============================================*/

#include "led_matrix.h"

/*==================[typedef]================================================*/
/**
 * @brief Internal typedef for deltas on stage point movements
 */
typedef struct {
	int8_t dx;
	int8_t dy;
} delta_t;

/*==================[internal data definition]===============================*/

/* Accessed through moveDir_t tag*/
static const delta_t move_delta[] =	{ { +1, 0 }, { -1, 0 }, { 0, +1 }, { 0, -1 } };

static bool tick = false;

/*==================[internal functions definition]==========================*/

static void SetTick() {
	tick = true;
}

static void ResetTick() {
	tick = false;
}

static void TransformPoint(uint8_t x_in, uint8_t y_in, uint8_t *x_out,
		uint8_t *y_out, matrixOrientation_t ori) {
	switch (ori) {

	case ROT_0_CW:
		*x_out = x_in;
		*y_out = y_in;
		break;
	case ROT_90_CW:
		*x_out = (MATRIX_SIZE + 1) - y_in;
		*y_out = x_in;
		break;
	case ROT_180_CW:
		*x_out = (MATRIX_SIZE + 1) - x_in;
		*y_out = (MATRIX_SIZE + 1) - y_in;
		break;
	case ROT_270_CW:
		*x_out = y_in;
		*y_out = (MATRIX_SIZE + 1) - x_in;
		break;
	}
}

/*==================[external functions definition]==========================*/

void MatrixInit(ledMatrix_t *mat, max7219_t dev, matrixOrientation_t ori) {
	mat->dev = dev; // Initialized
	mat->ori = ori;
	mat->st_pt.x = 1;
	mat->st_pt.y = 1;
	mat->st_pt.status = RST_POINT;
	mat->st_pt.staged = false;
}

void MatrixUpdate(ledMatrix_t *mat) {
	Max7219Update(&(mat->dev));
}

void MatrixBlank(ledMatrix_t *mat) {
	Max7219Blank(&(mat->dev));
}

void MatrixRotate(ledMatrix_t *mat, matrixOrientation_t ori) {
	ledStatus_t status;
	uint8_t x_tf;
	uint8_t y_tf;

	max7219_t virtual_dev;
	max7219Data_t dat;
	Max7219GetImage(&(mat->dev), dat);
	Max7219Blank(&(mat->dev));
	Max7219SetImage(&virtual_dev, dat);

	for (uint8_t row = 0; row < MATRIX_SIZE; row++) {
		for (uint8_t col = 0; col < MATRIX_SIZE; col++) {

			status = Max7219GetPoint(&(virtual_dev), col + 1, row + 1);

			if (status == SET_POINT) {
				TransformPoint(col + 1, row + 1, &x_tf, &y_tf, ori);
				Max7219SetPoint(&(mat->dev), x_tf, y_tf);
			}

		}
	}
}

void MatrixSetPoint(ledMatrix_t *mat, uint8_t x, uint8_t y) {
	uint8_t x_tf;
	uint8_t y_tf;
	TransformPoint(x, y, &x_tf, &y_tf, mat->ori);
	Max7219SetPoint(&(mat->dev), x_tf, y_tf);
}

void MatrixRstPoint(ledMatrix_t *mat, uint8_t x, uint8_t y) {
	uint8_t x_tf;
	uint8_t y_tf;
	TransformPoint(x, y, &x_tf, &y_tf, mat->ori);
	Max7219ResetPoint(&(mat->dev), x_tf, y_tf);
}

ledStatus_t MatrixTogPoint(ledMatrix_t *mat, uint8_t x, uint8_t y) {
	ledStatus_t status;
	uint8_t x_tf;
	uint8_t y_tf;
	TransformPoint(x, y, &x_tf, &y_tf, mat->ori);
	status = Max7219TogglePoint(&(mat->dev), x_tf, y_tf);
	return status;
}

ledStatus_t MatrixGetPoint(ledMatrix_t *mat, uint8_t x, uint8_t y) {
	ledStatus_t status;
	uint8_t x_tf;
	uint8_t y_tf;
	TransformPoint(x, y, &x_tf, &y_tf, mat->ori);
	status = Max7219GetPoint(&(mat->dev), x_tf, y_tf);
	return status;
}

/**
 * @note Generate uint64_t images with next web page:
 *
 *  https://xantorohara.github.io/led-matrix-editor/#
 *
 */
void MatrixSetImage(ledMatrix_t *mat, uint64_t img) {

	for (uint8_t row = 0; row < MATRIX_SIZE; row++) {

		uint8_t img_row = (img >> row * MATRIX_SIZE) & 0xFF;

		for (uint8_t col = 0; col < MATRIX_SIZE; col++) {

			uint8_t value = (img_row >> col) & (0x01);

			if (value == 1) {
				MatrixSetPoint(mat, col + 1, row + 1);
			} else {
				MatrixRstPoint(mat, col + 1, row + 1);
			}
		}
	}
}

void MatrixGetImage(ledMatrix_t *mat, uint64_t *img){
	//TODO
}

void MatrixStagePoint(ledMatrix_t *mat, uint8_t x, uint8_t y) {
	if (x > 0 && x < 9 && y > 0 && y < 9) {
		mat->st_pt.x = x;
		mat->st_pt.y = y;
		mat->st_pt.status = MatrixGetPoint(mat, x, y);
		mat->st_pt.staged = true;
	}
}

void MatrixUnstagePoint(ledMatrix_t *mat) {
	mat->st_pt.staged = false;

	if (mat->st_pt.status == SET_POINT) { /* Restores the previous status */
		MatrixSetPoint(mat, mat->st_pt.x, mat->st_pt.y);
	} else {
		MatrixRstPoint(mat, mat->st_pt.x, mat->st_pt.y);
	}
}

ledStatus_t MatrixTogStagePoint(ledMatrix_t *mat) {
	ledStatus_t status = INVALID;
	if (mat->st_pt.staged == true) {
		MatrixUnstagePoint(mat);
	}
	status = MatrixTogPoint(mat, mat->st_pt.x, mat->st_pt.y);
	return status;
}

void MatrixMoveStagedPoint(ledMatrix_t *mat, moveDir_t dir) {
	if (mat->st_pt.staged == true){
		MatrixUnstagePoint(mat);
	}
	uint8_t x = (uint8_t) ((int8_t) mat->st_pt.x + move_delta[dir].dx);
	uint8_t y = (uint8_t) ((int8_t) mat->st_pt.y + move_delta[dir].dy);

	/* Toroidal movement */
	if (x == 0)
		x = 8;
	if (x == 9)
		x = 1;
	if (y == 0)
		y = 8;
	if (y == 9)
		y = 1;

	MatrixStagePoint(mat, x, y);
}

void MatrixTick() {
	SetTick();
}

void MatrixOnTickUpdateSP(ledMatrix_t *mat) {
	if (tick == true) {
		if (mat->st_pt.staged == true)
		MatrixTogPoint(mat, mat->st_pt.x, mat->st_pt.y);
		ResetTick();
	}
}

/** @} doxygen end group definition */
