
#include "globals.h"

#include "memory.h"
#include <string.h>
/* The contents of "buffer" are not needed after
   this function is called
   TODO
   check for max size
 */

int load_cartridge(const uint8_t* buffer, int size) {
    assert(buffer != NULL);
    assert(size > 0);
    
    //TODO make this better
    assert(cartridge_rom == NULL);

    cartridge_rom = (uint8_t*)malloc(size);
    
    //TODO better null check
    assert(cartridge_rom != NULL);
    memcpy(cartridge_rom, buffer, size);

    cartridge_rom_size = size;

    printf("cartridge loaded: %i KiB\n", size / 1024);

    //TODO actually verify...
    verify_cartridge();

    return 0;
}


/* TODO finish this */
int verify_cartridge() {
	
	char title[0x10];
	title[0x10 - 1] = '\0';
	int i;
	for(i = 0x134; i < 0x144; ++i) {
		title[i - 0x134] = cartridge_rom[i];
	}
	printf("Title: %s\n", title);
	printf("CartridgeType: %X\n", cartridge_rom[0x147]);
	printf("Rom Size: %X\n", cartridge_rom[0x148]);
	
	// calculate checksum
	u16 sum;
	for(i = 0; i < cartridge_rom_size; ++i) {
		sum += cartridge_rom[i];
	}
	sum -= (cartridge_rom[0x14e] + cartridge_rom[0x14f]);
	
	printf("Sum: %X\n", sum);
	
	printf("checksum: %X %X\n", cartridge_rom[0x14e], cartridge_rom[0x14f]);
	
	return 0;
}


