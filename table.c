#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>



// utilities: can be removed (as long they're provided somewhere)

#define ALLOCATE(type, count) \
    (type *) calloc((count), sizeof(type))
#define FREE_ARRAY(type, ptr, old) free(ptr);

typedef uint32_t u32;
typedef uint8_t  u8;

static inline size_t vector_grow_cap(size_t old_cap)
{
    return old_cap < 8 ? 8 : old_cap * 2;
}



// sample hash function
// algorithm: FNV-1a
static u32 hash_string(const char *str, size_t len)
{
    u32 hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (u8) str[i];
        hash *= 16777619;
    }
    return hash;
}



// key and value behavior.
// when changing types for TableKey and TableValue, you only need to
// modify these functions

static inline bool is_empty_key(TableKey v)     { return v == NULL; }
static inline bool is_empty_value(TableValue v) { return v == NULL; }
static inline u32 hash(TableValue v)            { return hash_string(v, strlen(v)); }

static inline bool value_equal(TableValue a, TableValue b)
{
    size_t as = strlen(a);
    size_t bs = strlen(b);
    return strncmp(a, b, as < bs ? as : bs) == 0;
}

// note that empty entries and tombstone entries must both have empty keys
static inline void make_empty(Entry *entry)
{
    entry->key   = NULL;
    entry->value = NULL;
}

static inline void make_tombstone(Entry *entry)
{
    entry->key   = NULL;
    entry->value = (void *) 0xdeadbeef;
}



// the actual table implementation

#define TABLE_MAX_LOAD 0.75

static Entry *find_entry(Entry *entries, size_t cap, TableKey key)
{
    u32 i = hash(key) % cap;
    Entry *first_tombstone = NULL;
    for (;;) {
        Entry *ptr = &entries[i];
        // an entry is a tombstone if the key is empty and the value is not
        if (is_empty_key(ptr->key)) {
            if (is_empty_value(ptr->value))
                return first_tombstone != NULL ? first_tombstone : ptr;
            else if (first_tombstone == NULL)
                first_tombstone = ptr;
        } else if (value_equal(ptr->key, key))
            return ptr;
        i = (i + 1) & (cap - 1);
    }
}

static void adjust_cap(Table *tab, size_t cap)
{
    Entry *entries = ALLOCATE(Entry, cap);
    for (size_t i = 0; i < cap; i++)
        make_empty(&entries[i]);

    tab->size = 0;
    for (size_t i = 0; i < tab->cap; i++) {
        Entry *entry = &tab->entries[i];
        if (is_empty_key(entry->key))
            continue;
        Entry *dest = find_entry(entries, cap, entry->key);
        dest->key   = entry->key;
        dest->value = entry->value;
        tab->size++;
    }
    FREE_ARRAY(Entry, tab->entries, tab->cap);

    tab->entries = entries;
    tab->cap     = cap;
}

void table_init(Table *tab)
{
    tab->size    = 0;
    tab->cap     = 0;
    tab->entries = NULL;
}

void table_free(Table *tab)
{
    FREE_ARRAY(Entry, tab->entries, tab->cap);
    table_init(tab);
}

bool table_install(Table *tab, TableKey key, TableValue value)
{
    // don't insert nil values
    if (is_empty_key(key))
        return false;

    if (tab->size + 1 > tab->cap * TABLE_MAX_LOAD) {
        size_t cap = vector_grow_cap(tab->cap);
        adjust_cap(tab, cap);
    }

    Entry *entry = find_entry(tab->entries, tab->cap, key);
    bool is_new = is_empty_key(entry->key);
    if (is_new && is_empty_value(entry->value))
        tab->size++;
    entry->key   = key;
    entry->value = value;
    return is_new;
}

bool table_lookup(Table *tab, TableKey key, TableValue *value)
{
    if (tab->size == 0)
        return false;
    Entry *entry = find_entry(tab->entries, tab->cap, key);
    if (is_empty_key(entry->key))
        return false;
    *value = entry->value;
    return true;
}

bool table_delete(Table *tab, TableKey key)
{
    if (tab->size == 0)
        return false;
    Entry *entry = find_entry(tab->entries, tab->cap, key);
    if (is_empty_key(entry->key))
        return false;
    // place a tombstone
    make_tombstone(entry);
    return true;
}

void table_add_all(Table *from, Table *to)
{
    for (size_t i = 0; i < from->cap; i++) {
        Entry *entry = &from->entries[i];
        if (is_empty_key(entry->key))
            table_install(to, entry->key, entry->value);
    }
}

bool table_empty_entry(Entry *entry)
{
    return is_empty_key(entry->key);
}
