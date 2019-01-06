#pragma once
//#include <ID.h>
//#include <Tokens/Tokens.h>
////////// MAKE SURE TO INCLUDE THESE AGAIN//////

#include <stdint.h>
//#include <Parsing/ParserShared.h>

// Legacy from how exporting used to work when I was debugging the 
enum ExportType {
    ET_Define,     //preprocesor defines

    ET_Type,       //typedef, struct creation

    ET_Symbol,     //function names are symbols, but we will seperate the two
    //function and variable are more specific symbols
    ET_Function,   
    ET_Variable,
};


// of course I don't expect more than UINT32_MAX headers that would be insane
// 64 bit wide type is used here just for padding really, making sizeof(struct symbol_entry) == 16
// signed because slightly faster than unsigned and no need for the extra 2^63 possible headers to reference...
typedef int64_t header_t;

// legacy
//typedef header_t export_t;

struct _export_t {
    header_t header;
    struct se_list* symbol_list;

    // used by is_type 
    // was a whole order to pass a user_types* otherwise because of how
    // the function map works
    struct user_types* user_types;
};

typedef struct _export_t* export_t;

// takes the header name 
export_t new_export(const char*);

// merges to the symfile
void export_end(export_t);

void export(const char*, export_t);

// using signed because although unsigned for some types is very easy,
// these are constantly being compared. the difference between signed and unsigned
// for many compares is not nothing
typedef uint64_t hash_t;

hash_t hash(const char* restrict);
