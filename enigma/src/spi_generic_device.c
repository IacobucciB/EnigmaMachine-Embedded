/** @addtogroup spi_generic_device
 * @{
 */

/*==================[inclusions]=============================================*/

#include "spi_generic_device.h"
#include "chip.h"

/*==================[macros and definitions]=================================*/

#define NONE 0
#define MAX_PIN_INT 8

/*==================[internal data definition]===============================*/

static uint8_t id_generator=NONE;
static uint8_t last_dev_used=NONE;
static uint8_t enabled_irqs=NONE;
static LPC43XX_IRQn_Type interrupts[MAX_PIN_INT]={
		PIN_INT0_IRQn,
		PIN_INT1_IRQn,
		PIN_INT2_IRQn,
		PIN_INT3_IRQn,
		PIN_INT4_IRQn,
		PIN_INT5_IRQn,
		PIN_INT6_IRQn,
		PIN_INT7_IRQn
};

/*==================[internal functions definition]==========================*/

/**
 * @brief Enables all IRQ previously set
 */
static void EnableIrqs(){
	for(uint8_t i=0;i<MAX_PIN_INT;i++){
		if(((enabled_irqs>>i)&0x01)){
			NVIC_EnableIRQ(interrupts[i]);
		}
	}
}

/**
 * @brief Disable all IRQ previously set
 */
static void DisableIrqs(){
	for(uint8_t i=0;i<MAX_PIN_INT;i++){
		if(((enabled_irqs>>i)&0x01)){
			NVIC_DisableIRQ(interrupts[i]);
		}
	}
}

/*==================[external functions definition]==========================*/

void SpiDevInit(spiDevice_t *dev){
	id_generator++;
	dev->id=id_generator;
	SpiInit(dev->cfg.ssp);
}

void SpiDevDeInit(spiDevice_t *dev){
	if(id_generator>0){
		id_generator--;
	}
	SpiDeInit(dev->cfg.ssp);
}

void SpiDevWriteBlocking(spiDevice_t *dev,void *buffer,uint32_t buffer_size){
	DisableIrqs();

	if(dev->id!=last_dev_used){
		SpiConfig(&dev->cfg);
		last_dev_used=dev->id;
	}
	SpiWriteBlocking(dev->cfg.ssp, buffer,  buffer_size );

	EnableIrqs();
}

void SpiDevReadBlocking(spiDevice_t *dev,uint8_t *buffer,uint32_t buffer_size){
	DisableIrqs();

	if(dev->id!=last_dev_used){
		SpiConfig(&dev->cfg);
		last_dev_used=dev->id;
	}
	SpiReadBlocking(dev->cfg.ssp, buffer,  buffer_size );

	EnableIrqs();
}

void SpiDevRWBlocking(spiDevice_t *dev, void *buffer_tx,uint8_t *buffer_rx, uint32_t bytes_to_rw ){
	DisableIrqs();

	if(dev->id!=last_dev_used){
		SpiConfig(&dev->cfg);
		last_dev_used=dev->id;
	}
	SpiRWBlocking(dev->cfg.ssp, buffer_tx,buffer_rx, bytes_to_rw );

	EnableIrqs();
}

void SpiDevSetIrqList(uint8_t irq){
	if(irq<8){
		enabled_irqs|=(0x01<<irq);
	}
}

void SpiDevClearIrqList(uint8_t irq){
	if(irq<8){
		enabled_irqs&=~(0x01<<irq);
	}
}

uint16_t SpiDevMake2BPacket(uint8_t cmd, uint8_t data){
	uint16_t packet =((uint16_t)cmd<<8);
	packet|=(0x00FF & (uint16_t)data);
	return packet;
}

/** @} doxygen end group definition */
