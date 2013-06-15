CC = gcc
CFLAGS = -I. -lpthread
DEPS = include/node.h
SRCDIR = src
OBJ = $(SRCDIR)/main.o $(SRCDIR)/node.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mesh: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(SRCDIR)/*.o