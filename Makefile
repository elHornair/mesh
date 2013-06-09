CC = gcc
CFLAGS = -I.
DEPS = include/server.h
SRCDIR = src
OBJ = $(SRCDIR)/main.o $(SRCDIR)/server.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mesh: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(SRCDIR)/*.o