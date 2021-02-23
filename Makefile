# ...
NAME=app

# Eventually we will need to use sanitization
DFLAGS = -g -O0 -fsanitize=address -fsanitize-undefined-trap-on-error -DDEBUG_H

# This works for this
CFLAGS = -g -O0 -Wall -Werror -Wno-unused -Wno-format-security \
	-fPIC -std=c99 -Ivendor -Iutil

# This works for the actual library
SFLAGS = -g -O0 -Wall -Werror -Wno-unused -Wno-format-security \
	-std=c99 -Ivendor -Iutil

CC=clang

SRC=vendor/zhttp.c vendor/zwalker.c vendor/database.c vendor/zhasher.c \
	vendor/zrender.c util/util.c app/home.c main.c

OBJ=$(SRC:.c=.o)


# ...
main: $(OBJ)
	$(CC) $(SFLAGS) -shared -o app.so $(OBJ)

debug: CFLAGS += $(DFLAGS)
debug: SFLAGS += $(DFLAGS)
debug: main
	@printf '' > /dev/null

clean:
	rm -f *.o vendor/*.o

dltest:
	$(CC) -Wall -Werror -ldl -Ivendor -o dylib dylib.c && ./dylib ./app.so app

