
SOURCES=android_interface.c cpu.c debug.c gameboy.c graphics.c joypad.c memory.c screen.c tools.c test.c
OBJECTS=$(SOURCES:.c=.o)
CC=gcc
CFLAGS= -c
LFLAGS= -lglut
EXECUTABLE=test

linux: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@
    
.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

