# Fichiers
EXE = main
TEST_EXE = test_main
SRC = $(filter-out src/test.c, $(wildcard src/*.c))
TEST_SRC = $(filter-out src/main.c, $(wildcard src/*.c))

#OBJ = $(SRC:.c=.o)
#TEST_OBJ = $(TEST_SRC:.c=.o)

# Compilation
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -pedantic -I/usr/include/SDL2
LDFLAGS = -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Debug
CFLAGS += -g
CFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address

all: $(EXE) $(TEST_EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)

$(TEST_EXE): $(TEST_SRC)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_EXE) $(LDFLAGS)

#%.o: %.c %.h
#	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXE)
	./$(EXE)

test: $(TEST_EXE)
	./$(TEST_EXE)

clean:
	rm -f $(EXE) $(TEST_EXE) 
	rm -f src/*.o
#						... src/*.o
