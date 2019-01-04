#pragma once

#include <stdint.h>

typedef enum TokenType {
    TT_Operator = 0,
    TT_PreprocessorLine, //pass entire line as a token

    TT_Literal, //string, integral, char literals
    TT_TypeSpecifier,
    TT_Type,

    TT_Unknown,
} TokenType;

// we can safely use pointers with only 61 bits. and much fewer than that as well
// I only needed 3 for the enum so thats all I took
typedef struct token  {
    uintptr_t   type:3,     // enum TokenType
                str:61;     // char *
} Token;
