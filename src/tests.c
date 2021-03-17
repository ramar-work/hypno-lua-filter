//tests.c - Should simply check input and output...

#define ZLUA_NO_ERROR 0
#define ZLUA_MISSING_ARGS 0
#define ZLUA_INCORRECT_ARGS 0

struct test_t {
	//A string to test
	const char *string;
	//A code (or number of args) to expect back
	int expected;
	//Using real error codes from argument handler could make a lot of sense
	int error;
} tests[] = {
	{ "test.number( 0 )", 1, ZLUA_NO_ERROR },
	{ "test.number( 'abc' )", 0, ZLUA_NO_ERROR },
	{ "test.number( )", 0, ZLUA_NO_ERROR },
	{ NULL }
};
