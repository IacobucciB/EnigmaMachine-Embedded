#include "sapi.h"
#include "rotor.h"

// Variables estáticas para almacenar el estado anterior
static bool_t lastStateA = FALSE;
static bool_t lastStateB = FALSE;

// Inicialización del Rotary Encoder
void RotaryEncoder_Init(void) {
    // Configuración de los pines del encoder como entradas usando sAPI
    gpioConfig(ENCODER_PIN_A, GPIO_INPUT_PULLDOWN);  // Pin A como entrada
    gpioConfig(ENCODER_PIN_B, GPIO_INPUT_PULLDOWN);  // Pin B como entrada
    gpioConfig(ENCODER_BUTTON, GPIO_INPUT_PULLDOWN);  // Botón como entrada
    
    // Leer los estados iniciales de A y B
    lastStateA = gpioRead(ENCODER_PIN_A);
    lastStateB = gpioRead(ENCODER_PIN_B);
}

// Función para leer el estado del Rotary Encoder utilizando cuadratura
EncoderState_t RotaryEncoder_Read(void) {
    // Leer el estado actual de los pines A y B
    bool_t stateA = gpioRead(ENCODER_PIN_A);
    bool_t stateB = gpioRead(ENCODER_PIN_B);

    EncoderState_t direction = ENCODER_NONE;

    // Detectar flanco en A (cambio de estado en A)
    if (stateA != lastStateA) {
        // Si A cambió, verificamos el estado de B
        if (stateA == TRUE) {  // Flanco ascendente en A
            if (stateB == FALSE) {
                direction = ENCODER_CLOCKWISE;  // Si B es bajo, es sentido horario
            } else {
                direction = ENCODER_COUNTERCLOCKWISE;  // Si B es alto, es sentido antihorario
            }
        } else {  // Flanco descendente en A
            if (stateB == TRUE) {
                direction = ENCODER_CLOCKWISE;  // Si B es alto, es sentido horario
            } else {
                direction = ENCODER_COUNTERCLOCKWISE;  // Si B es bajo, es sentido antihorario
            }
        }
    }

    // Actualizar los estados anteriores
    lastStateA = stateA;
    lastStateB = stateB;

    return direction;
}

// Función para leer el estado del botón del Rotary Encoder
bool_t RotaryEncoder_ButtonPressed(void) {
    return !gpioRead(ENCODER_BUTTON);  // Negado porque el pulsador suele ser activo bajo
}

#define DEBOUNCE_DELAY_MS  5  // Tiempo de retardo en milisegundos para el debounce

bool_t DebounceRead(gpioMap_t pin) {
    bool_t lastState = gpioRead(pin);  // Leer el estado inicial
    delay(DEBOUNCE_DELAY_MS);          // Esperar un tiempo definido
    bool_t currentState = gpioRead(pin);  // Leer el estado después del retardo

    // Si el estado se mantiene igual después del retardo, es un estado válido
    if (lastState == currentState) {
        return currentState;
    }
    // Si hubo un cambio durante el retardo, se ignora como glitch
    return lastState;
}

// Función para detectar cambio en el pin A
bool_t RotaryEncoder_ChangeDetectedA(void) {
    bool_t stateA = DebounceRead(ENCODER_PIN_A);  // Usar el debounce en la lectura
    bool_t changeDetected = (stateA != lastStateA);
    lastStateA = stateA;  // Actualizar el estado anterior
    return changeDetected;
}

bool_t RotaryEncoder_ChangeDetectedB(void) {
    bool_t stateB = DebounceRead(ENCODER_PIN_B);  // Usar el debounce en la lectura
    bool_t changeDetected = (stateB != lastStateB);
    lastStateB = stateB;  // Actualizar el estado anterior
    return changeDetected;
}

