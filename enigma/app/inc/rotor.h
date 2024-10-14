#ifndef __ROTOR_H_
#define __ROTOR_H_

#include <chip.h>

// Definiciones para los pines del encoder
#define ENCODER_PIN_A_PORT 0
#define ENCODER_PIN_A_PIN  4  // Ajustar según la conexión real
#define ENCODER_PIN_B_PORT 0
#define ENCODER_PIN_B_PIN  5  // Ajustar según la conexión real
#define ENCODER_BUTTON_PORT 0
#define ENCODER_BUTTON_PIN  6  // Ajustar según la conexión real

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
bool RotaryEncoder_ButtonPressed(void);

#endif /*__ROTOR_H_*/