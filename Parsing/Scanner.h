#pragma once

#include <stdio.h>

#include <ID.h>
#include "Tokens.h"
#include "ParserShared.h"

extern const char  __known_operators[];
extern const char* __known_keywords[];
extern const char* __standard_types[];

bool is_keyword (char* restrict);
bool is_operator(char*);

struct alloc_page* scan_file(FILE*);
