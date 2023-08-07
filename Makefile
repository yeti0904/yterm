SRC   = $(wildcard source/*.c)
DEPS  = $(wildcard source/*.h)
OBJ   = $(addsuffix .o,$(subst source/,bin/,$(basename ${SRC})))
LIBS  = -lSDL2 -lSDL2_ttf
FLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -g -I./lib

compile: ./bin $(OBJ) $(SRC)
	$(CC) $(OBJ) $(LIBS) -o yterm

./bin:
	mkdir -p bin

bin/%.o: source/%.c
	$(CC) -c $< $(FLAGS) -o $@

clean:
	rm bin/*.o yterm

