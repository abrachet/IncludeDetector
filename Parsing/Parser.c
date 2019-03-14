/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * Implemention of outward facing parse_header and parse_source
 * The majority of the work for extracting symbols from header files
 * is done in _parse_header.c and _parse_source.c
 * 
 */ 

#include "Parser.h"
#include "Scanner.h"
#include <Export/export.h>

#include "ParserShared.h"

#include <stdio.h>
#include <string.h>

/* relic from before using clang
// can make a faster way to appened filepath to "cc -E"
// but its not important right now. Small TODO
static inline FILE*
preprocessed_file(const char* restrict filepath)
{
    char buf[256];
    // turning warnings off because sometimes the preprocessor complains?
    (void) snprintf(buf, 256, "cc -w -E %s", filepath);

    return popen(buf, "r");
}
*/

// having char** maybe seem weird, but getline
// takes this, by doing this we can optimize by letting the compiler keep the pointer
// in its respective register. probably getline the compile is not allowed to make
// the assumption that the register wont be changed because getline is not static
// but it cant hurt...
// these loops can be very easily unwound as well
static inline char*
find_define(char** line)
{
    // quite the large macro name...
    static char ret[512];

    const char* str = *line;

    for (; *str != '\n' && *str == ' '; ++str );

    // this means it isnt a preprocessor line
    // bug here is that you could do /*...*/ #define
    // but if youre doing that youre being a pain and its too much work to be worth it
    // its way too unlikely. Also no standard headers would ever look like that
    if ( likely(*str != '#') )
        return NULL;
    
    str++;

    for (; *str != '\n' && *str == ' '; ++str );

    if (strncmp(str, "define", 6))
        return NULL;
    
    str += 6;

    for (; *str != '\n' && *str == ' '; ++str );

    int i = 0;
    
    // this looks not very robust, and admitedly it could be better
    // however ISO C99 standard forbids macros without whitespace after them
    // so #define DEREF_STR*str would expand DEREF_STR to *str 
    // because the preprocessor knows * is an operator which would end a token
    // however no one should be doing this, but thats not to say that 
    // it shouldn't be changed in the future, it should
    for (; *str != ' ' && *str != '(' && *str != '\n'; i++, str++)
        ret[i] = *str;
    
    ret[i] = 0;

    return ret;
}

//static 
void 
parse_defines(FILE* file, export_t et)
{
    char*  line  = NULL;
    size_t size  = 0;

    char* found;

    while ( getline(&line, &size, file) != -1 ) {
        if ( (found = find_define(&line)) )
            export(found, et);
    }
}

void
parse_header(const char* restrict filepath)
{
    FILE* before_pp = fopen(filepath, "r");

    if (!before_pp) {
        DEBUG_LOG("unable to open file %s", filepath);
        return;
    }

    export_t et = new_export(filepath);

    //parse_defines(before_pp, et);

    (void) fclose(before_pp);

    /*
    FILE* after_pp = preprocessed_file(filepath);

    if (!after_pp) {
        DEBUG_LOG("popen failed from preprocessed_file");
        return;
    }

    struct alloc_page* tokenized_file = scan_file(after_pp);

    (void) fclose(after_pp);
    */
    _parse_header(filepath, et);

    //alloc_page_free(tokenized_file);
    export_end(et);
}

struct needed_headers 
parse_source(const char* restrict filepath)
{
    FILE* file = fopen(filepath, "r");

    if (!file) {
        DEBUG_LOG("Couldn't open file: '%s'", filepath);
        return (struct needed_headers) {0};
    }

    struct alloc_page* tokenized_file = scan_file(file);

    (void) fclose(file);

    struct needed_headers headers = _parse_source(tokenized_file);

    alloc_page_free(tokenized_file);

    return headers;
}
