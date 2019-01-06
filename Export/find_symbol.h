#include <stdbool.h>

#include "sym_file.h"


// should be called by open_sym_file
// initilaizes a static variable in find_symbol.c
void fs_init(struct sym_file*);

// returns the string literal of the header found from the literal token
// returning NULL when none was found
const char* find_header(char*);
