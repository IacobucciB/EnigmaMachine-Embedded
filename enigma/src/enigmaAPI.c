#include "sapi.h"
#include "enigmaAPI.h"

#include <ctype.h>
#include <stdlib.h>
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

struct Rotor new_rotor(struct Enigma *machine, int rotornumber, int offset) {
    struct Rotor r;
    r.offset = offset;
    r.turnnext = 0;
    r.cipher = rotor_ciphers[rotornumber - 1];
    r.turnover = rotor_turnovers[rotornumber - 1];
    r.notch = rotor_notches[rotornumber - 1];

    return r;
}

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

void rotor_cycle(struct Rotor *rotor) {
    rotor->offset++;
    rotor->offset = rotor->offset % ROTATE;

    // Check if the notch is active, if so trigger the turnnext
    if(str_index(rotor->turnover, alpha[rotor->offset]) >= 0) {
        rotor->turnnext = 1;
    }
}

int rotor_forward(struct Rotor *rotor, int index) {

    // In the cipher side, out the alpha side
    index = (index + rotor->offset) % ROTATE;
    index = str_index(alpha, rotor->cipher[index]);
    index = (ROTATE + index - rotor->offset) % ROTATE;

    return index;
}

int rotor_reverse(struct Rotor *rotor, int index) {

    // In the cipher side, out the alpha side
    index = (index + rotor->offset) % ROTATE;
    index = str_index(rotor->cipher, alpha[index]);
    index = (ROTATE + index - rotor->offset) % ROTATE;

    return index;

}

static struct Enigma machine = {};

void enigma_init(int rotor1 ,int rotor2 ,int rotor3, int offset1, int offset2, int offset3)
{
    // Configurar Enigma
	machine.numrotors = 3;
    machine.reflector = reflectors[1]; // Usamos el reflector B
    machine.rotors[0] = new_rotor(&machine, rotor1, offset1);
    machine.rotors[1] = new_rotor(&machine, rotor2, offset2);
    machine.rotors[2] = new_rotor(&machine, rotor3, offset3);

}

//PENDING: manejar error por ingreso de un caracter invalido sin usar printf
char encrypt_char(char character)
{
    int i, index;

    if (!isalpha(character)) {
        printf("%c", character);
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
    return alpha[index];
}
