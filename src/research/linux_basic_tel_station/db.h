
typedef struct db_value {
    char *key;
    char *str
} db_value;



void db_init(size_t count);
void db_deinit();
db_value db_get(size_t id);
void db_set(size_t *id, db_value val);
ssize_t db_find(char *key);
