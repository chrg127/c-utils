#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hash.h"
#include "ht.h"

bool string_equal(char *s, char *t)
{
    return strcmp(s, t) == 0;
}

HT_DECLARE(Table, char *, char *, table)

HT_DEFINE(Table, char *, char *, fnv_1a_32_cstring, string_equal, table)

int main()
{
    Table tab = HT_INIT();
    char *key = "aaa";
    char *value = "bbb";
    table_install(&tab, key, value);
    char **test;
    if (test = table_lookup(&tab, key), test != NULL) {
        printf("%s\n", *test);
    }
    table_free(&tab);
}
