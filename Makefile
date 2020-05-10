LDLIBS += `pkg-config --libs x11`
CFLAGS += -std=c99 -Wall -Wextra `pkg-config --cflags x11`

.PHONY: all clean

all: eduterm

eduterm: eduterm.c

clean:
	rm eduterm
