#ifndef __ROTOR_H_
#define __ROTOR_H_

#include <chip.h>

// Definiciones para los pines del encoder
#define ENCODER_PIN_A_PORT 2
#define ENCODER_PIN_A_PIN  6  // Ajustar seg�n la conexi�n real
#define ENCODER_PIN_B_PORT 3
#define ENCODER_PIN_B_PIN  3  // Ajustar seg�n la conexi�n real
#define ENCODER_BUTTON_PORT 2
#define ENCODER_BUTTON_PIN  8  // Ajustar seg�n la conexi�n real

// Estados del encoder
typedef enum {
    ENCODER_NONE,
    ENCODER_CLOCKWISE,
    ENCODER_COUNTERCLOCKWISE
} EncoderState_t;

// Inicializa el Rotary Encoder
void RotaryEncoder_Init(void);

// Lee el estado del Rotary Encoder y devuelve la direcci�n del giro
EncoderState_t RotaryEncoder_Read(void);

// Lee el estado del pulsador del encoder
bool RotaryEncoder_ButtonPressed(void);

#endif /*__ROTOR_H_*/