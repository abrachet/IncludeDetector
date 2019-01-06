/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * Responsbile for extracting symbols from the symbol table
 * 
 * note that this doesn't require locking or unlocking the mutex
 * because symbols will only ever be searched for after all writes made to the symfile have
 * already been made
 * this makes NO WRITES only reads to the sym_file
 * 
 * find header attempts to return the header associated with a literal token
 * 
 */ 

#include <ID.h>
#include "find_symbol.h"
#include "sym_file.h"
#include "export.h"

// this is a tough design decision, doesn't seem like a good idea
// but the alternatives aren't great either.
// ultimately, I went with this because after all headers are scanned and the 
// final version of the sym_file is loaded then fs_init can be called
// it's not perfect but it seems like the best way to do this
// moreover, only after the program finishes searching for all symbols will the sym_file
// be commited back to the file system and munmapping this data. 
// In theory this doesn't cause any catastrophic problems. But, famous last words I suppose...
extern struct sym_file* sym_file;

// binary search in the symbol table for the specified string
static inline struct symbol_entry* 
find_symbol(hash_t hashed)
{
    ssize_t left = 0;
    ssize_t right = sym_file->symbols.length - 1;
    struct symbol_entry* arr = sym_file->symbols.arr;
    size_t mid = 0;

    while (left <= right) {
        mid = (int)((right + left) / 2);

        if (arr[mid].hash == hashed)
            return arr + mid;
        
        if (arr[mid].hash < hashed)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return NULL;
}

const char*
find_header(char* str)
{
    hash_t hashed = hash(str);
    struct symbol_entry* se = find_symbol(hashed);

    if (!se)
        return NULL;

    return ht_index(sym_file->headers, se->header_name);
}
