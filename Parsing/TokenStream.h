#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ParserShared.h"

struct current_token {
    struct alloc_page* current_page;    // its called a page because an sizeof(struct alloc_page) == pagesize
    size_t current_index;               // in that page

    #ifndef NDEBUG
    size_t ct_debug_count;
    #endif
};

// returns the token which the current_token iterator is pointing to
struct token* ct_get_token(struct current_token*);
// returns the literal string representation from a token
char* tk_get_str(struct token*);

// makes the struct current_token go forward or backward
bool ct_next(struct current_token*);
bool ct_prev(struct current_token*);

// checks if at the end of the token list
bool ct_eof(struct current_token*);

//return true if the token is '{', '<', '(' as well as '[' for expedience
bool is_scope_creator(char*);
// will advance the stream until the matching ending of the scope '}'...
// this is used when parsing headers for example when for functions we dont care about the arguments.
// likewise for cpp templates we dont care about the body of the classes or functions
bool ct_advance_past_scope(struct current_token*);

// used when a variable is found like int a = 5; once on token 'a' it would advance the current_token one
// one past the semi colon
bool ct_advance_past_semi_colon(struct current_token*);

// allocates a struct current_token from malloc
// its unclear wether ct needs to be heap allocated but for convience I will
// it should be free'd by the caller
struct current_token* ct_begin(struct alloc_page*);
