
SOURCES=cgbemu.c cpu.c debug.c gameboy.c joypad.c memory.c screen.c cartridge.c test.c
OBJECTS=$(SOURCES:.c=.o)
CC=c99
CFLAGS= -c -O2 -g -Wall -pg
LFLAGS= -lglut -g -pg
EXECUTABLE=test

linux: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@
    
.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

