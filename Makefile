CC=g++
CFLGAS = -I.
DEPS = scheduler.h linked-list.h include.h 
OBJ = main.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)

clean:
		rm -f main *.o 
