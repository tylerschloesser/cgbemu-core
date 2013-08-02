cd ..
gcc -std=c99 -m64 test.c cgbemu.c cpu.c debug.c gameboy.c joypad.c memory.c screen.c cartridge.c -lfreeglut -lopengl32 -lglu32 -o win/test.exe
cd win
