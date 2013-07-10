#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "globals.h"

int load_cartridge(const uint8_t* buffer, int size);
int verify_cartridge();

#endif /* CARTRIDGE_H */
