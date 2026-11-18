#ifndef CATALOG_H
#define CATALOG_H

#include <stddef.h>
#include <stdint.h>

typedef struct { char name[13]; uint32_t offset, length, kind; } cat_entry;
typedef struct { const uint8_t *data; size_t size; int count; int blocks; cat_entry entries[256]; } catalog;

int  cat_open(catalog *c, const uint8_t *data, size_t size);  /* parses the directory of a file already in memory; 0 ok */
const cat_entry *cat_find(const catalog *c, const char *name); /* case-insensitive; NULL if absent */
const uint8_t   *cat_data(const catalog *c, const cat_entry *e); /* pointer into data; NULL if out of range */

#endif
