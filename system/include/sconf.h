#ifndef SIMPLE_CONFIG_H
#define SIMPLE_CONFIG_H

#include <tstr.h>

#define S_CONF_ITEM_MAX 32

typedef struct {
	tstr_t* name;
	tstr_t* value;
} sconf_item_t;

typedef struct {
	sconf_item_t items[S_CONF_ITEM_MAX];
} sconf_t;

sconf_t* sconf_parse(const char* str);
void sconf_free(sconf_t* conf);
const char* sconf_get(sconf_t *conf, const char*name);
sconf_t* sconf_load(const char* fname);

#endif