//books.h
#include <zhttp.h>
#include <ztable.h>
#include <zrender.h>
#include <database.h>
#include <megadeth.h>

#ifndef BOOKS_H
#define BOOKS_H
int books( struct HTTPBody *req, struct HTTPBody *res, zTable ** );
#endif
