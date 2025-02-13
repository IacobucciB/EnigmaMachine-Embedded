 /**
 * @file animation.c
 * @brief LED matrix animation functions for displaying characters, numbers, and animations.
 *
 * Provides functions to initialize and control animations on an LED matrix
 * using a MAX7219 driver.
 *
 * @author
 *   - Juan Bautista Iacobucci <link00222@gmail.com>
 *   - Fernando Ramirez Tolentino <fernandoramireztolentino@hotmail.com>
 *   - Lisandro Martinez <lisandromartz@gmail.com>
 * @date 2025-02-02
 * @version 1.0
 */

#include "led_matrix.h"
#include "font8x8_basic.h"
 
#define CHAR_TO_IMAGE64x1_ROW( ROW, VALUE ) ( ((uint64_t) VALUE) << ((7 - ROW) * 8) )
 
/*=====[Character Font Definitions]==========================================*/

/** Font for 5x3 pixel numbers */
const uint8_t font5x3_numbers[10][8] = {
    {0x00, 0xe0, 0xa0, 0xa0, 0xa0, 0xe0, 0x00, 0x00}, // 0
    {0x00, 0xc0, 0x40, 0x40, 0x40, 0xe0, 0x00, 0x00}, // 1
    {0x00, 0xe0, 0x20, 0xe0, 0x80, 0xe0, 0x00, 0x00}, // 2
    {0x00, 0xe0, 0x20, 0x60, 0x20, 0xe0, 0x00, 0x00}, // 3
    {0x00, 0xa0, 0xa0, 0xe0, 0x20, 0x20, 0x00, 0x00}, // 4
    {0x00, 0xe0, 0x80, 0xe0, 0x20, 0xe0, 0x00, 0x00}, // 5
    {0x00, 0xe0, 0x80, 0xe0, 0xa0, 0xe0, 0x00, 0x00}, // 6
    {0x00, 0xe0, 0x20, 0x60, 0x20, 0x20, 0x00, 0x00}, // 7
    {0x00, 0xe0, 0xa0, 0xe0, 0xa0, 0xe0, 0x00, 0x00}, // 8
    {0x00, 0xe0, 0xa0, 0xe0, 0x20, 0x20, 0x00, 0x00}  // 9
};

/** Font for Roman numerals */
const uint8_t RomanNumbers[3][8] = {
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // I
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66}, // II
    {0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB}  // III
};

/** LED matrix instance */
static ledMatrix_t mat;

/*=====[Function Implementations]============================================*/

/**
 * @brief Initializes the animation system.
 *
 * Sets up the LED matrix and initializes the MAX7219 driver.
 */
void Animation_Init() {
    max7219_t max7219;
    Max7219Init(&max7219, ENET_RXD1, max7219_spi_default_cfg);
    MatrixInit(&mat, max7219, ROT_270_CW);
}

/**
 * @brief Displays a single character on the LED matrix.
 *
 * @param c Character to display.
 */
void Animation_DrawCharacter(char c) {
    uint64_t image = 0x00;
    for (uint8_t i = 0; i < 8; ++i) {
        image |= CHAR_TO_IMAGE64x1_ROW(i, font8x8_ib8x8u[c][i]);
    }
    MatrixSetImage(&mat, image);
    MatrixUpdate(&mat);
}

/**
 * @brief Displays a two-digit number on the LED matrix.
 *
 * @param number The number to display (0-99).
 */
void Animation_DrawNumber(uint8_t number) {
    uint8_t digit0 = number % 10;
    uint8_t digit1 = number / 10;
    uint64_t image = 0x00;

    for (uint8_t i = 1; i < 6; ++i) {
        image |= CHAR_TO_IMAGE64x1_ROW(i, font5x3_numbers[digit1][i]);
        image |= CHAR_TO_IMAGE64x1_ROW(i, font5x3_numbers[digit0][i]) >> 4;
    }

    MatrixSetImage(&mat, image);
    MatrixUpdate(&mat);
}

/**
 * @brief Displays a Roman numeral representation of a number (1-3).
 *
 * @param number The number to display.
 */
void Animation_DrawRomanNumber(uint8_t number) {
    number %= 4;
    if (number != 0) {
        uint64_t image = 0x00;
        for (uint8_t i = 0; i < 8; ++i) {
            image |= CHAR_TO_IMAGE64x1_ROW(i, RomanNumbers[number - 1][i]);
        }
        MatrixSetImage(&mat, image);
        MatrixUpdate(&mat);
    }
}

/**
 * @brief Scrolls a text message across the LED matrix.
 *
 * @param text The message to display.
 * @param reset If true, resets the scrolling animation.
 * @return true if the text has finished scrolling, false otherwise.
 */
bool_t Animation_ShiftText(char* const text, bool_t reset) {
    static uint8_t frame = 0;
    static uint64_t image = 0x00;
    static delay_t frameDelay;
    static char* message;
    static char current_char = 0x00;

    if (reset) {
        frame = 0;
        message = text;
        current_char = message[0];
        image = 0x00;
        delayInit(&frameDelay, 75);
    }

    if (delayRead(&frameDelay)) {
        image <<= 1;
        image &= 0xfefefefefefefefe;

        for (uint8_t i = 0; i < 8; ++i) {
            uint8_t byte = (font8x8_ib8x8u[current_char][i] >> (7 - frame % 8)) & 0x01;
            image |= CHAR_TO_IMAGE64x1_ROW(i, byte);
        }

        MatrixSetImage(&mat, image);
        MatrixUpdate(&mat);

        frame++;
        if (frame % 8 == 0) {
            current_char = message[frame / 8];
            if (current_char == '\0') {
                frame = 0;
                current_char = message[0];
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Displays a waiting animation.
 *
 * @param reset If true, resets the animation.
 * @return true when the animation cycle completes, false otherwise.
 */
bool_t Animation_WaitInput(bool_t reset) {
    static uint8_t frame = 0;
    static uint64_t image = 0x1054381000000038;
    static delay_t frameDelay;

    if (reset) {
        frame = 0;
        image = 0x1054381000000038;
        delayInit(&frameDelay, 180);
    }

    if (delayRead(&frameDelay)) {
        switch ( frame ) {
            case 0:
                image = 0x1054381000000038;
                break;
            case 1:
                image = 0x1010543810000038;
                break;
            case 2:
                image = 0x1010105438100038;
                break;
            case 3:
                image = 0x0010105438100038;
                break;
            case 4:
                image = 0x000010105438107c;
                break;
            case 5:
                image = 0x00001010543810fe;
                break;
        }

        MatrixSetImage(&mat, image);
        MatrixUpdate(&mat);
        frame = (frame + 1) % 6;
        return frame == 5;
    }

    return false;
}

/**
 * @brief Displays a loading animation.
 *
 * @param reset If true, resets the animation.
 * @return true when the animation cycle completes, false otherwise.
 */
bool_t Animation_Loading(bool_t reset)
{
    static uint8_t frame = 0;
    static uint8_t i = 2;
    static delay_t frameDelay;

    if ( reset ) {
        frame = 0;
        i = 2;
        delayInit(&frameDelay, 100);
    }

    if ( delayRead(&frameDelay) )
    {
        switch (frame) {
            case 0:
                MatrixSetImage(&mat, 0x0018181818181800);
                break;
            case 1:
                MatrixSetImage(&mat, 0x0002040810204000);
                break;
            case 2:
                MatrixSetImage(&mat, 0x7ffe000000);
                break;
            case 3:
                MatrixSetImage(&mat, 0x0040201008040200);
                i--;
                break;
            default:
                break;
        }
        MatrixUpdate(&mat);

        frame = (frame + 1) % 4;
        return i == 0;
    }

    return false;
}
