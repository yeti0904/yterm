SRC   = $(wildcard source/*.c)
DEPS  = $(wildcard source/*.h)
OBJ   = $(addsuffix .o,$(subst source/,bin/,$(basename ${SRC})))
LIBS  = -lX11 -lSDL2 -lSDL2_ttf
FLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -g

compile: ./bin $(OBJ) $(SRC)
	$(CC) $(OBJ) $(LIBS) -o yterm

./bin:
	mkdir -p bin

bin/%.o: source/%.c
	$(CC) -c $< $(FLAGS) -o $@

clean:
	rm bin/*.o yterm

