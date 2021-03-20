#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <zhttp.h>
#include <ztable.h>
#include <zrender.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <router.h>
#include <megadeth.h>

#ifndef LLUA_H
#define LLUA_H

enum zlua_error {
	ZLUA_NO_ERROR,
	ZLUA_MISSING_ARGS,
	ZLUA_INCORRECT_ARGS
};

int lc ( struct HTTPBody *, struct HTTPBody * );
#endif
