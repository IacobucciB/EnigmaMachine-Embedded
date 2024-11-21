/** @addtogroup spi_master_hal SPI master HAL
 *  @{
 */

/*==================[inclusions]=============================================*/

#include "spi_master_hal.h"
#include "chip.h"

/*==================[macros and definitions]=================================*/

#define N_CONFIG  3

/*==================[typedef]================================================*/

/**
 * @brief Internal typedef for SSP pin configuration
 */
typedef struct {
	uint8_t port; 	/**< Hardware Port */
	uint8_t pin;  	/**< Hardware Pin */
	uint16_t func;	/**< Pin Function */
} sspCfg_t;

/*==================[internal data definition]===============================*/

static LPC_SSP_T* ssp[] = { LPC_SSP0, LPC_SSP1 };

static bool initialized_ssp[] = { false, false };

static const uint8_t bits[] = { SSP_BITS_8, SSP_BITS_16 };

static const uint8_t clock_mode[] = { SSP_CLOCK_CPHA0_CPOL0, /**< Clock Phase: First transition 	| Clock Out Polarity: Low */
SSP_CLOCK_CPHA0_CPOL1, /**< Clock Phase: First transition 	| Clock Out Polarity: High */
SSP_CLOCK_CPHA1_CPOL0, /**< Clock Phase: Second transition | Clock Out Polarity: Low */
SSP_CLOCK_CPHA1_CPOL1 /**< Clock Phase: Second transition | Clock Out Polarity: High */
};

static const sspCfg_t ssp_1_cfg[] = {
	{ 0x01, 19, (SCU_MODE_PULLUP	| SCU_MODE_FUNC1) }, /**< HW pin: spi_clk */
	{ 0x01, 3, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS	| SCU_MODE_FUNC5) }, /**< HW pin: spi_miso */
	{ 0x01, 4, (SCU_MODE_PULLUP | SCU_MODE_FUNC5) } /**< HW pin: spi_mosi */
};

static const sspCfg_t* ssp_cfg[] = { null, ssp_1_cfg };

/*==================[external functions definition]==========================*/

void SpiInit(spiSsp_t n) {
	if (n < (sizeof(ssp) / sizeof(LPC_SSP_T*))) {
		if (initialized_ssp[n] == false && ssp_cfg[n] != null) {

			for (uint8_t i = 0; i < N_CONFIG; i++) {
				Chip_SCU_PinMuxSet(ssp_cfg[n][i].port, ssp_cfg[n][i].pin,
						ssp_cfg[n][i].func);
			}

			Chip_SSP_Init(ssp[n]); //set as master
			Chip_SSP_Enable(ssp[n]);
			initialized_ssp[n] = true;
		}
	}
}

void SpiDeInit(spiSsp_t n) {
	if (initialized_ssp[n] == true) {
		Chip_SSP_DeInit(ssp[n]);
		initialized_ssp[n] = false;
	}
}

void SpiConfig(spiConfig_t *cfg) {
	Chip_SSP_SetFormat(ssp[cfg->ssp], bits[cfg->bits], SSP_FRAMEFORMAT_SPI,
			clock_mode[cfg->clock_mode]);
	Chip_SSP_SetBitRate(ssp[cfg->ssp], cfg->clock_freq);
}

void SpiReadBlocking(spiSsp_t n, uint8_t *buffer, uint32_t bytes_to_r) {
	Chip_SSP_DATA_SETUP_T xferConfig;
	xferConfig.tx_data = null;
	xferConfig.tx_cnt = 0;
	xferConfig.rx_data = buffer;
	xferConfig.rx_cnt = 0;
	xferConfig.length = bytes_to_r;

	Chip_SSP_RWFrames_Blocking(ssp[n], &xferConfig);
}

void SpiWriteBlocking(spiSsp_t n, void *buffer, uint32_t bytes_to_w) {
	Chip_SSP_DATA_SETUP_T xferConfig;
	xferConfig.tx_data = buffer;
	xferConfig.tx_cnt = 0;
	xferConfig.rx_data = null;
	xferConfig.rx_cnt = 0;
	xferConfig.length = bytes_to_w;

	Chip_SSP_RWFrames_Blocking(ssp[n], &xferConfig);
}

void SpiRWBlocking(spiSsp_t n, void *buffer_tx, uint8_t *buffer_rx,
		uint32_t bytes_to_rw) {
	Chip_SSP_DATA_SETUP_T xferConfig;
	xferConfig.tx_data = buffer_tx;
	xferConfig.tx_cnt = 0;
	xferConfig.rx_data = buffer_rx;
	xferConfig.rx_cnt = 0;
	xferConfig.length = bytes_to_rw;

	Chip_SSP_RWFrames_Blocking(ssp[n], &xferConfig);
}

void SpiTestLoopBack(spiSsp_t n, bool enable) {
	if (enable == true) {
		Chip_SSP_EnableLoopBack(ssp[n]);
	} else {
		Chip_SSP_DisableLoopBack(ssp[n]);
	}

}

