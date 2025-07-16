#ifndef BIOS_H
#define BIOS_H

#include <stdint.h>

unsigned char bios_constat(void);
unsigned char bios_conin(void);
void bios_conout(int);

#endif /* BIOS_H */

