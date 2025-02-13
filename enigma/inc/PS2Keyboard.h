/* Version V1.0.9 (Modified for EDU-CIAA)

   	PS2Keyboard.h - Library for handling PS/2 keyboards on EDU-CIAA
	Based on the original library by Paul Carpenter.

	Copyright (c) 2007 Free Software Foundation. All rights reserved.
	Originally written by Paul Carpenter, PC Services <sales@pcserviceselectronics.co.uk>
	Created: September 2014
	Last original update: July 2021
	Modified in 2025 for adaptation to EDU-CIAA.

	### Original GitHub repository: https://github.com/techpaul/PS2KeyAdvanced

	### MODIFICATIONS FOR EDU-CIAA:
	- Removed compatibility with Arduino, ESP32, STM32.
	- Converted PS2KeyAdvanced from a C++ class (PS2KeyAdvanced.cpp) to a C implementation (PS2Keyboard.c), replacing class methods with standard functions.
	- Adjusted pin handling and register configuration for NXP LPC4337 architecture.
	- Improved comments for readability
	- Added functions `PS2Keyboard_EnableInt()` and `PS2Keyboard_DisableInt()` for manual control of PS/2 interrupt handling.

	### IMPORTANT:
	This library has been adapted for use with the EDU-CIAA board and the NXP LPC4337 microcontroller.
	It may not be compatible with other architectures without additional modifications.

	### ASSUMPTION: Only one PS/2 keyboard is connected.

	This implementation is designed for Latin-style keyboards using Scan Code Set 2, which is the default set for PS/2 keyboards on power-up.

	### MAIN FUNCTIONALITY:
	- PS/2 keyboard handling with Scan Set 2 protocol.
	- Supports key detection, command sending, and LED control.
	- Compatible with keyboards in different languages.

   --- DETAILED LIBRARY EXPLANATION ---

	### Overview
	This is a fully-featured PS/2 keyboard library that supports:
	- All function and movement keys, including multilingual layouts.
	- Parity checking for sent/received data and request-based keyboard resends.
	- Automatic handling of the PS/2 protocol for **RESEND** and **ECHO** commands.
	- Functions to:
	- Retrieve the active scancode set (read-only).
	- Control LED indicators and lock states.
	- Read the keyboard ID.
	- Reset the keyboard.
	- Send **ECHO** commands.
	- Manage **NUM LOCK**, **CAPS LOCK**, and **SCROLL LOCK** states and LEDs.
	- Manage **NUM LOCK** and **SCROLL LOCK** internally

	### Key Data Representation
	Keys are returned as a 16-bit (`uint16_t`) value containing:
	- **Make/Break** status.
	- **CAPS LOCK** state.
	- **Modifier keys**: SHIFT, CTRL, ALT, ALT GR, and GUI.
	- **Function key flag**, indicating non-printable characters.
	- **8-bit key code**.

	#### Key Code Ranges (Lower Byte of `uint16_t`). See PS2Keyboard.h for details
	- `0x00` -> Invalid/Error.
	- `0x01 - 0x1F` -> Function keys (e.g., CAPS, SHIFT, ALT, ENTER, DEL).
	- `0x1A - 0x1F` -> Function keys with ASCII control codes (DEL, BS, TAB, ESC, ENTER, SPACE).
	- `0x20 - 0x60` -> Printable characters:
		- `0x30 - 0x39` -> Digits (`0-9`).
		- `0x41 - 0x5A` -> Uppercase letters (`A-Z`).
	- `0x8B` -> Extra European key.
	- `0x61 - 0xA0` (less 0x8B) -> Function and special keys:
		- `0x61 - 0x78` -> Function keys (`F1 - F24`).
		- `0x79 - 0x8A` -> Multimedia keys.
		- `0x8C - 0x8E` -> ACPI power keys.
		- `0x91 - 0xA0` -> Special multilingual keys (`F1` and `F2` included).
	- `0xA8 - 0xFF` -> Keyboard communication commands (`F1` and `F2` have special multilingual codes).

	These ranges allow for key detection and efficient conversion to ASCII/UTF-8 while filtering out non-printable keys.

	### Key Flags (Upper Byte of `uint16_t`)
	Each bit represents a specific key state:

	| Define Name     | Bit Position  | Description |
	|-----------------|---------------|-------------|
	| `PS2_BREAK`     | 15 (MSB)      | `1` = Break key, `0` = Make key |
	| `PS2_SHIFT`     | 14            | `1` = SHIFT key pressed |
	| `PS2_CTRL`      | 13            | `1` = CTRL key pressed |
	| `PS2_CAPS`      | 12            | `1` = CAPS LOCK is ON |
	| `PS2_ALT`       | 11            | `1` = Left ALT key pressed |
	| `PS2_ALT_GR`    | 10            | `1` = Right ALT (ALT GR) key pressed |
	| `PS2_GUI`       | 9             | `1` = GUI key pressed |
	| `PS2_FUNCTION`  | 8             | `1` = Function key or non-printable character (SPACE, TAB, ENTER included), `0` = Standard character key |

	### Error Codes
	Most functions return `0x0000` or `0xFFFF` to indicate errors. Other notable return values (bottom byte):
	- `0xAA` -> Keyboard reset successfully and passed power-up tests. It will happen if keyboard plugged in after code start.
	- `0xFC` -> Keyboard general error or power-up failure.

   ### Custom Key Mapping
	Special key combinations, such as `<CTRL> + <ENTER>`, may require custom handling. The recommended approach is to use the **PS2KeyTable** library and extend it with a custom key mapping table. If modifying the base functionality, it is advised to create a separate library instead of altering this one to maintain compatibility.

	### Related files:
	- **PS2KeyCode.h** -> Contains original PS/2 keyboard scancodes used by this library.
	- **PS2KeyTable.h** -> Provides mapping tables for converting scancodes into ASCII/UTF-8 characters.

   See this file for returned definitions of Keys and accessible definitions.

   Refer to this file for the returned key definitions.
   - Defines starting with `PS2_KC_*` represent internal key codes from the keyboard.
   - Defines starting with `PS2_KEY_*` correspond to the key codes returned by this library.
   - Defines prefixed with `PS2_*` are intended for higher-level use.

   ### LICENSE:
	This software is open-source and distributed under the GNU Lesser General Public License (LGPL).
	For more details, see the license at: https://www.gnu.org/licenses/lgpl-2.1.html
*/

#ifndef PS2Keyboard_h
#define PS2Keyboard_h

#include "sapi.h"

/* Flags/bit masks for status bits in returned unsigned int value */
#define PS2_BREAK   0x8000
#define PS2_SHIFT   0x4000
#define PS2_CTRL    0x2000
#define PS2_CAPS    0x1000
#define PS2_ALT      0x800
#define PS2_ALT_GR   0x400
#define PS2_GUI      0x200
#define PS2_FUNCTION 0x100

/* General defines of communications codes */
/* Command or response */
#define PS2_KEY_RESEND   0xFE
#define PS2_KEY_ACK      0xFA
#define PS2_KEY_ECHO     0xEE
/* Responses */
#define PS2_KEY_BAT      0xAA
// Actually buffer overrun
#define PS2_KEY_OVERRUN  0xFF
// Below is general error code
#define PS2_KEY_ERROR    0xFC

/* Command parameters for functions */
/* LED codes OR together */
#define PS2_LOCK_SCROLL  0x01
#define PS2_LOCK_NUM     0x02
#define PS2_LOCK_CAPS    0x04
/* Only useful for very few keyboards */
#define PS2_LOCK_EXTRA   0x08

/* Returned keycode definitions */
/* Do NOT change these codings as you will break base
   functionality. Use PS2KeyTable for that and internationalization */
#define PS2_KEY_NUM         0x01
#define PS2_KEY_SCROLL      0x02
#define PS2_KEY_CAPS        0x03
#define PS2_KEY_PRTSCR      0x04
#define PS2_KEY_PAUSE       0x05
#define PS2_KEY_L_SHIFT     0x06
#define PS2_KEY_R_SHIFT     0x07
#define PS2_KEY_L_CTRL      0X08
#define PS2_KEY_R_CTRL      0X09
#define PS2_KEY_L_ALT       0x0A
#define PS2_KEY_R_ALT       0x0B
#define PS2_KEY_L_GUI       0x0C	// Sometimes called windows key
#define PS2_KEY_R_GUI       0x0D
#define PS2_KEY_MENU        0x0E
#define PS2_KEY_BREAK       0x0F	// Break is CTRL + PAUSE generated inside keyboard
#define PS2_KEY_SYSRQ       0x10	// Generated by some keyboards by ALT and PRTSCR
#define PS2_KEY_HOME        0x11
#define PS2_KEY_END         0x12
#define PS2_KEY_PGUP        0x13
#define PS2_KEY_PGDN        0x14
#define PS2_KEY_L_ARROW     0x15
#define PS2_KEY_R_ARROW     0x16
#define PS2_KEY_UP_ARROW    0x17
#define PS2_KEY_DN_ARROW    0x18
#define PS2_KEY_INSERT      0x19
#define PS2_KEY_DELETE      0x1A
#define PS2_KEY_ESC         0x1B
#define PS2_KEY_BS          0x1C
#define PS2_KEY_TAB         0x1D
#define PS2_KEY_ENTER       0x1E
#define PS2_KEY_SPACE       0x1F
#define PS2_KEY_KP0         0x20
#define PS2_KEY_KP1         0x21
#define PS2_KEY_KP2         0x22
#define PS2_KEY_KP3         0x23
#define PS2_KEY_KP4         0x24
#define PS2_KEY_KP5         0x25
#define PS2_KEY_KP6         0x26
#define PS2_KEY_KP7         0x27
#define PS2_KEY_KP8         0x28
#define PS2_KEY_KP9         0x29
#define PS2_KEY_KP_DOT      0x2A
#define PS2_KEY_KP_ENTER    0x2B
#define PS2_KEY_KP_PLUS     0x2C
#define PS2_KEY_KP_MINUS    0x2D
#define PS2_KEY_KP_TIMES    0x2E
#define PS2_KEY_KP_DIV      0x2F
#define PS2_KEY_0           0X30
#define PS2_KEY_1           0X31
#define PS2_KEY_2           0X32
#define PS2_KEY_3           0X33
#define PS2_KEY_4           0X34
#define PS2_KEY_5           0X35
#define PS2_KEY_6           0X36
#define PS2_KEY_7           0X37
#define PS2_KEY_8           0X38
#define PS2_KEY_9           0X39
#define PS2_KEY_APOS        0X3A
#define PS2_KEY_COMMA       0X3B
#define PS2_KEY_MINUS       0X3C
#define PS2_KEY_DOT         0X3D
#define PS2_KEY_DIV         0X3E
#define PS2_KEY_KP_EQUAL    0x3F	// Some numeric keyboards have an '=' on right keypad
#define PS2_KEY_SINGLE      0X40	// Single quote or back quote
#define PS2_KEY_A           0X41
#define PS2_KEY_B           0X42
#define PS2_KEY_C           0X43
#define PS2_KEY_D           0X44
#define PS2_KEY_E           0X45
#define PS2_KEY_F           0X46
#define PS2_KEY_G           0X47
#define PS2_KEY_H           0X48
#define PS2_KEY_I           0X49
#define PS2_KEY_J           0X4A
#define PS2_KEY_K           0X4B
#define PS2_KEY_L           0X4C
#define PS2_KEY_M           0X4D
#define PS2_KEY_N           0X4E
#define PS2_KEY_O           0X4F
#define PS2_KEY_P           0X50
#define PS2_KEY_Q           0X51
#define PS2_KEY_R           0X52
#define PS2_KEY_S           0X53
#define PS2_KEY_T           0X54
#define PS2_KEY_U           0X55
#define PS2_KEY_V           0X56
#define PS2_KEY_W           0X57
#define PS2_KEY_X           0X58
#define PS2_KEY_Y           0X59
#define PS2_KEY_Z           0X5A
#define PS2_KEY_SEMI        0X5B
#define PS2_KEY_BACK        0X5C
#define PS2_KEY_OPEN_SQ     0X5D
#define PS2_KEY_CLOSE_SQ    0X5E
#define PS2_KEY_EQUAL       0X5F
#define PS2_KEY_KP_COMMA    0x60	// Some numeric keypads have a comma key
#define PS2_KEY_F1          0X61
#define PS2_KEY_F2          0X62
#define PS2_KEY_F3          0X63
#define PS2_KEY_F4          0X64
#define PS2_KEY_F5          0X65
#define PS2_KEY_F6          0X66
#define PS2_KEY_F7          0X67
#define PS2_KEY_F8          0X68
#define PS2_KEY_F9          0X69
#define PS2_KEY_F10         0X6A
#define PS2_KEY_F11         0X6B
#define PS2_KEY_F12         0X6C
#define PS2_KEY_F13         0X6D
#define PS2_KEY_F14         0X6E
#define PS2_KEY_F15         0X6F
#define PS2_KEY_F16         0X70
#define PS2_KEY_F17         0X71
#define PS2_KEY_F18         0X72
#define PS2_KEY_F19         0X73
#define PS2_KEY_F20         0X74
#define PS2_KEY_F21         0X75
#define PS2_KEY_F22         0X76
#define PS2_KEY_F23         0X77
#define PS2_KEY_F24         0X78
#define PS2_KEY_NEXT_TR     0X79
#define PS2_KEY_PREV_TR     0X7A
#define PS2_KEY_STOP        0X7B
#define PS2_KEY_PLAY        0X7C
#define PS2_KEY_MUTE        0X7D
#define PS2_KEY_VOL_UP      0X7E
#define PS2_KEY_VOL_DN      0X7F
#define PS2_KEY_MEDIA       0X80
#define PS2_KEY_EMAIL       0X81
#define PS2_KEY_CALC        0X82
#define PS2_KEY_COMPUTER    0X83
#define PS2_KEY_WEB_SEARCH  0X84
#define PS2_KEY_WEB_HOME    0X85
#define PS2_KEY_WEB_BACK    0X86
#define PS2_KEY_WEB_FORWARD 0X87
#define PS2_KEY_WEB_STOP    0X88
#define PS2_KEY_WEB_REFRESH 0X89
#define PS2_KEY_WEB_FAVOR   0X8A
#define PS2_KEY_EUROPE2     0X8B
#define PS2_KEY_POWER       0X8C
#define PS2_KEY_SLEEP       0X8D
#define PS2_KEY_WAKE        0X90
#define PS2_KEY_INTL1       0X91
#define PS2_KEY_INTL2       0X92
#define PS2_KEY_INTL3       0X93
#define PS2_KEY_INTL4       0X94
#define PS2_KEY_INTL5       0X95
#define PS2_KEY_LANG1       0X96
#define PS2_KEY_LANG2       0X97
#define PS2_KEY_LANG3       0X98
#define PS2_KEY_LANG4       0X99
#define PS2_KEY_LANG5       0xA0

 /**
 * @brief Initializes the PS/2 keyboard interface.
 *
 * This function configures the necessary GPIO pins and interrupt settings
 * to enable communication with a PS/2 keyboard. It sets the data and clock
 * pins as inputs with pull-up resistors and configures a GPIO interrupt in channel 0 to
 * trigger on the falling edge of the clock signal.
 *
 * @param dataPin     GPIO pin used for the PS/2 data line.
 * @param irqPin      GPIO pin used for the PS/2 clock line (interrupt trigger).
 */
void PS2Keyboard_Init(gpioMap_t dataPin, gpioMap_t irqPin);

/**
 * @brief Enables the PS/2 interrupt after resetting the state machine.
 *
 * This function resets the internal PS/2 state and enables the external interrupt
 * associated with the PS/2 clock line, allowing the system to receive keyboard data.
 */
void PS2Keyboard_EnableInt(void);


/**
 * @brief Disables the PS/2 interrupt.
 *
 * This function disables the external interrupt associated with the PS/2 clock line,
 * preventing further keyboard data reception until re-enabled.
 */
void PS2Keyboard_DisableInt(void);


/**
 * @brief Returns the count of available processed key codes.
 *
 * This function checks the internal buffers to count how many key codes are available.
 * If the processed key buffer is full, it will return the maximum count.
 * Otherwise, it processes the input key buffer until the input buffer is empty or the
 * output buffer is full.
 *
 * @return uint8_t The count of processed key codes available in the buffer, or 0 if the buffer is empty.
 */
uint8_t PS2Keyboard_Available(void);


/**
 * @brief Returns the key last read from the keyboard.
 *
 * This function retrieves the last key that was read from the internal buffer.
 * If the buffer is empty, it returns 0.
 *
 * @return uint16_t The key code of the last read key, or 0 if the buffer is empty.
 */
uint16_t PS2Keyboard_Read(void);


/**
 * @brief Retrieves the current status of the lock keys.
 * 
 * The function returns a byte where each bit represents the state of a lock key.
 * Use the predefined macros to extract specific lock states.
 *   - PS2_LOCK_SCROLL
 *   - PS2_LOCK_NUM
 *   - PS2_LOCK_CAPS
 *   - PS2_LOCK_EXTRA (rarely used)
 * 
 * Example usage:
 * if (PS2Keyboard_GetLock() & PS2_LOCK_NUM) {
 *     // Num Lock is active
 * }
 * 
 * @return uint8_t - A byte with lock key states.
 */
uint8_t PS2Keyboard_GetLock();


/**
 * @brief Sets the status of the lock keys and updates the keyboard LEDs.
 * 
 * This function modifies the state of the lock keys (Num Lock, Caps Lock, 
 * Scroll Lock, and Extra Lock) and ensures the keyboard LEDs reflect the change.
 * 
 * The bits in the `code` parameter correspond to the following macros:
 *   - PS2_LOCK_SCROLL
 *   - PS2_LOCK_NUM
 *   - PS2_LOCK_CAPS
 *   - PS2_LOCK_EXTRA (only useful for very few keyboards)
 * 
 * @param code A byte where each bit represents the desired lock state.
 */
void PS2Keyboard_SetLock(uint8_t code);


/**
 * @brief Enables or disables the sending of break key codes.
 * 
 * When enabled, the library will suppress break key codes (key releases). 
 * When disabled, break key codes will be sent as usual.
 * 
 * @param data Set to 1 to disable break codes, 0 to enable them.
 */
void PS2Keyboard_SetNoBreak(uint8_t data);


/**
 * @brief Enables or disables repeated make codes for modifier keys.
 * 
 * This setting controls whether the library should suppress repeated 
 * make codes for modifier keys (_CTRL, _ALT, _GUI, _SHIFT). 
 * 
 * @param data Set to 1 to disable repeated make codes, 0 to allow them.
 */
void PS2Keyboard_SetNoRepeat(uint8_t data);


/**
 * @brief Resets the keyboard and clears the internal lock and key status.
 * 
 * Sends a reset command to the keyboard. The keyboard will respond with:
 * - `0xAA` = PS2_KC_BAT 	(Self-test passed)
 * - `0xFC` = PS2_KC_ERROR 	(Self-test failed)
 * 
 * This function also resets the internal LED lock status and key status 
 * to match the keyboard’s state after reset.
 */
void PS2Keyboard_ResetKey();


/**
 * @brief Requests the current scan code set used by the keyboard.
 *
 * This function sends a command to the keyboard to retrieve the currently active
 * scan code set. The response is stored in the keyboard buffer and can be read
 * using buffer-processing functions.
 *
 * Scan code sets define how key presses are translated into data bytes. 
 * The default set for most PS/2 keyboards is Scan Code Set 2.
 */
void PS2Keyboard_GetScanCodeSet(void);


/**
 * @brief Requests the keyboard identification (ID).
 *
 * This function sends the "Read ID" command to the keyboard, which responds with
 * a two-byte identifier. The response is stored in the keyboard buffer and can
 * be read using buffer-processing functions.
 */
void PS2Keyboard_ReadID(void);


/**
 * @brief Sends an echo command to the keyboard.
 *
 * This function sends the PS/2 ECHO command to the keyboard, which should respond
 * with the same ECHO code. The response is stored in the keyboard buffer and can
 * be read using buffer-processing functions.
 * 
 * The response from the keyboard is stored in the input buffer and can be read
 * using the appropriate buffer-reading functions.
 */
void PS2Keyboard_Echo(void);


/**
 * @brief Sends a typematic rate and delay command to the keyboard.
 *
 * This function configures the typematic rate and delay of the keyboard, which determines
 * how fast a key repeats when held down and the delay before the first repeat occurs.
 * The typematic rate is defined as the number of characters per second (CPS), and the delay
 * defines the time interval before the first repeat occurs.
 * 
 * The response from the keyboard is stored in the input buffer and can be read
 * using the appropriate buffer-reading functions.
 *
 * @param rate Typematic rate (0 - 0x1F, where 0 = 30 CPS and 0x1F = 2 CPS). Default is 0xB (10.9 CPS).
 * @param delay Typematic delay (0 - 3), defining the delay before the first key repeat (0.25s to 1s in 0.25s increments).
 *              Default is 1 (0.5s delay).
 *
 * @return int Error code (0 = OK, -5 = parameter error).
 */
int PS2Keyboard_Typematic(uint8_t rate, uint8_t delay);
#endif
