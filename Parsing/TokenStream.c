/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * These functions are an iterator for iterating over the tokenized file
 * as well as some helper functions for traversing the list of tokens
 */ 

#include "TokenStream.h"
#include "ParserShared.h"

#include <string.h>

char* 
tk_get_str(struct token* tk)
{
    return (char*)tk->str;
}

inline struct current_token* 
ct_begin(struct alloc_page* file)
{
    struct current_token* ct = (struct current_token*) calloc(1, sizeof(struct current_token) );

    ct->current_page = file;
    return ct;
}

inline struct current_token*
ct_clone(struct current_token* ct)
{
    struct current_token* ret = (struct current_token*) malloc(sizeof(struct current_token));
    
    memcpy(ret, ct, sizeof(struct current_token));

    return ret;
}

inline void
ct_free(struct current_token* ct)
{
    free(ct);
}

inline bool 
ct_eof(struct current_token* ct)
{
    return (!ct->current_page->next && (ct->current_index >= ct->current_page->length - 1));
}

struct token* 
ct_get_token(struct current_token* ct)
{
    return ct->current_page->tokens + ct->current_index;
}

bool 
ct_next(struct current_token* ct)
{
    if ( unlikely(ct->current_index == _CT_LIST_SIZE - 1) ) {
        if (unlikely( !ct->current_page->next) )
            return false;
        
        ct->current_page = ct->current_page->next;
        ct->current_index = 0;
        return true;
    }

    ct->current_index++;

    #ifndef NDEBUG
    ct->ct_debug_count++;
    #endif

    return true;    
}

bool 
ct_prev(struct current_token* ct)
{
    if ( unlikely( !ct->current_index) ) {
        if ( unlikely( !ct->current_page->prev) )
            return false;

        ct->current_page = ct->current_page->prev;
        ct->current_index = _CT_LIST_SIZE;
        return true;
    }

    ct->current_index--;

    #ifndef NDEBUG
    ct->ct_debug_count--;
    #endif
    return true;
}

static bool 
_is_scope_creator(char c)
{
    switch (c) {
        case '{':
        case '(':
        case '[':
        case '<':
            return true;
        
        default:
            return false;
    }
}

static inline bool
is_scope_ender(char c)
{
     switch (c) {
        case '}':
        case ')':
        case ']':
        case '>':
            return true;
        
        default:
            return false;
    }
}

#define ct_get_char(ct) tk_get_str(ct_get_token(ct))[0]

bool 
ct_advance_past_scope(struct current_token* ct)
{
    char c = ct_get_char(ct);

    if ( !_is_scope_creator(c) ) {
        ct_next(ct);
        c = ct_get_char(ct);

        if ( !_is_scope_creator(c) ) {
            ct_prev(ct);
            return false; //incorrect token to call
        }
    }

    ct_next(ct);
    for (int stack = 1; stack; ct_next(ct)) {
        c = ct_get_char(ct);

        stack += _is_scope_creator(c) ? 1 : is_scope_ender(c) ? -1 : 0;
    }

    return true;
}

bool
is_scope_creator(char* c)
{
    return _is_scope_creator(*c);
}

bool 
ct_advance_past_semi_colon(struct current_token* ct)
{
    while (ct_get_char(ct) != ';') {
        if (!ct_next(ct))
            return false;
    }

    return true;
}
