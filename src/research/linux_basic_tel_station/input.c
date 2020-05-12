#include "input.h"
#include "parse.h"
#include "db.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 250 //including null-terminator

int stream_read(char div) {
    int n = 0;
    char *line = 0;
    ssize_t size = getline(&line, &n, stdio) + 1;
    if (size > MAX_LINE_LENGTH) {
        return 1;
    }
    
    char key[MAX_LINE_LENGTH];
    
    char *key_b, *key_w, *value_b, *value_e;
    int rc = parse(line, &key_b, &key_w, &value_b, &value_e, div);
    if (rc) {
        return 1;
    }
    
    *key_w = 0;
    int id = db_find(key_b);
    db_value *tt = db_get(id);
    if (tt == 0) {
        return 2;
    }
    
    strcmp(tt->value, value_b);
    free(line);
}
