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
 */ 

#include "Parser.h"
#include <Export/export.h>
#include <Export/sym_file.h> // for struct se_list
#include "Scanner.h"

#include <unistd.h>
#include <stdbool.h>

static bool preprocessor(struct current_token*, export_t);
static bool type_name(struct current_token*, export_t);
static bool type_specifier(struct current_token*, export_t);
static bool attribute(struct current_token*, export_t);
static bool type_def(struct current_token*, export_t);
static bool symbol(struct current_token*, export_t);
static bool template_(struct current_token*, export_t); //underscode because editor parser was very confused using the symbol "template"
static bool object_definition(struct current_token*, export_t);

static bool generic(struct current_token*, export_t);


#define Lexer_t(in_f, call_f) (struct lexer_t) {.in = in_f, .call = call_f }

static struct lexer_t map[] = {
    Lexer_t(is_template, template_),            //LexFuncs::Template
    Lexer_t(is_type_specifier, type_specifier), //etc...
    Lexer_t(is_attribute, attribute),
    Lexer_t(is_typedef, type_def),
    Lexer_t(is_object_def, object_definition),
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

    expect(TypeSpecifier, ct, et);

    //expect(Template, ct);

    //expect(ObjectDefinition, ct);

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
    ct_get_token(ct)->type = TT_TypeSpecifier;
    ct_next(ct);

    expect(TypeSpecifier, ct, et);

    expect(TypeName, ct, et);

    expect(Attribute, ct, et);

    expect(Symbol, ct, et);

    return false;
}

static bool
type_name(struct current_token* ct, export_t et)
{
    ct_get_token(ct)->type = TT_Type;
    ct_next(ct);

    expect(Attribute, ct, et);

    expect(TypeSpecifier, ct, et);

    expect(Symbol, ct, et);

    return false;
}

static bool
type_def(struct current_token* ct, export_t et)
{
    ct_next(ct);

    //expect(ObjectDefinition)

    while (((char*)ct_get_token(ct)->str)[0] != ';') {
        ct_next(ct);
    }

    ct_prev(ct);
    //if ( (char*) (ct_get_token(ct)->str)[0] == ']') {

    //}
    char* str = tk_get_str(ct_get_token(ct));
    if (!str)
        return false;
    export(str, et);
    ct_next(ct);
    //do typedef stuff

    //go till ; if we see () then the first 
    // pair of () will have the typedef in it
    //add it to user types. 

    return true; //unless ct_eof();
}

static bool
template_(struct current_token* ct, export_t et)
{
    return true;
}

static bool
object_definition(struct current_token* ct, export_t et)
{
    return true;
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
        return true;
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
