/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * Why is it called lexer not parser? Not sure... but i cant be bothered to change file names
 * 
 * This handwritten RDP attempts to extract useful tokens from the file
 * specifically, this file deals with exclusively header files, and when symbols are found calls export
 * 
 * perhaps in the future a more streamlined approach will be taken which can be used on both headers and source files
 * 
 * specifically the distinction between the two is that this doesnt attempt to look anywhere other than global scope
 * and as was already stated, when a symbol is found it is exported. it does not annotate the tokens with their respective type
 * which would be a much more versitile way to solve the problem, but a more difficult one on my end as well.
 * 
 * DEPRECATED:
 * No longer being used because C++ source is too difficult to parse
 * It worked pretty well for C code, however C++ is just too hard, even when only extracting symbols
 */ 

#error "This file is deprecated and no longer being used"

#include "Parser.h"
#include <Export/export.h>
#include <Export/sym_file.h> // for struct se_list
#include "Scanner.h"

#include <unistd.h>
#include <stdbool.h>
#include <string.h> // for strcmp
#include <assert.h>




static bool preprocessor(struct current_token*, export_t);
static bool type_name(struct current_token*, export_t);
static bool type_specifier(struct current_token*, export_t);
static bool attribute(struct current_token*, export_t);
static bool type_def(struct current_token*, export_t);
static bool symbol(struct current_token*, export_t);
static bool object_definition(struct current_token*, export_t);
static bool template(struct current_token*, export_t);
static bool namespace(struct current_token*, export_t);


//temp
bool is_namespace(const char* restrict);

static bool generic(struct current_token*, export_t);

#define Lexer_t(in_f, call_f) (struct lexer_t) {.in = in_f, .call = call_f }
#if 0
typedef enum LexFuncs {
    Template = 0, //need to ensure 0 first to index map
    TypeSpecifier,
    Attribute,
    TypeDef,
    ObjectDefinition,

    Namespace,
    
    Symbol, //just falls back on symbol

    Preprocessor,

    TypeName, // == 7
} LexFuncs;
#endif

static struct lexer_t map[] = {
    Lexer_t(is_template, template),            //LexFuncs::Template
    Lexer_t(is_type_specifier, type_specifier), //etc...
    Lexer_t(is_attribute, attribute),
    Lexer_t(is_typedef, type_def),
    Lexer_t(is_object_def, object_definition),
    Lexer_t(is_namespace, namespace),              // Namespace
    Lexer_t(always_true, symbol),
    Lexer_t(is_preprocessor, preprocessor),

    // its .in function is different
    //Lexer_t(is_type, type_name),                //LexFuncs::TypeName
};

#define MAP_SIZE (sizeof(map) / sizeof(*map))

static inline struct lexer_t
find(enum LexFuncs lf)
{
    assert((int)lf < MAP_SIZE);

    return map[(int)lf];
}



#define expect(enum, ct, et)                        \
    do {                                            \
        if (enum == TypeName) {                     \
            if (is_type(et->user_types, tk_get_str(ct_get_token(ct))))   \
                return type_name(ct, et);           \
        } else {                                    \
            struct lexer_t lt = find(enum);         \
            if (lt.in( tk_get_str(ct_get_token(ct))))   \
                return lt.call(ct, et);            \
        }                                          \
    } while(0)


//lets you use the current_token iterator afterwords without returning
#define no_return_expect(enum, ct)                  \
    do {                                            \
        struct lexer_t lt = find(enum);             \
        if (lt.in( tk_get_str(ct_get_token(ct))))   \
            lt.call(ct);                            \
    } while(0)


void
_parse_header(struct alloc_page* tokenized_file, export_t et)
{
    struct current_token* ct = ct_begin(tokenized_file);

    struct user_types user_types = {0};

    et->user_types = &user_types;

    while (!ct_eof(ct)) {
        generic(ct, et);
    }

    export_end(et);
    
    free(ct);
}

static bool 
generic(struct current_token* ct, export_t et)
{
    expect(Preprocessor, ct, et);

    expect(Namespace, ct, et);

    expect(TypeSpecifier, ct, et);

    expect(Template, ct, et);

    expect(ObjectDefinition, ct, et);

    expect(TypeName, ct, et);
    
    expect(Attribute, ct, et);

    expect(TypeDef, ct, et);

    expect(Symbol, ct, et);

    return false;
}

static bool 
preprocessor(struct current_token* ct, export_t et)
{
    //do things later
    ct_next(ct);
    return true;
}

static bool
attribute(struct current_token* ct, export_t et)
{
    ct_next(ct);

    if ( unlikely(!ct_advance_past_scope(ct)) )
        return false; //will return false on ct_eof()
    
    if (tk_get_str(ct_get_token(ct))[0] == ';')
        ct_next(ct);

    expect(Attribute, ct, et);

    expect(TypeSpecifier, ct, et);

    expect(TypeName, ct, et);

    //expect '='

    expect(Symbol, ct, et);

    return false;
}

static bool
type_specifier(struct current_token* ct, export_t et)
{
    const char* str = ct_str(ct);
    assert(is_type_specifier(str));
    puts("=======");
    puts(str);
    puts("=======");
    ct_next(ct);

    expect(TypeSpecifier, ct, et);

    expect(TypeName, ct, et);

    // inline namespace
    expect(Namespace, ct, et);

    expect(Attribute, ct, et);

    expect(Symbol, ct, et);

    return false;
}

static bool
type_name(struct current_token* ct, export_t et)
{
    ct_get_token(ct)->type = TT_Type;
    ct_next(ct);

    if (ct_str(ct)[0] == '<')
        ct_advance_past_scope(ct);

    expect(Attribute, ct, et);

    expect(TypeSpecifier, ct, et);

    expect(Symbol, ct, et);

    return false;
}

static bool 
namespace(struct current_token* ct, export_t et)
{
    ct_next(ct);

    ut_vec_add_type(et->user_types, ct_str(ct));

    ct_next(ct);

    assert(ct_str(ct)[0] == '{');

    ct_next(ct);

    return true;
}


// This needs to be redone
// ie use expect so that type_def works the same as the rest of the program
// the problem is there is no function which can easily handle new types
// symbols works for identifers, and object definition is for new objects
// typedef is more difficult. Function pointers are hard, so too are typedef'd arrays
// can't just go from typedef to the first symbol before the ;
static bool
type_def(struct current_token* ct, export_t et)
{
    // using goes to type_def() too
    // in this case we dont care about using statements because
    // they only have local implications
    if ( !strcmp(ct_str(ct), "using") )
        return ct_advance_past_semi_colon(ct);
    

    ct_next(ct);

    expect(ObjectDefinition, ct, et);

    while ( ((char*)ct_get_token(ct)->str)[0] != ';')
        ct_next(ct);
    
    assert(!ct_eof(ct));

    ct_prev(ct);

    if ( unlikely(ct_str(ct)[0] == ']') ) {
        while (ct_str(ct)[0] != '[')
            ct_prev(ct);

        ct_prev(ct);
    }

    const char* str = ct_str(ct);

    export(str, et);

    ut_vec_add_type(et->user_types, str);

    ct_next(ct);


    //go till ; if we see () then the first 
    // pair of () will have the typedef in it
    //add it to user types. 

    return true; //unless ct_eof();
}


#define add_to_stack(str) (str[0] == '<' ? 1 : str[0] == '>' ? -1 : 0)


#if 0
// im probably missing some keywords which can be used
// in place of these but I dont use C++ I can easily fix this later
static inline bool
_typename(const char* restrict str)
{
    if (!strcmp(str, "typename"))   return true;
    if (!strcmp(str, "class"))      return true;

    return false;
}
#endif

static bool
template(struct current_token* ct, export_t et)
{
    ct_next(ct);

    assert(ct_str(ct)[0] == '<');


/*
    int stack = 1;

    bool ignore = false;

    ct_next(ct);

    while (stack) {
        const char* str = ct_str(ct);
        stack += add_to_stack(str);

        if (ignore) {
            if (str[0] == ',')
                ignore = false;

            continue;
        }

        if (_typename(str)) {
            ct_next(ct);
            ut_vec_add_type(et->user_types, ct_str(ct));
            ignore = true;
        }
        ct_next(ct);
    }
*/

    ct_advance_past_scope(ct);

    expect(Attribute, ct, et);

    expect(TypeSpecifier, ct, et);

    expect(TypeName, ct, et);

    expect(ObjectDefinition, ct, et);

    return ct_next(ct);
}

// gets here from struct class enum or union keywords
static bool
object_definition(struct current_token* ct, export_t et)
{
    ct_next(ct);

    const char* str = ct_str(ct);

    //expect(Attribute, ct, et);
    while (is_attribute(str)) {
        ct_next(ct);
        ct_advance_past_scope(ct);
    }

    export(str, et);
    
    // go past any inheritance nonsense
    while (str[0] != '{') {
        ct_next(ct);
        str = ct_str(ct);
    }

    return ct_advance_past_scope(ct);
}

static bool
symbol(struct current_token* ct, export_t et)
{
    char* current = tk_get_str(ct_get_token(ct));

    //if next token after symbol is '('
    //export function
    //update ct to go to the end of function scope
    //if next token is = or ;, export variable
    //
    //else use unsure_export(); does more checking on the token
    ct_next(ct);
    char* next_tk_str = tk_get_str(ct_get_token(ct));
    if ( is_scope_creator(next_tk_str) ) {
        export(current, et);
        ct_advance_past_scope(ct);
        struct current_token* peek = ct;
        next_tk_str = tk_get_str(ct_get_token(peek));
        if (is_scope_creator(next_tk_str)) {
            if (!ct_next(ct))
                return false;
            if (!ct_advance_past_scope(ct))
                return false;
        }
        
        expect(Attribute, ct, et);
        return ct_next(ct);
    }

    else if ( *next_tk_str == '=' ) {
        export(current, et);
        if (!ct_advance_past_semi_colon(ct))
            return false;

        return true;
    }

    export(current, et);

    
    return true;
}
