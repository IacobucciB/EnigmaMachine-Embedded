#include "board.h"

#include "rotor.h"

// Inicialización del Rotary Encoder
void RotaryEncoder_Init(void) {
    // Configuración de los pines del encoder como entradas
    Chip_SCU_PinMux(1, 0, SCU_MODE_INACT | SCU_MODE_INBUFF_EN, SCU_MODE_FUNC0);  // Configuración para pin A
    Chip_GPIO_SetDir(LPC_GPIO_PORT, ENCODER_PIN_A_PORT, (1 << ENCODER_PIN_A_PIN), 0);

    Chip_SCU_PinMux(1, 1, SCU_MODE_INACT | SCU_MODE_INBUFF_EN, SCU_MODE_FUNC0);  // Configuración para pin B
    Chip_GPIO_SetDir(LPC_GPIO_PORT, ENCODER_PIN_B_PORT, (1 << ENCODER_PIN_B_PIN), 0);

    Chip_SCU_PinMux(1, 2, SCU_MODE_INACT | SCU_MODE_INBUFF_EN, SCU_MODE_FUNC0);  // Configuración para botón
    Chip_GPIO_SetDir(LPC_GPIO_PORT, ENCODER_BUTTON_PORT, (1 << ENCODER_BUTTON_PIN), 0);
}

// Función para leer el estado del Rotary Encoder
EncoderState_t RotaryEncoder_Read(void) {
    static uint8_t lastStateA = 0;
    uint8_t stateA = Chip_GPIO_GetPinState(LPC_GPIO_PORT, ENCODER_PIN_A_PORT, ENCODER_PIN_A_PIN);
    uint8_t stateB = Chip_GPIO_GetPinState(LPC_GPIO_PORT, ENCODER_PIN_B_PORT, ENCODER_PIN_B_PIN);

    EncoderState_t direction = ENCODER_NONE;

    if (lastStateA == 0 && stateA == 1) {
        // Flanco ascendente en el pin A
        if (stateB == 0) {
            direction = ENCODER_CLOCKWISE;
        } else {
            direction = ENCODER_COUNTERCLOCKWISE;
        }
    }
    lastStateA = stateA;
    return direction;
}

// Función para leer el estado del botón del Rotary Encoder
bool RotaryEncoder_ButtonPressed(void) {
    return !Chip_GPIO_GetPinState(LPC_GPIO_PORT, ENCODER_BUTTON_PORT, ENCODER_BUTTON_PIN);  // Negado porque el pulsador suele ser activo bajo
}
