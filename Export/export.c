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
#include <pthread/pthread.h>
#include <stdlib.h>
#include <string.h>

extern struct sym_file* sym_file;

/*
pthread_mutex_t symbol_list_mtx = PTHREAD_MUTEX_INITIALIZER;
static struct se_list symbol_list = {0};
*/

////////////testing/////////////////
void export(const char* literal_token, export_t et)
{
    hash_t hashed = hash(literal_token);

    se_list_add(et->symbol_list, (struct symbol_entry) { 
        .hash = hashed, 
        .header_name = et->header,
    });
}

void export_end(export_t et)
{
    merge(&sym_file->symbols, et->symbol_list);

    free(et->symbol_list);
    free(et);
}

export_t
new_export(const char* restrict header_name)
{
    export_t ret = (export_t) malloc( sizeof(*ret) );

    // looking to see if the header already exists inside the header table
    for (int i = 0; i < sym_file->headers.ff_num; i++)
        if ( !strcmp(sym_file->headers.from_file[i], header_name) ) {
            ret->header = i;
            goto found_header;
        }
    
    for (int i = 0; i < sym_file->headers.new_num; i++)
        if ( !strcmp(sym_file->headers.new_headers[i], header_name) ) {
            ret->header = (i + sym_file->headers.new_num);
            goto found_header;
        }
        
    ret->header = ht_add_header(&sym_file->headers, header_name);

found_header:
    // calloc to zero the list for null head and size 0
    ret->symbol_list = (struct se_list*) calloc(1, sizeof(struct se_list));
    
    return ret;
}
////////////////////////////////

// not clear that this complexity is really worth anything...
// need to do more research on 64bit hashing of strings
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
hash(const char* restrict str)
{
    hash_t hash = 0xA71B0403F93;

    for (; *str; str++)
    {
        hash <<= *str;
        hash += murmur64(hash);
    }

    return hash;
}
