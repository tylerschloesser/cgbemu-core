
#include "globals.h"

#include "memory.h"
#include <string.h>
static int get_cartridge_ram_size(int header_value);
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

    /* Cartridge RAM */
    cartridge_ram_size = get_cartridge_ram_size(cartridge_rom[0x0147]);
    //TODO handle errors
    assert(cartridge_ram_size > 0);
    assert(cartridge_ram == NULL);
    cartridge_ram = (uint8_t*)malloc(cartridge_ram_size);
    assert(cartridge_ram != NULL); 
    

    printf("cartridge loaded \n\tRAM: %i KiB\n\tROM: %i KiB\n", 
            cartridge_ram_size / 1024, size / 1024);

    //TODO actually verify...
    verify_cartridge();

    return 0;
}

static int get_cartridge_ram_size(int header_value) {
    switch(header_value) {
        case 0:
            return 0; // no cartridge ram
        case 1:
            return 0x800; // 2KiB
        case 2:
            return 0x2000; // 8KiB
        case 3:
            return 0x8000; // 32KiB
    }
    return -1;
}

static int get_cartridge_rom_size(int header_value) {
    if(header_value >= 0 && header_value <= 7) {
        return(0x8000 << header_value);
    }
    return -1;
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

    int expected_cartridge_rom_size =
       get_cartridge_rom_size(cartridge_rom[0x148]);

	printf("expected ROM size: %i KiB\n", 
            expected_cartridge_rom_size / 1024);
	
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


