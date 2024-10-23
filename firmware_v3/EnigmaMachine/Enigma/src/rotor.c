#include "sapi.h"
#include "rotor.h"

// Inicialización del Rotary Encoder
void RotaryEncoder_Init(void) {
    // Configuración de los pines del encoder como entradas usando sAPI
    gpioConfig(ENCODER_PIN_A, GPIO_INPUT);  // Pin A como entrada
    gpioConfig(ENCODER_PIN_B, GPIO_INPUT);  // Pin B como entrada
    gpioConfig(ENCODER_BUTTON, GPIO_INPUT);  // Botón como entrada
}

// Función para leer el estado del Rotary Encoder
EncoderState_t RotaryEncoder_Read(void) {
    static bool_t lastStateA = FALSE;
    bool_t stateA = gpioRead(ENCODER_PIN_A);
    bool_t stateB = gpioRead(ENCODER_PIN_B);

    EncoderState_t direction = ENCODER_NONE;

    if (lastStateA == FALSE && stateA == TRUE) {
        // Flanco ascendente en el pin A
        if (stateB == FALSE) {
            direction = ENCODER_CLOCKWISE;
        } else {
            direction = ENCODER_COUNTERCLOCKWISE;
        }
    }
    lastStateA = stateA;
    return direction;
}

// Función para leer el estado del botón del Rotary Encoder
bool_t RotaryEncoder_ButtonPressed(void) {
    return !gpioRead(ENCODER_BUTTON);  // Negado porque el pulsador suele ser activo bajo
}
