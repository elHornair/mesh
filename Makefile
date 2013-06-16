CC = gcc
CFLAGS = -I. -lpthread -g
SRCDIR = src
OBJ = $(SRCDIR)/main.o $(SRCDIR)/node.o $(SRCDIR)/worker.o $(SRCDIR)/util.o

mesh: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(SRCDIR)/*.o