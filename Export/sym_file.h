#pragma once

#include "export.h"

#include <stdint.h>
#include <sys/_types/_size_t.h>
#include <stdbool.h>

// used to distinguish between char* and a char* from a mmaped file
// big confusion when with func(char* file) is that the file name or the file's buffer? 
typedef char    mmaped_t;

// almost all functions are completly dependent on mmap
// in the future I would like to make them to interface with a file layer
// which can fall back on read(2) however this seems like more work than is neccessary for right now
// not sure which OS's people are writing source code on that don't offer mmap anyway though...

// just a random number that is unlikely to be there
#define SF_FILE_ERROR (0xA2F89DA)
#define SF_FILE_DNE   (0xA2F89DB)

struct sf_file {
    mmaped_t*     file;
    size_t        file_size;  
    int           fd;           // filedes opened from open(2)
    int           error;        // if set to SF_FILE_ERROR, then file was not opened correctly
};


// hash symbol and assign get its header number
// this is easily serializable and O(log n) searchable
// much easier to deal with than raw strings in many aspects
// headers of course must be preserved in the file as strings, and cannot be hashed
// they are saved in a "header table", 0 seperated character arrays.
// header_name refers to the position in the table for this header
struct symbol_entry {
    hash_t   hash;
    header_t header_name;
};

// singly linked list of symbol_entries to construct the array i
// in a (hopefully) more effecient way than a raw array
// always allocated by malloc(3), ie free(3)'able
struct se_list_node {
    struct symbol_entry entry;
    struct se_list_node* next;
};

// used to add symbols from source file to create into a struct sym_table
// size is used to easily allocate space for a struct sym_table
// this should always be in order. always add with se_list_add()
struct se_list {
    struct se_list_node* head;
    int size;
};

void se_list_add(struct se_list*, struct symbol_entry);

// used to load from sym file and to create a new symbol table from source file
// ::arr should always be allocated with malloc(3)
struct sym_table {
    struct symbol_entry* arr;
    size_t length;
}; //16

// this gets loaded from the file
// from_file is allocated from malloc(3)
// the char* it points to are in the mmap'd region, never attemp to free them
// new_headers likewise is allocated from malloc(3), and its strings with strdup(3), free'able
// using uint32_t here instead of size_t for minor size concerns. 
// It will almost certainly be padded to 32 bytes but i'm leaving this one to the compiler
// probably should just make the uint32_t into header_t instead...
struct header_table {
    char**   from_file;
    uint32_t ff_num;
    char**   new_headers;
    uint16_t new_num;
    uint16_t max_capacity;
}; 

// used internally never written to the file
// used to create and pass around sym files
struct sym_file {
    struct sym_table    symbols;
    struct header_table headers;

    struct sf_file      sf_file;
}; 

struct sym_file_header {
    uint32_t num_syms;
    uint32_t num_headers;
    hash_t   hash_assert;   // must be equal to hash("IncludeDetector")
                            // this checks both if it is a valid header, and
                            // that the hash functions used for both are the same
};  // sizeof(struct sym_file_header) == 16
    // if compiler is realigning this 8 byte aligned struct then ¯\_(ツ)_/¯


// took this out, not sure wether or not to keep it
// this is the path to the director therefore the sym_file should be in _sym_file_dir + "/sym_file"
extern char* _sym_file_dir; // should only ever be changed by open_sym_file which finds it

// finds symbol file on the system first looking in getenv("TMPDIR") + "/IncludeDetector"
// then HOME, then PWD before it gives up, returning a zeroed sym_file
struct sym_file* open_sym_file(void);

// creates a symbol table from an se_list,
// allocated memory for se_list.size array of symbol entries
struct sym_table st_create(struct se_list);

// merges the linked list pointed to by list into the sym_tables sym array
// both the sym_table's array and the linked list are presumed to be already in order
// they will be if they are only inserted with the proper functions
// free's the nodes in the linked list
void merge(struct sym_table*, struct se_list);

// DEPRECATED
// changed to write_sym_file
// not yet sure how it should work
// but it renames the old symfile, found from _sym_file_dir, creates the new one,
// upon error it will rename the old file back and return false
// otherwise the new one will be in SYM_DIR/sym_file
// old one will be removed by remove(3), if it exists at all
bool create_sym_file(struct sym_file*); /* [[deprecated]] */

// this creates a header_table from the file loading in the header string literals into an array
// takes a 3rd argument int for how many char* to initially allocate, most of the times this will be 1 or 0
// the char* point to NULL
struct header_table load_header_table(mmaped_t* const, struct sym_file_header);


// NOT YET IMPLEMENTED
// realloc(3)'s the tables new strings to add more headers
header_t ht_add_header(struct header_table*, char*);

// returns the string literal from a header_t in struct symbol_entry
#define ht_index(ht, index)  index < ht.ff_num ? ht.from_file[index] : ht.new_headers[index - ht.ff_num]

// free's the char**'s and the strings the second char** point to
// if the table is it self allocated, it will not be released by ht_free
// not clear if this function should even exist
void ht_free(struct header_table);

// will get the sizeof the needed memory block to fit every string including the seperating '/0'
size_t ht_get_size(struct header_table);

// used by load_header_table, 
// it is also potentially useful for loading into the header_table::new_headers
// so it will be a public function, althought its unclear wether or not it will be used anywhere else at the moment
char** to_str_arr(char* strings, int num_strings);

// serializes sym_file struct and its data structures into a file
// writing into _sym_file_dir + "/sym_file"
// must free the sym_file because it unmaps the pages
bool write_sym_file(struct sym_file*);

// puts the symbols in the list
void merge(struct sym_table*, struct se_list);
