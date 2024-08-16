#ifndef HT_H_INCLUDED
#define HT_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef void *(*HtAllocator)(void *ptr, size_t old, size_t new);

enum {
    HT_ENTRY_VALID,
    HT_ENTRY_EMPTY,
    HT_ENTRY_TOMBSTONE,
};

static inline void *ht_default_allocator(void *ptr, size_t old, size_t new)
{
    (void) old;
    if (new == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, new);
}

#define HT_INIT() \
{ .size = 0, .cap = 0, .entries = NULL, .allocator = ht_default_allocator }

#define HT_INIT_WITH_ALLOCATOR(allocator) \
{ .size = 0, .cap = 0, .entries = NULL, .allocator = allocator }

#define HT_MAX_LOAD 0.75

#define HT_DECLARE(HashTable, HtKey, HtValue, header)                       \
                                                                            \
typedef struct HtEntry {                                                    \
    int tag;                                                                \
    HtKey key;                                                              \
    HtValue value;                                                          \
} HtEntry;                                                                  \
                                                                            \
typedef struct HashTable {                                                  \
    size_t size;                                                            \
    size_t cap;                                                             \
    HtEntry *entries;                                                       \
    HtAllocator allocator;                                                  \
} HashTable;                                                                \
                                                                            \
void header##_init(HashTable *tab);                                         \
void header##_init_with_allocator(HashTable *tab, HtAllocator allocator);   \
void header##_free(HashTable *tab);                                         \
                                                                            \
/* add (key, value) to tab and return whether it created (true) */          \
/* or modified (false) the key's value                          */          \
bool header##_install(HashTable *tab, HtKey key, HtValue value);            \
                                                                            \
/* lookup key in tab (putting it in *value) and return if key was found */  \
HtValue *header##_lookup(HashTable *tab, HtKey key);                        \
                                                                            \
/* delete key from tab and return if key was actually deleted */            \
bool header##_delete(HashTable *tab, HtKey key);                            \
                                                                            \
/* copy all entries from another HashTable */                               \
void header##_add_all(HashTable *from, HashTable *to);                      \

#define HT_DEFINE(HashTable, HtKey, HtValue, KEY_HASH, KEY_EQUAL, header) \
                                                                          \
void header##_init(HashTable *tab)                                        \
{                                                                         \
    tab->size    = 0;                                                     \
    tab->cap     = 0;                                                     \
    tab->entries = NULL;                                                  \
    tab->allocator = ht_default_allocator;                                \
}                                                                         \
                                                                          \
void header##_init_with_allocator(HashTable *tab, HtAllocator allocator)  \
{                                                                         \
    tab->size    = 0;                                                     \
    tab->cap     = 0;                                                     \
    tab->entries = NULL;                                                  \
    tab->allocator = allocator;                                           \
}                                                                         \
                                                                          \
void header##_free(HashTable *tab)                                        \
{                                                                         \
    tab->allocator(tab->entries, sizeof(HtEntry) * tab->cap, 0);          \
    header##_init(tab);                                                   \
}                                                                         \
                                                                          \
HtEntry *find_entry(HtEntry *entries, size_t cap, HtKey key)              \
{                                                                         \
    HtEntry *first_tombstone = NULL;                                      \
    for (uint32_t i = KEY_HASH(key) % cap; ; i = (i + 1) & (cap - 1)) {   \
        HtEntry *ptr = &entries[i];                                       \
        if (ptr->tag == HT_ENTRY_EMPTY) {                                 \
            return first_tombstone != NULL ? first_tombstone : ptr;       \
        } else if (ptr->tag == HT_ENTRY_TOMBSTONE                         \
                && first_tombstone == NULL) {                             \
            first_tombstone = ptr;                                        \
        } else if (KEY_EQUAL(ptr->key, key)) {                            \
            return ptr;                                                   \
        }                                                                 \
    }                                                                     \
}                                                                         \
                                                                          \
void adjust_cap(HashTable *tab, size_t cap)                               \
{                                                                         \
    HtEntry *entries = (HtEntry *) tab->allocator(                        \
        NULL, 0, sizeof(HtEntry) * cap                                    \
    );                                                                    \
    for (size_t i = 0; i < cap; i++) {                                    \
        entries[i].tag = HT_ENTRY_EMPTY;                                  \
    }                                                                     \
    /* copy over all values from old hashtable */                         \
    tab->size = 0;                                                        \
    for (size_t i = 0; i < tab->cap; i++) {                               \
        HtEntry *entry = &tab->entries[i];                                \
        if (entry->tag == HT_ENTRY_VALID) {                               \
            HtEntry *dest = find_entry(entries, cap, entry->key);         \
            dest->key   = entry->key;                                     \
            dest->value = entry->value;                                   \
            tab->size++;                                                  \
        }                                                                 \
    }                                                                     \
    /* free tab's array */                                                \
    tab->allocator(tab->entries, sizeof(HtEntry) * tab->cap, 0);          \
    tab->entries = entries;                                               \
    tab->cap     = cap;                                                   \
}                                                                         \
                                                                          \
bool header##_install(HashTable *tab, HtKey key, HtValue value)           \
{                                                                         \
    if (tab->size + 1 > tab->cap * HT_MAX_LOAD) {                         \
        adjust_cap(tab, tab->cap < 8 ? 8 : tab->cap * 2);                 \
    }                                                                     \
    HtEntry *entry = find_entry(tab->entries, tab->cap, key);             \
    if (entry->tag == HT_ENTRY_EMPTY) {                                   \
        tab->size++;                                                      \
    }                                                                     \
    entry->tag   = HT_ENTRY_VALID;                                        \
    entry->key   = key;                                                   \
    entry->value = value;                                                 \
    return entry->tag != HT_ENTRY_VALID;                                  \
}                                                                         \
                                                                          \
HtValue *header##_lookup(HashTable *tab, HtKey key)                       \
{                                                                         \
    if (tab->size == 0) {                                                 \
        return NULL;                                                      \
    }                                                                     \
    HtEntry *entry = find_entry(tab->entries, tab->cap, key);             \
    return entry->tag == HT_ENTRY_VALID ? &entry->value : NULL;           \
}                                                                         \
                                                                          \
bool header##_delete(HashTable *tab, HtKey key)                           \
{                                                                         \
    if (tab->size == 0) {                                                 \
        return false;                                                     \
    }                                                                     \
    HtEntry *entry = find_entry(tab->entries, tab->cap, key);             \
    if (entry->tag == HT_ENTRY_VALID) {                                   \
        entry->tag = HT_ENTRY_TOMBSTONE;                                  \
    }                                                                     \
    return entry->tag == HT_ENTRY_VALID;                                  \
}                                                                         \
                                                                          \
void header##_add_all(HashTable *from, HashTable *to)                     \
{                                                                         \
    for (size_t i = 0; i < from->cap; i++) {                              \
        HtEntry *entry = &from->entries[i];                               \
        if (entry->tag != HT_ENTRY_VALID) {                               \
            header##_install(to, entry->key, entry->value);               \
        }                                                                 \
    }                                                                     \
}                                                                         \

#define HT_FOR_EACH(tab, entry) \
    for (HtEntry *entry = (tab).entries; ((size_t) (entry - (tab).entries)) < (tab).cap; entry++)

#endif

