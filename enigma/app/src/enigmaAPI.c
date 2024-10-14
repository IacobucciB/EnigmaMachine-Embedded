#include "board.h"

#define TICKRATE_HZ (1000)

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ROTATE 26

static volatile uint32_t tick_ct = 0; // Esto no va

const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *rotor_ciphers[] = {
    "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
    "AJDKSIRUXBLHWTMCQGZNPYFVOE",
    "BDFHJLCPRTXVZNYEIWGAKMUSQO",
    "ESOVPZJAYQUIRHXLNFTGKDCMWB",
    "VZBRGITYUPSDNHLXAWMJQOFECK",
    "JPGVOUMFYQBENHZRDKASXLICTW",
    "NZJHGRCXMYSWBOUFAIVLPEKQDT",
    "FKQHTLXOCBJSPDZRAMEWNIUYGV"
};

const char *rotor_notches[] = {"Q", "E", "V", "J", "Z", "ZM", "ZM", "ZM"};

const char *rotor_turnovers[] = {"R", "F", "W", "K", "A", "AN", "AN", "AN"};

const char *reflectors[] = {
    "EJMZALYXVBWFCRQUONTSPIKHGD",
    "YRUHQSLDPXNGOKMIEBFZCWVJAT",
    "FVPJIAOYEDRZXWGCTKUQSBNMHL"
};

struct Rotor {
    int             offset;
    int             turnnext;
    const char      *cipher;
    const char      *turnover;
    const char      *notch;
};

struct Enigma {
    int             numrotors;
    const char      *reflector;
    struct Rotor    rotors[8];
};

/*
 * Produce a rotor object
 * Setup the correct offset, cipher set and turn overs.
 */
struct Rotor new_rotor(struct Enigma *machine, int rotornumber, int offset) {
    struct Rotor r;
    r.offset = offset;
    r.turnnext = 0;
    r.cipher = rotor_ciphers[rotornumber - 1];
    r.turnover = rotor_turnovers[rotornumber - 1];
    r.notch = rotor_notches[rotornumber - 1];
    machine->numrotors++;

    return r;
}

/*
 * Return the index position of a character inside a string
 * if not found then -1
 **/
int str_index(const char *str, int character) {
    char *pos;
    int index;
    pos = strchr(str, character);

    // pointer arithmetic
    if (pos){
        index = (int) (pos - str);
    } else {
        index = -1;
    }

    return index;
}

/*
 * Cycle a rotor's offset but keep it in the array.
 */
void rotor_cycle(struct Rotor *rotor) {
    rotor->offset++;
    rotor->offset = rotor->offset % ROTATE;

    // Check if the notch is active, if so trigger the turnnext
    if(str_index(rotor->turnover, alpha[rotor->offset]) >= 0) {
        rotor->turnnext = 1;
    }
}

/*
 * Pass through a rotor, right to left, cipher to alpha.
 * returns the exit index location.
 */
int rotor_forward(struct Rotor *rotor, int index) {

    // In the cipher side, out the alpha side
    index = (index + rotor->offset) % ROTATE;
    index = str_index(alpha, rotor->cipher[index]);
    index = (ROTATE + index - rotor->offset) % ROTATE;

    return index;
}

/*
 * Pass through a rotor, left to right, alpha to cipher.
 * returns the exit index location.
 */
int rotor_reverse(struct Rotor *rotor, int index) {

    // In the cipher side, out the alpha side
    index = (index + rotor->offset) % ROTATE;
    index = str_index(rotor->cipher, alpha[index]);
    index = (ROTATE + index - rotor->offset) % ROTATE;

    return index;

}

/*
 * Run the enigma machine
 **/

void SysTick_Handler(void) {
   tick_ct++;
}

void delay(uint32_t tk) {
   uint32_t end = tick_ct + tk;
   while(tick_ct < end)
       __WFI();
}

char output[] = "HELLO WORLD"; 

int main(void) {
   SystemCoreClockUpdate();
   Board_Init();
   SysTick_Config(SystemCoreClock / TICKRATE_HZ);
   
   struct Enigma machine = {}; // inicializado con valores por defecto
   int i, character, index;

    char input[] = "HELLO WORLD";  // Cadena de prueba
    int length = strlen(input);    // Longitud de la cadena

    // Configs
    int rotor1 = 3;  // Rotor III
    int rotor2 = 2;  // Rotor II
    int rotor3 = 1;  // Rotor I
    int offset1 = 0; // Offset del Rotor 1
    int offset2 = 0; // Offset del Rotor 2
    int offset3 = 0; // Offset del Rotor 3

    // Configurar Enigma
    machine.reflector = reflectors[1]; // Usamos el reflector B
    machine.rotors[0] = new_rotor(&machine, rotor1, offset1);
    machine.rotors[1] = new_rotor(&machine, rotor2, offset2);
    machine.rotors[2] = new_rotor(&machine, rotor3, offset3);

    // Proceso de cifrado utilizando un for loop
    for (int pos = 0; pos < length; pos++) {
        character = input[pos];

        if (!isalpha(character)) {
            printf("%c", character);
            continue;
        }

        character = toupper(character);
        index = str_index(alpha, character);

        // Ciclar el primer rotor antes de continuar
        rotor_cycle(&machine.rotors[0]);

        // Doble paso del rotor
        if (str_index(machine.rotors[1].notch, alpha[machine.rotors[1].offset]) >= 0) {
            rotor_cycle(&machine.rotors[1]);
        }

        // Ciclar los rotores
        for (i = 0; i < machine.numrotors - 1; i++) {
            if (machine.rotors[i].turnnext) {
                machine.rotors[i].turnnext = 0;
                rotor_cycle(&machine.rotors[i + 1]);
            }
        }

        // Pasar a traves de los rotores (hacia adelante)
        for (i = 0; i < machine.numrotors; i++) {
            index = rotor_forward(&machine.rotors[i], index);
        }

        // Pasar por el reflector
        character = machine.reflector[index];
        index = str_index(alpha, character);

        // Pasar de vuelta a traves de los rotores (en reversa)
        for (i = machine.numrotors - 1; i >= 0; i--) {
            index = rotor_reverse(&machine.rotors[i], index);
        }

        // Salida del caracter cifrado
        printf("%c", alpha[index]);
        output[pos] = alpha[index];
    }

    printf("\r\n");  // Salto de linea al final

   while (1) {
      Board_LED_Toggle(LED_3);
      delay(100);
      printf("Salida cifrada es: ");
      for(int random = 0; random < 11; random++)
      {
         printf("%c-", output[random]);
      }
      printf("\r\n");
   }
}
