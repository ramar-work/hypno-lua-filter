//form.h
#include <zhttp.h>
#include <ztable.h>
#include <zrender.h>
#include <database.h>
#include <megadeth.h>

#ifndef FORM_H
#define FORM_H
int form( struct HTTPBody *req, struct HTTPBody *res, zTable ** );
#endif
