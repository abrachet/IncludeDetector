#pragma once
//#include <ID.h>
//#include <Tokens/Tokens.h>
////////// MAKE SURE TO INCLUDE THESE AGAIN//////

#include <stdint.h>

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

typedef header_t export_t;
// takes the header name 
export_t new_export(char*);


void export(const char*, export_t);

// using signed because although unsigned for some types is very easy,
// these are constantly being compared. the difference between signed and unsigned
// for many compares is not nothing
typedef int64_t hash_t;

hash_t hash(char*);
