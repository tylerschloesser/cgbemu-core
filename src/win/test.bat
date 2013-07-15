cd ..
gcc test.c cgbemu.c cpu.c debug.c gameboy.c graphics.c joypad.c memory.c screen.c tools.c cartridge.c -lfreeglut -lopengl32 -lglu32 -o win/test.exe
cd win
