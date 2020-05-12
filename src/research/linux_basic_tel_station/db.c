#include "db.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static db_value *arr;
static size_t size;


void db_init(size_t count) {
    arr = calloc(sizeof(arr[0]), count);
    size = count;
}

void db_deinit() {
    free(arr);
}

db_value *db_get(size_t id) {
    if (id >= size) {
        return 0;
    }
    return &arr[id];
}

void db_set(size_t *id, db_value val) {
    assert(*id < size);
    arr[*id] = val;
}

size_t db_find(char *key) {
    for (int i = 0; i < size; i++) {
        if (strcmp(arr[i].key, key) == 0) {
            return i;
        }
    }
    return size;
}
