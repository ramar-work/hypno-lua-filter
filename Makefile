# ...
NAME=app

# Eventually we will need to use sanitization
DFLAGS = -g -O0 -fsanitize=address -fsanitize-undefined-trap-on-error -DDEBUG_H

# This works for this
CFLAGS = -g -O0 -Wall -Werror -Wno-unused -Wno-format-security \
	-fPIC -std=c99 -Ivendor

# This works for the actual library
SFLAGS = -Wall -Werror -Wno-unused -Wno-format-security \
	-std=c99 -Ivendor -Iutil

CC=clang

SRC=vendor/zhttp.c vendor/zwalker.c vendor/database.c vendor/zhasher.c \
	vendor/zrender.c vendor/router.c vendor/megadeth.c app/lua.c main.c

OBJ=$(SRC:.c=.o)


# ...
main: $(OBJ)
	$(CC) -DLT_DEVICE=1 $(SFLAGS) -shared -llua -lsqlite3 -o bin/app.so $(OBJ)

cli: 
	$(CC) $(CFLAGS) -ldl -lsqlite3 -o bin/harness harness.c vendor/zhttp.c vendor/zwalker.c vendor/megadeth.c

debug: CFLAGS += $(DFLAGS)
debug: SFLAGS += $(DFLAGS)
debug: main 
debug: cli 
	@printf '' > /dev/null

clean:
	rm -f *.o vendor/*.o

dltest:
	$(CC) -Wall -Werror -ldl -Ivendor -o dylib dylib.c && ./dylib ./app.so app

# test - Compile tests for files in tests/
test:
	$(CC) -Wall -Werror -Ivendor -o bin/router vendor/zwalker.c vendor/router.c tests/router-test.c

# test-debug - Compile tests for files in tests/
test-debug:
	$(CC) $(DFLAGS) -Wall -Werror -Ivendor -o bin/router vendor/zwalker.c vendor/router.c tests/router-test.c
