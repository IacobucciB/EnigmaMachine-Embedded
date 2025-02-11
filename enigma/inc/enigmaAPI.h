#ifndef __ENIGMA_H_
#define __ENIGMA_H_

void EnigmaAPI_Init(int rotor1 ,int rotor2 ,int rotor3, int reflector, int offset1, int offset2, int offset3);
char EnigmaAPI_EncryptChar(char character);
void EnigmaAPI_SetPlugboardMapping(const char* mapping);
unsigned int EnigmaAPI_GetRotorValue(unsigned int rotor);

#endif /* __ENIGMA_H_ */
