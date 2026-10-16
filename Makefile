CC = clang
CFLAGS = -Wall -Wextra -Werror -g -Iinclude/
SRC = src/main.c
OUT = bin/microshell.exe
OUT_PROFILE = bin/microshell_profiled.exe

phony: clean all profile

profile: $(OUT_PROFILE)
$(OUT_PROFILE): $(SRC)
		@mkdir -p bin
		$(CC) $(CFLAGS) -I../microprof/include -DMICROSHELL_PROFILE -o $@ $^

all: $(OUT)
$(OUT): $(SRC)
		@mkdir -p bin
		$(CC) $(CFLAGS) -o $@ $^

clean: 
		@rm -rf bin