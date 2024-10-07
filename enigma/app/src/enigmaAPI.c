/* Copyright 2013 Cory Lutton                                               */
/*                                                                          */
/* Licensed under the Apache License, Version 2.0 (the "License");          */
/* you may not use this file except in compliance with the License.         */
/* You may obtain a copy of the License at                                  */
/*                                                                          */
/*    http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                          */
/* Unless required by applicable law or agreed to in writing, software      */
/* distributed under the License is distributed on an "AS IS" BASIS,        */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied  */
/* See the License for the specific language governing permissions and      */
/* limitations under the License.                                           */
/*
Setting Wiring                      Notch   Window  Turnover
Base    ABCDEFGHIJKLMNOPQRSTUVWXYZ
I       EKMFLGDQVZNTOWYHXUSPAIBRCJ  Y       Q       R
II      AJDKSIRUXBLHWTMCQGZNPYFVOE  M       E       F
III     BDFHJLCPRTXVZNYEIWGAKMUSQO  D       V       W
IV      ESOVPZJAYQUIRHXLNFTGKDCMWB  R       J       K
V       VZBRGITYUPSDNHLXAWMJQOFECK  H       Z       A
VI      JPGVOUMFYQBENHZRDKASXLICTW  H/U     Z/M     A/N
VII     NZJHGRCXMYSWBOUFAIVLPEKQDT  H/U     Z/M     A/N
VIII    FKQHTLXOCBJSPDZRAMEWNIUYGV  H/U     Z/M     A/N

Setting     Wiring
Base        ABCDEFGHIJKLMNOPQRSTUVWXYZ
A           EJMZALYXVBWFCRQUONTSPIKHGD
B           YRUHQSLDPXNGOKMIEBFZCWVJAT
C           FVPJIAOYEDRZXWGCTKUQSBNMHL
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ROTATE 26

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
int main() {
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
            putchar(character);
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
        putchar(alpha[index]);
    }

    putchar('\n');  // Salto de linea al final
    return 0;
}
