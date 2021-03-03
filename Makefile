# ...
NAME=app
DFLAGS = -g -O0 -fsanitize=address -fsanitize-undefined-trap-on-error -DDEBUG_H
CFLAGS = -Wall -Werror -Wno-unused -Wno-format-security -fPIC -std=c99 -Ivendor
SFLAGS = -Wall -Werror -Wno-unused -Wno-format-security -std=c99 -Ivendor
CC=clang
SRC=vendor/zhttp.c vendor/zmime.c vendor/zwalker.c vendor/database.c \
	vendor/ztable.c vendor/zrender.c vendor/router.c vendor/megadeth.c
OBJ=$(SRC:.c=.o)
TARGET=


# ...
c: SRC += app/home.c app/form.c app/books.c app/c.c 
c: $(OBJ)
	$(CC) $(CFLAGS) -c -o app/form.o app/form.c
	$(CC) $(CFLAGS) -c -o app/home.o app/home.c
	$(CC) $(CFLAGS) -c -o app/books.o app/books.c
	$(CC) $(CFLAGS) -c -o app/c.o app/c.c
	$(CC) -DLT_DEVICE=1 $(SFLAGS) -shared -lsqlite3 -o bin/app.so $(OBJ)


lua: SRC += app/lua.c
lua: $(OBJ)
	$(CC) $(CFLAGS) -c -o app/lua.o app/lua.c
	$(CC) -DLT_DEVICE=1 $(SFLAGS) -shared -llua -lsqlite3 -o bin/app.so $(OBJ)


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
