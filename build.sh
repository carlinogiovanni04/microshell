#!/bin/sh

cd "$(dirname "$0")"
mkdir -p bin

# Optional instrumentation:  ./build.sh profile
profile_flags=""
for arg in "$@"; do
    if [ "$arg" = "profile" ]; then
        profile_flags="-DMICROSHELL_PROFILE -I../microprof/include"
    fi
done

clang -DBASE_IMPLEMENTATION -g -O0 -Iinclude $profile_flags -Wall -Wextra -Werror src/main.c -o bin/microshell

x86_64-w64-mingw32-gcc -DBASE_IMPLEMENTATION -g -O0 -Iinclude $profile_flags -Wall -Wextra -Werror src/main.c -o bin/microshell.exe
