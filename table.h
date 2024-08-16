#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

// These typedefs are provided as examples. Redefine them to whatever you want.
typedef char * TableKey;
typedef char * TableValue;

typedef struct {
    TableValue key;
    TableValue value;
} Entry;

typedef struct Table {
    size_t size;
    size_t cap;
    Entry *entries;
} Table;

/*
 * install() returns whether it created (true) or modified (false) the key's value
 * lookup() returns whether the lookup went well
 * delete() returns whether the delete went well
 * add_all() copies elements from another table
 */

void table_init(Table *tab);
void table_free(Table *tab);
bool table_install(Table *tab, TableKey key, TableValue value);
bool table_lookup(Table *tab, TableKey key, TableValue *value);
bool table_delete(Table *tab, TableKey key);
void table_add_all(Table *from, Table *to);
bool table_empty_entry(Entry *entry);

#define TABLE_FOR_EACH(tab, entry) \
    for (Entry *entry = tab->entries; ((size_t) (entry - tab->entries)) < tab->cap; entry++)

#endif
