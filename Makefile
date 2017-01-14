CC=gcc

CFLAGS=-Wall -Wextra -Ofast
LFLAGS=-s

OBJS=main.o
DEPS=
LIBS=

BIN=mac2file

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)
