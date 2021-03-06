# ...
NAME = app
DFLAGS = -g -O0 -DDEBUG_H -fsanitize=address -fsanitize-undefined-trap-on-error 
CFLAGS = -Wall -Werror -Wno-unused -Wno-format-security -fPIC -std=c99 -Ivendor
SFLAGS = -Wall -Werror -Wno-unused -Wno-format-security -std=c99 -Ivendor
CC = clang
SRC = vendor/zhttp.c vendor/zmime.c vendor/zwalker.c vendor/database.c \
	vendor/ztable.c vendor/zrender.c vendor/router.c vendor/megadeth.c
OBJ = $(SRC:.c=.o)
TARGET=


# ...
lua: SRC += lua.c
lua: $(OBJ)
	$(CC) $(CFLAGS) -c -o lua.o lua.c
	$(CC) -DLT_DEVICE=1 $(SFLAGS) -shared -llua -lsqlite3 -o bin/app.so $(OBJ)


lmain: SRC += lua.c
lmain: CFLAGS += -Werror -Wno-unused -Wno-format-security -std=c99 -Ivendor -DRUN_MAIN $(DFLAGS)
lmain: $(OBJ)
lmain: clean
	$(CC) $(CFLAGS) -c -o lua.o lua.c
	$(CC) $(CFLAGS) -llua -lsqlite3 -o bin/lmain $(OBJ)


cli: 
	$(CC) $(CFLAGS) -ldl -lsqlite3 -o bin/harness harness.c vendor/zhttp.c vendor/zwalker.c vendor/megadeth.c


debug: CFLAGS += $(DFLAGS)
debug: SFLAGS += $(DFLAGS)
debug: $(TARGET) 
	@printf '' > /dev/null


clean:
	rm -f *.o app/*.o vendor/*.o

dltest:
	$(CC) -Wall -Werror -ldl -Ivendor -o dylib dylib.c && ./dylib ./app.so app

# test - Compile tests for files in tests/
test:
	$(CC) -Wall -Werror -Ivendor -o bin/router vendor/zwalker.c vendor/router.c tests/router-test.c

# test-debug - Compile tests for files in tests/
test-debug:
	$(CC) $(DFLAGS) -Wall -Werror -Ivendor -o bin/router vendor/zwalker.c vendor/router.c tests/router-test.c
