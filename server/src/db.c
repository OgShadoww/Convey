#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

sqlite3 *db;

int open_db() {
  int rc = sqlite3_open("convey.db", &db);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "Error openning db: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  return 0;
}

int db_init_schema() {
  FILE *file = fopen("schema.sql", "r");
  if(!file) {
    perror("Error opening schema");
    return -1;
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char *schema = malloc(size+1);
  if(!schema) {
    fclose(file);
    perror("Malloc failled");
    return -1;
  }

  if(fread(schema, 1, size, file) != size) {
    return -1;
  }
  schema[size] = '\0';

  fclose(file);

  char *err = NULL;
  int rc = sqlite3_exec(db, schema, NULL, NULL, &err);
  if(rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err);
    sqlite3_free(err);
    return -1;
  }

  free(schema);
  return 0;
}

int close_db() {
  if(db) {
    sqlite3_close(db);
  }

  return 0;
}
