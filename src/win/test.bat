cd ..
gcc test/test.c android_interface.c cpu.c debug.c gameboy.c graphics.c joypad.c memory.c screen.c tools.c -lfreeglut -lopengl32 -lglu32 -o test/test.exe
cd test