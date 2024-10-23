#ifndef __ROTOR_H_
#define __ROTOR_H_

#include "sapi.h"

// Definiciones para los pines del encoder (ajustar según la conexión real)
#define ENCODER_PIN_A  GPIO2
#define ENCODER_PIN_B  GPIO3
#define ENCODER_BUTTON GPIO8

// Estados del encoder
typedef enum {
    ENCODER_NONE,
    ENCODER_CLOCKWISE,
    ENCODER_COUNTERCLOCKWISE
} EncoderState_t;

// Inicializa el Rotary Encoder
void RotaryEncoder_Init(void);

// Lee el estado del Rotary Encoder y devuelve la dirección del giro
EncoderState_t RotaryEncoder_Read(void);

// Lee el estado del pulsador del encoder
bool_t RotaryEncoder_ButtonPressed(void);

#endif /*__ROTOR_H_*/
