/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * These functions are used in parsing to determine if tokens are known keywords, operators, etc
 * 
 * A known bug is that these functions do nothing differently between C and C++ source
 * even this program would be difficult to run on itself because it uses "export" as a function,
 * a keyword in C++. It seems to me not worth the trouble to distinguish between the two because it is 
 * anyway bad practice to do what i did, no one is realistically writing C code that uses C++ reserved words
 * 
 */ 

#include <string.h>
#include "ParserShared.h"

#define SIZEOF_STR_ARR(arr) \
    (sizeof(arr) / sizeof(char*))

const char 
__operators[] = {
    '[', ']', '(', ')', '{', '}',
    ',', '.', '-', '=',
    '+', '-', '*', '/',
    '!', '~', '=', '>', '<', 
    '&', '|', '^', ';', ':',
};

bool
is_operator(char* restrict str) 
{
    char c = *str;
    for (int i = 0; i < sizeof(__operators); i++)
        if (__operators[i] == c)
            return true;

    return false;
}


const char* 
__keywords[] = {
    "alignas", "alignof", "and",
    "and_eq", "asm", "atomic_cancel",
    "atomic_commit", "atomic_noexcept", 
    "bitand", "bitor", 
    "break", "case", "catch",
    "class", "compl",
    "concept", "consteval",
    "constexpr", "const_cast", "continue",
    "co_await", "co_return", "co_yield",
    "decltype", "default", "delete",
    "do", "dynamic_cast",
    "else", "enum", "explicit",
    "export", "false",
    "for", "friend",
    "goto", "if", "import",
    "module", "mutable", "namespace",
    "new", "noexcept", "not",
    "not_eq", "nullptr", "operator",
    "or", "or_eq", "private",
    "protected", "public", "reflexpr",
    "reinterpret_cast", "requires",
    "return", "sizeof", "static_assert",
    "static_cast", "struct", "switch",
    "synchronized", "template", "this",
    "throw", "true",
    "try", "typeid",
    "typename", "union", 
    "using", "virtual",
    "while",
    "xor", "xor_eq", "override", 
    "final", "audit", "axiom", 
    "transaction_safe", "transaction_safe_dynamic",
};

bool 
is_keyword(char* restrict str) 
{
    for (int i = 0; i < SIZEOF_STR_ARR(__keywords); i++)
        if (!strcmp(__keywords[i], str))
            return true;

    if (is_standard_type(str))
        return true;

    return false;
}

const char*
__type_specifiers[] = {
    "inline", "const", "register",
    "restrict", "volatile", "static",
    "thread_local", "_Thread_local",
    "struct", "enum", "class", //elaborated types
    "long", "short", "unsigned", "signed",
    "extern", "*", //putting pointer opperator here for expedience
    "_Nullable", 
};

bool 
is_type_specifier(char* restrict str)
{
    if (!str)
        return false;

    for (int i = 0; i < SIZEOF_STR_ARR(__type_specifiers); i++)
        if (!strcmp(__type_specifiers[i], str))
            return true;

    return false;
}

const char* 
__standard_types[] = {
    "auto", "bool", "char", 
    "char8_t", "char16_t", "char32_t",
    "double", "int", "long", "short",
    "wchar_t", "void", "float"
    "unsigned", "signed",
};

bool 
is_standard_type(char* restrict str)
{
    assert(str);

    for (int i = 0; i < SIZEOF_STR_ARR(__standard_types); i++)
        if (!strcmp(__standard_types[i], str))
            return true;

    return false;
}

bool
is_typedef(char* restrict str)
{
    return !strcmp(str, "typedef");
}

bool
is_attribute(char* restrict str)
{
    //  __asm is not an "attribute" but in headers, at least 
    //  pthreads.h which I am testing with, it is used like this
    //  this will undoubtley cause bugs not to mention the countless keywords
    //  available to inline assembly
    //  anyway, having a keyword here or there slip through the cracks is not a huge deal
    return (!strcmp(str, "__attribute__") || !strcmp(str, "__asm"));
}

bool always_true(char* restrict _) { return true; }

bool
is_preprocessor(char* restrict str)
{
    for (; *str && (*str == ' ' || *str == '\t' ); str++ );

    return (*str == '#');
}

static bool 
is_int(char* restrict str)
{
    for (; str; str++) {
        char c = *str;
        if ( c > 57 || c < 48)
            return false;
    }

    return true;
}

bool
is_template(char* restrict str)
{
    return !strcmp(str, "template");
}

bool
is_object_def(char* restrict str)
{
    if (!strcmp(str, "struct")) return true;
    if (!strcmp(str, "class"))  return true;
    if (!strcmp(str, "enum"))   return true;

    return false;
}




bool 
is_known(char* restrict str)
{
    if (!is_operator(str))       return false;
    if (!is_keyword(str))        return false;
    if (!is_type_specifier(str)) return false;
    if (!is_standard_type(str))  return false;
    if (!is_typedef(str))        return false;
    if (!is_attribute(str))      return false;
    if (!is_int(str))            return false;

    return true;
}


// user_type vector

void 
ut_vec_add_type(struct user_types* ut, const char* const restrict type)
{
    if (ut->length == ut->max_size) {
        ut->max_size *= _UT_GROWTH_RATE;
        ut->types = (char**) realloc(ut->types, ut->max_size);
    }

    ut->types[++ut->length] = type;
}

bool
ut_vec_find(const struct user_types* const ut, const char* const restrict type)
{
    for (int i = 0; i < ut->length; i++)
        if (!strcmp(ut->types[i], type))
            return true;
    
    return false;
}

bool
is_type(struct user_types* ut, char* restrict str)
{
    if ( is_standard_type(str) )
        return true;

    for (int i = 0; i < ut->length; i++) 
        if (!strcmp(ut->types[i], str))
            return true;

    return false;
}