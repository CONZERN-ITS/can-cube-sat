#include "parse.h"
#include <string.h>
#include <ctype.h>



void _preparse(char *in) {
    char *cur = in;
    char *cpy = in;
    while (*cur++) {
        *cpy = *cur;
        if (!isspace(*cur)) {
            cpy++;
        }
    }
}

void pass_spaces(char **in) {
    while (**in && isspace(**in)) {
        ++(*in);
    }
}
void pass_spaces_or_unil_end(char **in, char end) {
    while (**in && isspace(**cur) && **cur != end) {
        ++(*in);
    }
}

int _read_until_end(char **in, char **wbegin, char **wend, char end) {
    pass_spaces(in);
    *wbegin = *in;
    
    char *cur = *in;
    char *last_nsp = *in;
    while (*cur && *cur != end) {
        if (!isspace(*cur)) {
            last_nsp = cur;
        }
        ++cur;
    }
    wend = last_nsp + 1;
    if (!*cur) {
        return 1;
    }
    return 0;    
}

int parse(char *in, char **key_b, char **key_e, char **value_b, char **value_e, char div) {
    char *cur;
    cur = in;
    
    if (_read_until_end(&cur, key_b, key_e, div)) {
        return 1;
    }
    cur++;
    _read_until_end(&cur, value, value_e, 0);
}
