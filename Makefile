CC = clang
CFLAGS = -Wall -Wextra -Werror -g -Iinclude/
SRC = src/main.c

# Windows binaries keep the .exe extension, unix-like systems do not.
ifeq ($(OS),Windows_NT)
EXT = .exe
else
EXT =
endif

OUT = bin/microshell$(EXT)
OUT_PROFILE = bin/microshell_profiled$(EXT)

.PHONY: all clean profile

all: $(OUT)

profile: $(OUT_PROFILE)

$(OUT): $(SRC)
		@mkdir -p bin
		$(CC) $(CFLAGS) -o $@ $^

$(OUT_PROFILE): $(SRC)
		@mkdir -p bin
		$(CC) $(CFLAGS) -I../microprof/include -DMICROSHELL_PROFILE -o $@ $^

clean: 
		@rm -rf bin
