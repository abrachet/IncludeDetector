/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * "Exporting" is the process by which known symbols found in header files are
 * exported into the serializable symfile for use later
 * 
 * this file deals with hashing literal tokens ie strings and making their way into the
 * globaly opened symfile
 * 
 */ 

#include "export.h"
#include "sym_file.h"

extern struct sym_file* sym_file;

static struct se_list symbol_list = {0};


////////////testing/////////////////
void export(const char* literal_token, export_t et)
{
    hash_t hashed = hash(literal_token);
    se_list_add(&symbol_list, (struct symbol_entry){ .hash = hash(literal_token), .header_name = et} );
}

void export_end(export_t et)
{
    merge(sym_file, symbol_list);
}
////////////////////////////////

hash_t 
murmur64(hash_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53L;
    h ^= h >> 33;
    return h;
}

hash_t  
hash(char* str)
{
    hash_t hash = 0xA71B0403F93;

    for (; *str; str++)
    {
        hash <<= *str;
        hash += murmur64(hash);
    }

    return hash;
}
