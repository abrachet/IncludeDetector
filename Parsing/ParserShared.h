#pragma once

#include <ID.h>
#include <Export/export.h>
#include "Tokens.h"

bool is_operator(const char* restrict);
bool is_keyword (const char* restrict);
bool is_type_specifier(const char* restrict);
bool is_standard_type(const char* restrict);
bool is_typedef(const char* restrict);
bool is_attribute(const char* restrict);
bool is_preprocessor(const char* restrict);
bool is_template(const char* restrict);
bool is_object_def(const char* restrict);
bool is_namespace(const char* restrict);

bool is_known(const char* restrict);

bool always_true(const char* restrict);

////// User Types Vector ///////

#ifndef _UT_INITAL_SIZE
#define _UT_INITAL_SIZE 100
#endif

#ifndef _UT_GROWTH_RATE
#define _UT_GROWTH_RATE 1.2
#endif

#define ut_vec_new() (struct user_types) {0}

// A user type vector holds only refernces to strings
// all strings should outlive the user_types vector
struct user_types {
    size_t max_size;
    size_t length;
    const char** types;
};

static inline void 
ut_vec_add_type(struct user_types* ut, const char* restrict type)
{
    if (ut->length == ut->max_size) {
        ut->max_size *= _UT_GROWTH_RATE;
        ut->types = (const char**) realloc(ut->types, ut->max_size);
    }

    ut->types[ut->length++] = type;
}

bool is_type(struct user_types*, const char* restrict);

#define should_not_be_exported(str) (!is_known(str))

/////////////////////////////////////////

#ifndef _CT_LIST_SIZE
#define _CT_LIST_SIZE 506
#endif

// assumes pagesize is 4096
// each node is page alligned including the bytes malloc adds to keep track of pointers
struct alloc_page {
    //64 bits of malloc info
    struct alloc_page* prev;
    struct alloc_page* next;

    uint16_t length;
    char __reserved__[6]; //padding and reserved for later

    struct token tokens[_CT_LIST_SIZE];
};

void alloc_page_free(struct alloc_page*);



///////// Needed headers ///////////

#ifndef _NH_INITIAL_SIZE
#define _NH_INITIAL_SIZE 10
#endif

#ifndef _NH_GROWTH_RATE
#define _NH_GROWTH_RATE 1.2
#endif

//#include "Parser.h"

struct needed_headers nh_new();
void nh_add_header(struct needed_headers*, const char*, const char*);
void print_needed_headers(struct needed_headers*, pthread_mutex_t);

void _parse_header(const char*, export_t);

struct needed_headers _parse_source(struct alloc_page* tokenized_file);
