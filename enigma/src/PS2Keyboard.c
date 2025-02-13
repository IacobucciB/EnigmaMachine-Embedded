/*	Version V1.0.9 (Modified for EDU-CIAA)

	PS2Keyboard.c - Library for handling PS/2 keyboards on EDU-CIAA
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
	- **PS2Keyboard.h** -> Contains returned key definitions and accessible mappings.
	- **PS2KeyCode.h** -> Contains original PS/2 keyboard scancodes used by this library.
	- **PS2KeyTable.h** -> Provides mapping tables for converting scancodes into ASCII/UTF-8 characters.

	### LICENSE:
	This software is open-source and distributed under the GNU Lesser General Public License (LGPL).
	For more details, see the license at: https://www.gnu.org/licenses/lgpl-2.1.html
*/

#include "sapi.h"
// Internal headers for library defines/codes/etc
#include "PS2Keyboard.h"
#include "PS2KeyCode.h"
#include "PS2KeyTable.h"

/*=====[Definitions of extern global variables]==============================*/

/* References for system configuration */
extern pinInitGpioLpc4337_t gpioPinsInit[];  // GPIO pin initialization
extern tick_t tickRateMS;  // System tick rate in milliseconds

// Private function declarations (internal use only)
static void send_bit(void);
static void send_now(uint8_t);
static int16_t send_next(void);
static void ps2_reset(void);
static uint8_t decode_key(uint8_t);
static void set_lock();

/**** Constant control functions to flags array
 in translated key code value order  ****/

/* Control flags for key decoding, mapped to translated key code values */
#if defined(PS2_REQUIRES_PROGMEM)
const uint8_t PROGMEM control_flags[] = {
#else
const uint8_t control_flags[] = {
#endif
    _SHIFT, _SHIFT, _CTRL, _CTRL,
    _ALT, _ALT_GR, _GUI, _GUI
};

// ------------------------------
// PS/2 Communication State Variables
// ------------------------------

/*=====[Definitions of private global variables]=============================*/

/*
 * _ps2mode: Stores various PS/2 communication flags.
 *
 * Bit 7 - _PS2_BUSY:       Set when the bus is active (RX/TX in progress).
 * Bit 6 - _TX_MODE:        1 = Transmitting, 0 = Receiving (default).
 * Bit 5 - _BREAK_KEY:      Set when a break code (key release) is detected.
 * Bit 4 - _WAIT_RESPONSE:  Set when waiting for a response from the keyboard.
 * Bit 3 - _E0_MODE:        Set when an E0-prefixed scancode is being processed.
 * Bit 2 - _E1_MODE:        Set when an E1-prefixed scancode is being processed.
 * Bit 1 - _LAST_VALID:     Set if the last sent byte was acknowledged correctly. If a resend request is received and no other byte was sent in the meantime, this flag determines whether the last byte should be resent.
 */
volatile uint8_t _ps2mode;

/*
 * RX Buffers and Variables (Updated via Interrupts)
 */
volatile uint16_t _rx_buffer[_RX_BUFFER_SIZE];  // Circular buffer for received keyboard data
volatile uint8_t _head;  // Points to the last written byte in the RX buffer
uint8_t _tail;           // Points to the last read byte (not modified in IRQ)
volatile int8_t _bytes_expected;  // Expected bytes in the current transmission
volatile uint8_t _bitcount;  // Tracks the bit count for received data
volatile uint8_t _shiftdata; // Stores the received/sent data
volatile uint8_t _parity;    // Stores parity bit status for validation

/*
 * TX Buffers and Variables (For Sending Commands to Keyboard)
 */
volatile uint8_t _tx_buff[_TX_BUFFER_SIZE];  // Buffer for outgoing keyboard commands
volatile uint8_t _tx_head;        // Write pointer for TX buffer
volatile uint8_t _tx_tail;        // Read pointer for TX buffer
volatile uint8_t _last_sent;      // Stores the last sent byte in case of resend request
volatile uint8_t _now_send;       // Holds a byte for immediate transmission
volatile uint8_t _response_count; // Number of bytes expected in response to the next TX command
volatile uint8_t _tx_ready;       // TX status flag for managing transmission types
/*
 * _tx_ready flags:
 * Bit 7 - _HANDSHAKE (0x80): Handshaking command (ECHO/RESEND).
 * Bit 0 - _COMMAND   (0x01): Regular command processing.
 */

/*
 * Output Key Buffering (For Processed Keystrokes)
 */
uint16_t _key_buffer[_KEY_BUFF_SIZE];  // Circular buffer for translated key events
uint8_t _key_head;  // Write pointer for the key buffer
uint8_t _key_tail;  // Read pointer for the key buffer
uint8_t _mode = 0; 	// Keyboard Output Mode Flags
/*
 * _mode flags:
 * Bit 7 - _NO_REPEATS (0x80): Disables repeat make codes for _CTRL, _ALT, _SHIFT, _GUI.
 * Bit 3 - _NO_BREAKS  (0x08): Disables break codes (key release events).
 */

/*
 * PS/2 Hardware Pin Configuration
 * (Set during initialization)
 */
uint8_t PS2_DataPin;     // Data line pin
uint8_t PS2_IrqPin;      // IRQ line pin

/*
 * Keyboard Lock and Status Variables
 */
uint8_t PS2_led_lock = 0;      // Stores the current LED lock status (Caps Lock, Num Lock, etc.)
uint8_t PS2_lockstate[4];      // Tracks whether locks had break events
uint8_t PS2_keystatus;         // Stores the current CAPS/Shift/Alt/etc. status

/*------------------ Code starts here -------------------------*/

/**
 * @brief Interrupt Service Routine for the PS/2 keyboard external interrupt.
 *
 * This ISR is triggered on each falling edge of the PS/2 clock signal.
 * It handles both receiving and transmitting data:
 * - If in TX mode, calls `send_bit()` to send data.
 * - If in RX mode, reads incoming data bits, performs parity checking,
 *   and stores valid bytes in the receive buffer.
 */
void GPIO0_IRQHandler(void) {
    // Check if the interrupt was triggered by the clock pin
    if (Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & PININTCH(0)) {
        // Clear the interrupt flag
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));

        // If in transmit mode, send the next bit
        if (_ps2mode & _TX_MODE) {
            send_bit();
        } else {
            // Receiving mode
            static tick_t prev_ticks = 0;
            tick_t now_ticks;
            uint8_t val, ret;

            val = gpioRead(PS2_DataPin);  // Read data line state

            // Timeout handling for glitches (if a delay exceeds 250ms, reset)
            now_ticks = tickRead();
            uint32_t elapsed_ms = (now_ticks - prev_ticks) * tickRateMS;
            if (elapsed_ms > 250) {
                _bitcount = 0;
                _shiftdata = 0;
            }
            prev_ticks = now_ticks;

            _bitcount++;  // Move to the next bit

            switch (_bitcount) {
                case 1: // Start bit
                    _parity = 0;
                    _ps2mode |= _PS2_BUSY;  // Mark as busy
                    break;

                case 2 ... 9: // Data bits (8 bits)
                    _parity += val;         // Update parity calculation
                    _shiftdata >>= 1;       // Shift bits to the right
                    if (val) _shiftdata |= 0x80;  // Store the received bit
                    break;

                case 10: // Parity check bit
                    _parity &= 1;  // Extract LSB (should be 0 for odd parity)
                    if (_parity == val) {
                        _parity = 0xFD;  // Parity error detected
                    }
                    break;

                case 11: // Stop bit (end of transmission)
                    if (_parity >= 0xFD) {  // Parity error detected
                        send_now(PS2_KC_RESEND);  // Request data resend
                        _tx_ready |= _HANDSHAKE;
                    } else {  // Data is valid, store in buffer
                        ret = decode_key(_shiftdata);  // Decode received key

                        if (ret & 0x02) _bytes_expected--;  // Decrease expected bytes

                        if (_bytes_expected <= 0 || (ret & 0x04)) {  // Save received byte
                            uint8_t next_head = _head + 1;
                            if (next_head >= _RX_BUFFER_SIZE) next_head = 0;

                            if (next_head != _tail) {  // Ensure buffer does not overflow
                                _rx_buffer[next_head] = (uint16_t)_shiftdata;
                                _rx_buffer[next_head] |= ((uint16_t)_ps2mode) << 8;
                                _head = next_head;
                            }
                        }

                        if (ret & 0x10) {  // Special command (ECHO/RESEND)
                            send_now(_now_send);
                            _tx_ready |= _HANDSHAKE;
                        } else if (_bytes_expected <= 0) {  // Reception complete
                            // Reset state for next byte reception
                            _ps2mode &= ~(_E0_MODE | _E1_MODE | _WAIT_RESPONSE | _BREAK_KEY);
                            _bytes_expected = 0;
                            _ps2mode &= ~_PS2_BUSY;
                            send_next();  // Check for pending transmissions
                        }
                    }
                    _bitcount = 0;  // Reset for next byte
                    break;

                default: // In case of weird error and end of byte reception re-sync
                    _bitcount = 0;
            }
        }
    }
}

/**
 * @brief Decodes the received PS/2 scan code to determine its meaning.
 *
 * This function processes received PS/2 scan codes and determines the appropriate action.
 * It checks for errors, special commands, and responses.
 *
 * @param value The received scan code byte.
 * @return uint8_t A bitwise OR'd status code indicating the necessary action:
 *         - 0x10: A command should be sent (stored in `_now_send`).
 *         - 0x08: Reception should be aborted, resetting status and buffers.
 *         - 0x04: The received value should be saved after translation.
 *         - 0x02: The expected byte count should be decremented.
 *         - 0x06: Default state (save and decrement expected bytes).
 */
static uint8_t decode_key(uint8_t value) {
    uint8_t state = 6;  // Default: save and decrement expected bytes

    // Clear last valid send flag unless it's a resend command
    if (value != PS2_KC_RESEND) {
        _ps2mode &= ~_LAST_VALID;
    }

    // If waiting for a response from the host, treat valid responses normally
    if (_ps2mode & _WAIT_RESPONSE) {
        if (value < 0xF0) {
            return state;  // Save response and decrement expected count
        }
    }

    // If in E1 (Pause key) mode, just decrement expected bytes and return
    if (_ps2mode & _E1_MODE) {
        return 2;
    }

    switch (value) {
        case 0:
        case PS2_KC_OVERRUN:  // Buffer overrun or error, reset everything
            ps2_reset();
            state = 0x0C;
            break;

        case PS2_KC_RESEND:  // Request to resend last sent byte
            if (_ps2mode & _LAST_VALID) {
                _now_send = _last_sent;
                state = 0x10;  // Indicate command should be sent
            } else {
                state = 0;
            }
            break;

        case PS2_KC_ERROR:  // General error, stop all sending and receiving
            _bytes_expected = 0;
            _ps2mode = 0;
            _tx_ready = 0;
            state = 0x0E;
            break;

        case PS2_KC_KEYBREAK:  // Break code, expect one more byte
            _bytes_expected = 1;
            _ps2mode |= _BREAK_KEY;
            state = 0;
            break;

        case PS2_KC_ECHO:  // Echo response, ensure it wasn’t self-initiated
            state = 4;  // Always save
            if ((_ps2mode & _LAST_VALID) && _last_sent != PS2_KC_ECHO) {
                _now_send = PS2_KC_ECHO;
                state |= 0x10;  // Indicate command should be sent
            }
            break;

        case PS2_KC_BAT:  // BAT (Basic Assurance Test) pass, reset expected bytes
            _bytes_expected = 0;
            state = 4;
            break;

        case PS2_KC_EXTEND1:  // Extended PAUSE key sequence
            if (!(_ps2mode & _E1_MODE)) {  // Only first E1 is valid
                _bytes_expected = 7;  // Expect 7 more bytes
                _ps2mode |= _E1_MODE;
                _ps2mode &= ~_BREAK_KEY;  // Always a make (press)
            }
            state = 0;
            break;

        case PS2_KC_EXTEND:  // Extended two-byte sequence
            _bytes_expected = 1;  // Expect at least one more byte
            _ps2mode |= _E0_MODE;
            state = 0;
            break;
    }
    return state;
}


/**
 * @brief Sends a bit (or byte) to the PS/2 keyboard.
 *
 * This function sends a series of bits (start bit, data bits, parity bit, stop bit, and acknowledgment bit)
 * to the PS/2 keyboard. It handles bit-level communication by shifting the data and setting the appropriate
 * GPIO pin states for each bit in the sequence.
 *
 * The data pin direction should already be set before calling this function.
 * The start bit is already set, and the `_bitcount` and `_parity` should be initialized before this function is called.
 */
static void send_bit(void) {
    uint8_t val;

    _bitcount++;  // Increment bit count to point to the next bit in sequence

    switch (_bitcount) {
        case 1:
// #if defined(PS2_CLEAR_PENDING_IRQ)
//             gpioWrite(PS2_DataPin, OFF);  // Start bit set to 0 (OFF)
//             break;
// #endif
        case 2 ... 9:
            // Data bits (LSB first)
            val = _shiftdata & 0x01;  // Extract the least significant bit (LSB)
            gpioWrite(PS2_DataPin, val);  // Send the bit to the keyboard
            _parity += val;  // Update the parity if the bit was 1
            _shiftdata >>= 1;  // Shift data right to prepare the next bit
            break;

        case 10:
            // Parity bit - Set the parity (0 or 1) based on the number of 1s in the data
            gpioWrite(PS2_DataPin, (~_parity & 1));  // If odd parity, send 0 (parity bit = 0)
            break;

        case 11:
            // Stop bit - Change pin configuration to input pull-up (high) for stop bit
            gpioConfig(PS2_DataPin, GPIO_INPUT_PULLUP);
            break;

        case 12:
            // Acknowledgment bit - send acknowledgment (low for success)
            if (!(_now_send == PS2_KC_ECHO || _now_send == PS2_KC_RESEND)) {
                _last_sent = _now_send;  // Save the last sent byte in case of resend request
                _ps2mode |= _LAST_VALID;  // Mark this as the last valid byte sent
            }
            // Clear the transmission mode for receiving again
            _ps2mode &= ~_TX_MODE;

            // Handle handshake or command completion
            if (_tx_ready & _HANDSHAKE) {
                _tx_ready &= ~_HANDSHAKE;  // If handshake is complete, clear handshake flag
            } else {
                _tx_ready &= ~_COMMAND;  // If command finished, clear command flag
            }

            // If not waiting for a response, check for any pending commands to send
            if (!(_ps2mode & _WAIT_RESPONSE)) {
                send_next();  // Check if there is anything else to queue up
            }

            _bitcount = 0;  // Reset bit count to indicate end of byte
            break;

        default:
            // Error handling: reset bit count if there is an issue with the transmission
            _bitcount = 0;
    }
}


/**
 * @brief Sends a byte to the PS/2 keyboard.
 *
 * This function takes a byte (command), sets up the necessary variables, and begins the data transmission process to the PS/2 keyboard.
 * The calling code must ensure that the line is idle and ready to send before invoking this function.
 *
 * While this function introduces long delays, these delays will stop the interrupt source (keyboard) externally when the clock is held low.
 *
 * The transmission process involves managing the _tx_ready flags, which determine the type of command being sent:
 * - _HANDSHAKE: Command part of receiving data, e.g., ECHO, RESEND.
 * - _COMMAND: Other commands not related to receiving data.
 *
 * A key difference is that _bytes_expected is NOT altered in _HANDSHAKE mode, but is updated in command mode with the expected number of response bytes.
 *
 * @param command The byte to be transmitted to the PS/2 keyboard.
 */
static void send_now(uint8_t command) {
    _shiftdata = command;  // Prepare the command byte for transmission
    _now_send = command;   // Save the command byte for later (used for resend/echo)

#if defined( PS2_CLEAR_PENDING_IRQ )
    _bitcount = 0;  // Reset bit count to handle any pending interrupts on AVR/SAM
#else
    _bitcount = 1;  // Normal processors: start with bit count 1
#endif

    _parity = 0;  // Reset parity counter
    _ps2mode |= _TX_MODE + _PS2_BUSY;  // Set transmission mode and mark PS2 as busy

    // Only proceed with command mode if we're not in handshake mode
    if (!(_tx_ready & _HANDSHAKE) && (_tx_ready & _COMMAND)) {
        _bytes_expected = _response_count;  // Set the number of expected response bytes for the command
        _ps2mode |= _WAIT_RESPONSE;  // Wait for a response from the keyboard
    }

    /* STOP interrupt handler */
    // Disabling interrupt temporarily to prevent interference while preparing the data line
    NVIC_ClearPendingIRQ( PIN_INT0_IRQn + 0 );
    NVIC_DisableIRQ( PIN_INT0_IRQn + 0 );

    // Set the data pin to output and set it high (idle state)
    gpioWrite(PS2_DataPin, ON);
    gpioConfig(PS2_DataPin, GPIO_OUTPUT);

    // Set IRQ pin high (idle state)
    gpioWrite(PS2_IrqPin, ON);
    gpioConfig(PS2_IrqPin, GPIO_OUTPUT);

    // Compliance with PS2 spec: wait for 10us
    delayInaccurateUs(10);

    // Set the clock low (this will trigger a clock pulse)
    gpioWrite(PS2_IrqPin, ON);
    // PS2 spec compliance: hold clock low for 60us
    delayInaccurateUs(60);

    // Set data line low to indicate the start bit
    gpioWrite(PS2_DataPin, OFF);
    // Set clock pin to input pull-up while data is being sent
    gpioConfig(PS2_IrqPin, GPIO_INPUT_PULLUP);

    /* Restart interrupt handler */
    // Re-enable the interrupt to allow further clock signals to be sent
    NVIC_ClearPendingIRQ( PIN_INT0_IRQn + 0 );
    NVIC_EnableIRQ( PIN_INT0_IRQn + 0 );

    // Wait for clock interrupt to continue sending data
}


/**
 * @brief Send the next byte/command from the TX queue and start the transmission.
 *
 * This function is used to send a byte or command to the device over the PS2 interface.
 * It checks the state of the transmission queue and sends the next byte. If the queue is empty,
 * or the system is busy, it will return an error code. After sending the byte, it determines if there
 * are further bytes to send or if it is waiting for a response.
 *
 * Assumes commands consist of one or more bytes and may or may not be followed by a response.
 *
 * @note PS2_KEY_IGNORE is used to mark bytes that are expected as a response, but should not be sent.
 *
 * @retval 1 If the transmission was started or queued.
 * @retval -2 If the buffer is empty (no bytes to send).
 * @retval -134 If the system is busy and cannot send the byte right now (checked by the interrupt routines later).
 */
static int16_t send_next(void) {
    uint8_t i;        	/**< Index to scan the transmission buffer */
	int16_t val = -1;   /**< Value of the byte to send, initialized to -1 to signal no byte yet */

    /* Check if the transmission buffer is empty (no data to send) */
    i = _tx_tail;
    if (i == _tx_head)
        return -2;      /**< Return -2 indicating empty buffer */

    /* Set the command bit in _tx_ready to indicate that another command needs to be sent */
    _tx_ready |= _COMMAND;

    /* Check if there is already an item waiting to be sent or if the interrupt routines are busy */
    if (_tx_ready & _HANDSHAKE)
        return -134;

    /* If the system is in busy mode, let the interrupt catch it and process later */
    if (_ps2mode & _PS2_BUSY)
        return -134;

    /* Only proceed if we're not currently receiving or sending protocol bytes */
    /* Scan through the buffer to identify the next byte to send and any expected response bytes */
    _response_count = 0;
    do {
        i++;
        if (i >= _TX_BUFFER_SIZE)  /**< If we go beyond the buffer size, wrap around */
            i = 0;

        /* If no byte has been found yet, take the byte from the buffer */
        if (val == -1)
            val = _tx_buff[i];
        else if (_tx_buff[i] != PS2_KEY_IGNORE)
            break;  /**< Exit the loop as we've found the byte to send */
        else
            _response_count++;  /**< If the byte is marked as ignore, increment the response count */

        /* Update the tail pointer to the new position */
        _tx_tail = i;
    } while (i != _tx_head);  /**< Keep looping until we reach the head of the buffer */

    /* Now we know which byte to send and how many bytes we are expecting as a response */
    send_now(val);  /**< Call the send_now function to start transmitting the byte */

    return 1;  /**< Return 1 to indicate that the transmission has started or queued */
}


/**
 * @brief Send a byte to the TX buffer.
 *
 * This function adds a byte to the transmission buffer. If the buffer is not full,
 * the byte is written to the buffer and the head pointer is updated. If the buffer is
 * full, the function returns an error code (-4) indicating a buffer overrun.
 * If a byte is added successfully, it returns 1.
 *
 * The buffer is used to store data that is to be sent over the PS2 interface.
 * If the byte value is `PS2_KEY_IGNORE`, it signifies that the system is waiting for a response,
 * and this should be used for each byte expected in the response.
 *
 * @param val The byte to be sent to the TX buffer.
 *
 * @retval 1 If the byte was successfully written to the buffer.
 * @retval -4 If the buffer is full and the byte cannot be written (buffer overrun).
 */
static int send_byte(uint8_t val) {
    uint8_t ret;

    /* Calculate the next head position in the circular buffer */
    ret = _tx_head + 1;
    if (ret >= _TX_BUFFER_SIZE)  /**< Wrap around to 0 if the end of the buffer is reached */
        ret = 0;

    /* Check if the buffer is full (head pointer is next to the tail pointer) */
    if (ret != _tx_tail) {
        _tx_buff[ret] = val;  /**< Write the byte to the buffer */
        _tx_head = ret;       /**< Update the head pointer */
        return 1;  /**< Return 1 to indicate the byte was successfully written */
    }

    return -4;  /**< Return -4 if the buffer is full and the byte could not be written */
}

/**
 * @brief Reset the PS2 communication buffers and states.
 *
 * This function resets all relevant buffers, flags, and internal states used for PS2
 * communication. It clears the TX buffer pointers, prepares the system for a fresh start,
 * and resets mode flags to ensure no residual state affects subsequent operations.
 *
 * This reset is typically called when reinitializing or recovering the PS2 interface
 * to ensure all variables are in a known, clean state.
 *
 * @note This function does not reset the hardware, but only the internal software states
 *       associated with the PS2 communication process.
 */
static void ps2_reset(void) {
    /* reset buffers and states */
    _tx_head = 0;
    _tx_tail = 0;
    _tx_ready = 0;
    _response_count = 0;
    _head = 0;
    _tail = 0;
    _bitcount = 0;
    PS2_keystatus = 0;
    PS2_led_lock = 0;
    _ps2mode = 0;
}

/**
 * @brief Check the number of available keys in the RX buffer.
 *
 * This function calculates how many bytes (keys) are currently available in the RX buffer
 * by checking the difference between the head and tail pointers. If the buffer is circular,
 * the function properly handles the wraparound by adjusting the calculation.
 *
 * @return The number of available keys in the buffer.
 *         A value of 0 indicates no keys are available.
 */
static uint8_t key_available() {
    int8_t i;

    i = _head - _tail;
    if (i < 0)
        i += _RX_BUFFER_SIZE;
    return (uint8_t) i;
}

/**
 * @brief Translate a PS2 keyboard scan code sequence into a key code data.
 *
 * This function reads a sequence of bytes from the RX buffer and translates it into
 * a corresponding key code. The function is designed to process key events when the
 * data is read from the buffer. It processes the buffer in a non-interrupt context,
 * called from the read function.
 *
 * Key points:
 * - The PAUSE key (_E1_MODE) is treated as a special case.
 * - Command responses are not translated but returned as raw data.
 * - Lock keys toggle their state when pressed and are ignored when repeated.
 * - Modifier keys (e.g., Shift, Ctrl) are processed and handled in the `PS2_keystatus`.
 * - Function keys and numeric keypad keys are processed according to lock states and shift status.
 * - Break codes are handled appropriately.
 *
 * @return A translated key code with the PS2 keystatus encoded in the upper byte.
 *         - 0 if no valid key is processed, empty buffer, or ignored key.
 */
static uint16_t translate(void) {
    uint8_t index, length, data;
    uint16_t retdata;

    // Get the next character from the buffer
    // Check if there's anything to fetch
    index = _tail;

    // If the buffer is empty, return 0
    if (index == _head)
        return 0;

    index++;

    // Wrap around the index if it exceeds buffer size
    if (index >= _RX_BUFFER_SIZE)
        index = 0;

    // Update the tail pointer to the new index
    _tail = index;

    // Get the flags byte (break modes, etc.)
    data = _rx_buffer[index] & 0xFF; // Lower byte for data
    index = (_rx_buffer[index] & 0xFF00) >> 8; // Upper byte for flags

    // Check if it's a special PAUSE key (E1 mode is enabled)
    if (index & _E1_MODE)
        return PS2_KEY_PAUSE + _FUNCTION; // Return a specific key code for PAUSE

    // Ignore command/response data (not actual key codes)
    if ((data >= PS2_KC_BAT && data != PS2_KC_LANG1 && data != PS2_KC_LANG2)
        || (index & _WAIT_RESPONSE))
        return (uint16_t) data; // Return untranslated command/response data

    // Handle the "break" (key release) status
    if (index & _BREAK_KEY)
        PS2_keystatus |= _BREAK;
    else
        PS2_keystatus &= ~_BREAK;

    // Initialize return data as error code by default
    retdata = 0;

    // Scan the appropriate key table based on the flags (E0 mode for extended keys)
    if (index & _E0_MODE) {
        length = sizeof(extended_key) / sizeof(extended_key[0]); // Size of extended key table
        for (index = 0; index < length; index++) {
            #if defined( PS2_REQUIRES_PROGMEM )
                // If using PROGMEM, read the data from program memory
                if( data == pgm_read_byte( &extended_key[ index ][ 0 ] ) ) {
                    retdata = pgm_read_byte( &extended_key[ index ][ 1 ] );
                }
            #else
                if (data == extended_key[index][0]) {
                    retdata = extended_key[index][1]; // Return the matching key code
                }
            #endif
            if (retdata != 0) break; // Break when a match is found
        }
    } else {
        // For non-extended keys, use the single key table
        length = sizeof(single_key) / sizeof(single_key[0]);
        for (index = 0; index < length; index++) {
            #if defined( PS2_REQUIRES_PROGMEM )
                // If using PROGMEM, read the data from program memory
                if( data == pgm_read_byte( &single_key[ index ][ 0 ] ) ) {
                    retdata = pgm_read_byte( &single_key[ index ][ 1 ] );
                }
            #else
                if (data == single_key[index][0]) {
                    retdata = single_key[index][1]; // Return the matching key code
                }
            #endif
            if (retdata != 0) break; // Break when a match is found
        }
    }

    // If no matching key was found, set retdata to 0 (invalid key)
    if (index == length)
        retdata = 0;

    /* Handle the found key codes */
    if (retdata > 0) {
        // Special processing for lock keys (CAPS, NUM, SCROLL lock)
        if (retdata <= PS2_KEY_CAPS) {
            if (PS2_keystatus & _BREAK) {
                // If key is released (break), reset lock state
                PS2_lockstate[retdata] = 0;
                retdata = PS2_KEY_IGNORE; // Ignore the key press
            } else {
                // If the lock is active, ignore repeated make events
                if (PS2_lockstate[retdata] == 1)
                    retdata = PS2_KEY_IGNORE; // Ignore repeated press
                else {
                    // Set lock state for the first press
                    PS2_lockstate[retdata] = 1;
                    // Handle each lock key (CAPS, NUM, SCROLL)
                    switch (retdata) {
                        case PS2_KEY_CAPS:
                            index = PS2_LOCK_CAPS;
                            if (PS2_keystatus & _CAPS)
                                PS2_keystatus &= ~_CAPS; // Toggle CAPS lock
                            else
                                PS2_keystatus |= _CAPS;
                            break;
                        case PS2_KEY_SCROLL:
                            index = PS2_LOCK_SCROLL;
                            break;
                        case PS2_KEY_NUM:
                            index = PS2_LOCK_NUM;
                            break;
                    }
                    // Update LED lock status based on the lock state
                    if (PS2_led_lock & index) {
                        PS2_led_lock &= ~index;
                        PS2_keystatus |= _BREAK; // Send break for the key
                    } else
                        PS2_led_lock |= index; // Toggle lock LED
                    set_lock(); // Update the lock state
                }
            }
        }
        // Process modifier keys like SHIFT, CTRL, ALT, etc.
        else if (retdata >= PS2_KEY_L_SHIFT && retdata <= PS2_KEY_R_GUI) {
            #if defined( PS2_REQUIRES_PROGMEM )
                index = pgm_read_byte( &control_flags[ retdata - PS2_KEY_L_SHIFT ] );
            #else
                index = control_flags[retdata - PS2_KEY_L_SHIFT];
            #endif
            if (PS2_keystatus & _BREAK)
                PS2_keystatus &= ~index; // Clear status for key release
            else if ((PS2_keystatus & index) && (_mode & _NO_REPEATS))
                retdata = PS2_KEY_IGNORE; // Ignore repeated modifier keys
            else
                PS2_keystatus |= index; // Set the modifier key status
        }
        // Handle numeric keypad keys only if numlock is active or SHIFT is pressed
        else if (retdata >= PS2_KEY_KP0 && retdata <= PS2_KEY_KP_DOT)
            if (!(PS2_led_lock & PS2_LOCK_NUM) || (PS2_keystatus & _SHIFT)) {
                #if defined( PS2_REQUIRES_PROGMEM )
                    retdata = pgm_read_byte( &scroll_remap[ retdata - PS2_KEY_KP0 ] );
                #else
                    retdata = scroll_remap[retdata - PS2_KEY_KP0];
                #endif
            }

        // Handle break codes (key release)
        if ((PS2_keystatus & _BREAK) && (_mode & _NO_BREAKS))
            return (uint16_t) PS2_KEY_IGNORE; // Ignore break events if configured

        // Set function key mode if a function key is pressed
        if ((retdata <= PS2_KEY_SPACE || retdata >= PS2_KEY_F1)
            && retdata != PS2_KEY_EUROPE2)
            PS2_keystatus |= _FUNCTION; // Set function mode
        else
            PS2_keystatus &= ~_FUNCTION; // Clear function mode
    }

    // Return the processed key with its status in the upper byte
    return (retdata | ((uint16_t) PS2_keystatus << 8));
}

/**
 * @brief Sends the current lock status to the keyboard.
 *
 * This function builds and sends a command to the PS/2 keyboard to update the lock status
 * (e.g., Caps Lock, Num Lock, Scroll Lock). It sends the lock command followed by the
 * current lock status stored in `PS2_led_lock`.
 *
 * The function ensures that the lock status is correctly transmitted to the keyboard,
 * and if the system is idle, it starts the transmission process.
 */
static void set_lock() {
    send_byte(PS2_KC_LOCK);        // Send the lock command (0xED)
    send_byte(PS2_KEY_IGNORE);     // Wait for ACK (0xFA)
    send_byte(PS2_led_lock);       // Send the current lock status
    if (send_byte(PS2_KEY_IGNORE)) // Wait for ACK (0xFA)
        send_next();               // If idle, start transmission
}


/**
 * @brief Sends an echo command to the keyboard.
 *
 * This function sends the PS/2 ECHO command (0xEE) to the keyboard. The keyboard
 * should respond with the same ECHO code (0xEE), allowing verification that the
 * keyboard is correctly connected and communicating.
 *
 * The response from the keyboard is stored in the input buffer, where it can be
 * retrieved using functions that process incoming data.
 */
void PS2Keyboard_Echo(void) {
    send_byte(PS2_KC_ECHO);             // Send the ECHO command (0xEE)

    // Wait for a response; if the keyboard is idle, start transmission
    if ((send_byte(PS2_KEY_IGNORE)))
        send_next();
}


/**
 * @brief Sends the "Read ID" command to the keyboard.
 *
 * This function requests the keyboard identification by sending the "Read ID"
 * command (0xF2). The keyboard should respond with an acknowledgment followed by
 * a two-byte identifier, which can be retrieved from the keyboard buffer.
 *
 * The function handles waiting for the acknowledgment and response.
 */
void PS2Keyboard_ReadID(void) {
    send_byte(PS2_KC_READID);           // Send the "Read ID" command (0xF2)
    send_byte(PS2_KEY_IGNORE);          // Wait for ACK (0xFA)
    send_byte(PS2_KEY_IGNORE);          // Wait for the first byte of ID
    if ((send_byte(PS2_KEY_IGNORE)))    // Wait for the second byte of ID
        send_next();                    // If idle, start transmission
}


/**
 * @brief Requests the active scan code set from the keyboard.
 *
 * This function sends the "Get Scan Code Set" command (0xF0) followed by `0x00`
 * to request the currently active scan code set. The keyboard responds with an
 * acknowledgment (0xFA) followed by a single-byte scan code set identifier.
 *
 * There are three standard scan code sets:
 *  - Set 1: Used in older IBM systems.
 *  - Set 2: Default for most PS/2 keyboards.
 *  - Set 3: Less commonly used.
 *
 * The returned scan code set identifier is stored in the keyboard buffer.
 */
void PS2Keyboard_GetScanCodeSet(void) {
    send_byte(PS2_KC_SCANCODE);         // Send "Get/Set Scan Code Set" command (0xF0)
    send_byte(PS2_KEY_IGNORE);          // Wait for ACK (0xFA)
    send_byte(0);                       // Send data byte 0 (request current set)
    send_byte(PS2_KEY_IGNORE);          // Wait for ACK (0xFA)
    if ((send_byte(PS2_KEY_IGNORE)))    // Wait for scan code set identifier
        send_next();                    // If idle, start transmission
}


/**
 * @brief Retrieves the current status of the lock keys.
 *
 * Returns the current status of the lock keys (NUM, CAPS, SCROLL, and EXTRA).
 * Each bit in the returned byte corresponds to a specific lock key.
 *   - PS2_LOCK_SCROLL
 *   - PS2_LOCK_NUM
 *   - PS2_LOCK_CAPS
 *   - PS2_LOCK_EXTRA (rarely used)
 *
 * Use the macros (PS2_LOCK_NUM, PS2_LOCK_CAPS, etc.) defined in PS2Keyboard.h to
 * mask out the specific lock key you need.
 *
 * @return A byte where each bit represents the status of a lock key.
 */
uint8_t PS2Keyboard_GetLock() {
    return (PS2_led_lock);
}


/**
 * @brief Sets the current status of the lock keys (NUM, CAPS, SCROLL, and EXTRA)
 * and updates the keyboard LEDs accordingly.
 *
 * The provided `code` parameter is masked to 4 bits to ensure compatibility
 * with keyboards that might support an additional lock key.
 *
 * The function also updates `PS2_keystatus`, ensuring the internal state
 * reflects the Caps Lock status correctly.
 *
 * @param code A byte where each bit represents the desired lock state.
 *             See PS2Keyboard.h for the corresponding macros.
 */
void PS2Keyboard_SetLock(uint8_t code) {
    code &= 0xF;                // Mask to 4 bits (supporting extra LED if present)
    PS2_led_lock = code;        // Update the internal lock state
    PS2_keystatus &= ~_CAPS;    // Clear the CAPS lock flag
    PS2_keystatus |= (code & PS2_LOCK_CAPS) ? _CAPS : 0; // Set CAPS lock if needed
    set_lock();                 // Send the updated lock status to the keyboard
}


/**
 * @brief Configures whether the library should process break key codes (key releases).
 *
 * The keyboard always sends both make (press) and break (release) codes, but this setting
 * allows the library to ignore break codes if desired. When enabled, only make codes
 * will be processed, and break codes will be discarded.
 *
 * @param data 1 to ignore break key codes, 0 to process them.
 */
void PS2Keyboard_SetNoBreak(uint8_t data) {
    _mode &= ~_NO_BREAKS;       // Clear the NO_BREAKS flag
    _mode |= data ? _NO_BREAKS : 0;  // Set the flag if data is 1
}


/**
 * @brief Configures whether modifier keys (_CTRL, _ALT, _GUI, _SHIFT)
 *        should send repeated make codes.
 *
 * Some applications may want to suppress repeated make codes for modifier keys.
 * When enabled, the library will only send a single make code when a modifier
 * key is pressed and held.
 *
 * @param data 1 to disable repeated make codes, 0 to allow them.
 */
void PS2Keyboard_SetNoRepeat(uint8_t data) {
    _mode &= ~_NO_REPEATS;       // Clear the NO_REPEATS flag
    _mode |= data ? _NO_REPEATS : 0;  // Set the flag if data is 1
}


/**
 * @brief Resets the keyboard and clears the internal lock and key status.
 *
 * Sends a reset command to the keyboard. The keyboard will respond with:
 * - `0xAA` = PS2_KC_BAT 	(Self-test passed)
 * - `0xFC` = PS2_KC_ERROR 	(Self-test failed)
 *
 * The function waits for an acknowledgment (ACK) and then waits for the keyboard’s
 * response indicating the result of the reset.
 *
 * Additionally, this function resets the internal LED lock status and key status
 * to match the keyboard’s state after reset.
 */
void PS2Keyboard_ResetKey() {
    send_byte( PS2_KC_RESET );         	// Send reset command
    send_byte( PS2_KEY_IGNORE );       	// Wait for ACK
    if ( (send_byte( PS2_KEY_IGNORE)) ) // Wait for self-test result (0xAA or 0xFC)
        send_next();                  	// If idle, start transmission
    // Reset internal LED lock and key status
    PS2_led_lock = 0;
    PS2_keystatus = 0;
}


/**
 * @brief Sends a typematic rate and delay command to the keyboard.
 *
 * This function sends a typematic rate and delay command to the keyboard to control the
 * speed at which a key repeats when held down and the delay before the first repeat occurs.
 * The rate is specified as characters per second (CPS), and the delay is the time interval
 * before the first repeat starts.
 *
 * The function sends the following parameters:
 * - The rate (0 to 31), where 0 is 30 CPS and 31 is 2 CPS. The default is 0xB (10.9 CPS).
 * - The delay (0 to 3), specifying the delay before the first repeat (from 0.25s to 1s in 0.25s increments).
 *   The default is 1 (0.5s delay).
 *
 * The response from the keyboard is stored in the input buffer and can be read
 * using the appropriate buffer-reading functions.
 *
 * @param rate Typematic rate (0 to 31).
 * @param delay Typematic delay (0 to 3).
 *
 * @return int Error code (0 = OK, -5 = parameter error).
 *
 * @note Returns 0 on success, -5 on parameter error.
 */
int PS2Keyboard_Typematic(uint8_t rate, uint8_t delay) {
    if (rate > 31 || delay > 3)
        return -5;  // Return error if parameters are out of range

    send_byte(PS2_KC_RATE);               // Send the command to set typematic rate and delay
    send_byte(PS2_KEY_IGNORE);            // Wait for ACK (Acknowledgement)
    send_byte((delay << 5) + rate);       // Send the delay and rate values
    if ((send_byte(PS2_KEY_IGNORE)))      // Wait for ACK (Acknowledgement)
        send_next();                      // If idle, start the transmission

    return 0;  // Return success
}

/**
 * @brief Returns the count of available processed key codes.
 *
 * This function processes the input key code buffer and checks the output queue to determine
 * how many key codes are available in the buffer. If the processed key buffer is full, it will
 * return the maximum count. The function processes the input buffer until the buffer is empty or
 * the output buffer is full.
 *
 * @return uint8_t The count of available processed key codes in the buffer, or 0 if the buffer is empty.
 *
 * @note The function uses a ring buffer approach, where when the pointers match, the buffer is considered empty.
 */
uint8_t PS2Keyboard_Available() {
    int8_t i, idx;
    uint16_t data;

    // Check the output queue for available keys
    i = _key_head - _key_tail;
    if (i < 0)
        i += _KEY_BUFF_SIZE;

    // Process keys if the buffer is not full
    while (i < (_KEY_BUFF_SIZE - 1)) {
        if (key_available()) {  // Check if there are more keys to process
            data = translate();  // Get the next translated key
            if (data == 0)  // If the buffer is empty, exit the loop
                break;
            if ((data & 0xFF) != PS2_KEY_IGNORE && (data & 0xFF) > 0) {
                idx = _key_head + 1;  // Point to the next space in the buffer
                if (idx >= _KEY_BUFF_SIZE)  // Loop back to the start if necessary
                    idx = 0;
                _key_buffer[idx] = data;  // Save the data to the buffer
                _key_head = idx;  // Update the head pointer
                i++;  // Update the available count
            }
        } else {
            break;  // Exit if no more keys are coming in
        }
    }
    return (uint8_t)i;
}

/**
 * @brief Returns the key last read from the keyboard.
 *
 * This function checks if there is a key available in the buffer. If a key is
 * available, it retrieves the last key read and updates the buffer's read index.
 * If the buffer is empty, it returns 0.
 *
 * @return uint16_t The key code of the last read key, or 0 if the buffer is empty.
 */
uint16_t PS2Keyboard_Read()
{
    uint16_t result;
    uint8_t idx;

    // Check if a key is available in the buffer
    result = PS2Keyboard_Available();
    if (result)
    {
        idx = _key_tail;  // Get current read index
        idx++;
        if (idx >= _KEY_BUFF_SIZE)  // Wrap around if end of buffer is reached
            idx = 0;

        _key_tail = idx;  // Update buffer read index
        result = _key_buffer[idx];  // Retrieve the last key read
    }

    return result;
}


/**
 * @brief Enables the PS/2 interrupt after resetting the state machine.
 *
 * This function resets the internal PS/2 state variables and clears any pending
 * interrupts before enabling the external interrupt associated with the PS/2 clock line.
 */
void PS2Keyboard_EnableInt()
{
    ps2_reset();  // Reset internal PS/2 state

    // Clear any pending interrupts before enabling
    NVIC_ClearPendingIRQ(PIN_INT0_IRQn + 0);
    NVIC_EnableIRQ(PIN_INT0_IRQn + 0);
}


/**
 * @brief Disables the PS/2 interrupt.
 *
 * This function disables the external interrupt associated with the PS/2 clock line,
 * preventing further keyboard data reception. It can be used to temporarily ignore
 * PS/2 signals while processing data.
 */
void PS2Keyboard_DisableInt()
{
    NVIC_DisableIRQ(PIN_INT0_IRQn + 0);
}


// Initializes the PS/2 keyboard interface (see PS2Keyboard.h for details)
void PS2Keyboard_Init(gpioMap_t dataPin, gpioMap_t irqPin) {
    /* Reset internal PS/2 state variables */
    ps2_reset();

    /* Store pin configuration */
    PS2_DataPin = dataPin;
    PS2_IrqPin = irqPin;

    /* --- GPIO Configuration --- */
    // Configure Data and Clock pins as input with pull-up resistors
    gpioInit(PS2_DataPin, GPIO_INPUT_PULLUP);
    gpioInit(PS2_IrqPin, GPIO_INPUT_PULLUP);

    /* --- Interrupt Configuration --- */
    // Associate the selected GPIO pin with the interrupt channel
    Chip_SCU_GPIOIntPinSel( 0, gpioPinsInit[PS2_IrqPin].gpio.port,
                           gpioPinsInit[PS2_IrqPin].gpio.pin );

    // Clear any pending interrupts on the selected channel
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH(0) );

    // Set interrupt to trigger on edge events (instead of level-triggered)
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH(0) );

    // Enable interrupt on falling edge (PS/2 clock transitions from HIGH to LOW)
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH(0) );

    /* --- NVIC (Nested Vectored Interrupt Controller) Configuration --- */
    // Define interrupt priority level (lower values = higher priority)
    const uint8_t PS2_INTERRUPT_PRIORITY = 7;

    NVIC_SetPriority( PIN_INT0_IRQn + 0, PS2_INTERRUPT_PRIORITY );
    NVIC_ClearPendingIRQ( PIN_INT0_IRQn + 0 );
    NVIC_EnableIRQ( PIN_INT0_IRQn + 0 );
}
