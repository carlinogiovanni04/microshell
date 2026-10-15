mkdir -p bin

clang -DBASE_IMPLEMENTATION -g -O0 -Iinclude -Wall -Wextra -Wpedantic -Werror src/main.c -o bin/microshell

x86_64-w64-mingw32-gcc -DBASE_IMPLEMENTATION -g -O0 -Iinclude -Wall -Wextra -Wpedantic -Werror src/main.c -o bin/microshell.exe
