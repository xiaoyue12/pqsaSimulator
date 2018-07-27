CC=g++

CXXFLAGS += -std=gnu11
CFLAGS += $(WERROR_FLAGS) -Wno-error -Wno-unused-function
LDFLAGS += -L/usr/local/lib64
LDLIBS += -lhs -lstdc++ -lm
DEPS = scheduler.h linked-list.h include.h
OBJ = main.o


: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

main: $(OBJ)
	    $(CC) -o $@ $^ $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
		rm -f main *.o 
