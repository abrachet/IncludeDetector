#pragma once

#include "ParserShared.h"
#include "Tokens.h"
#include "TokenStream.h"

#include <Export/export.h>

typedef enum LexFuncs {
    Template = 0, //need to ensure 0 first to index map
    TypeSpecifier,
    Attribute,
    TypeDef,
    ObjectDefinition,
    
    Symbol, //just falls back on symbol

    Preprocessor,

    TypeName, // == 7
} LexFuncs;


/*
bool preprocessor(struct current_token*, export_t);
bool type_name(struct current_token*, export_t);
bool type_specifier(struct current_token*, export_t);
bool attribute(struct current_token*, export_t);
bool type_def(struct current_token*, export_t);
bool symbol(struct current_token*, export_t);
bool template_(struct current_token*, export_t); //underscode because editor parser was very confused using the symbol "template"
bool object_definition(struct current_token*, export_t);

//bool generic(struct current_token*, export_t);
*/
typedef bool(*lexer_f)(struct current_token*, export_t);
 
struct lexer_t {
    bool(*in)(const char* restrict);
    lexer_f call;
};

struct sym_header_pair {
    char* symbol;
    char* header;

};

struct parsed_sourced {
    size_t length;
    struct sym_header_pair* arr;

    size_t max_length;
};

////////////////////////////////////////////////////////////

struct string_list {
    char* str;
    struct string_list* next;
};

struct needed_header {
    char* header;
    struct string_list* head; // probably wont be used for a while
};

struct needed_headers {
    size_t max_size;
    size_t length;
    struct needed_header* headers;
};

// will add symbols to the symtable
void parse_header(const char* restrict);

// will find symbols and return a needed_heders struct
// which shows which headers are needed and which symbols led to that conclusion
// in the future it will also parse includes to determine which includes are already existant
struct needed_headers parse_source(const char* restrict);

// basic printer for a needed_headers struct
void needed_headers_print(struct needed_headers* nh, const char* filename);
