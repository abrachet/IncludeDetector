#include "ParserShared.h"
#include "TokenStream.h"
#include <Export/export.h>
#include <Export/find_symbol.h>
#include <Parsing/Parser.h>
#include <string.h>


#define nh_new() (struct needed_headers) { \
    .headers  = calloc(1, sizeof(struct needed_header) * _NH_INITIAL_SIZE ), \
    .max_size = _NH_INITIAL_SIZE,          \
    .length   = 0                          \
}


// headers and symbols must be used with strdup because it is not possible
// to assume that the pointer will outlive the needed_headers struct
// these strings will be made by the scanner calling strdup on strings from a file
// it si very likely that all of those strings be freed before these
//
// Note: symbols which were gotten from those headers were saved however its not
// clear how they will be represented to the user. perhaps a verbose option.
// its outside of the scope of this function however so I have just decided its easier to remove it
// later
void 
nh_add_header(struct needed_headers* nh, const char* header, const char* symbol)
{
    struct needed_header to_add;

    for (int i = 0; i < nh->length; i++)
        if ( !strcmp(nh->headers[i].header, header) ) {
            to_add = nh->headers[i];
            goto found;
        }

    if (nh->length == nh->max_size) {
        //size_t old_size = nh->max_size;
        nh->max_size *= _NH_GROWTH_RATE;
        nh->headers = realloc(nh->headers, sizeof(*nh->headers) * nh->max_size);
    }

    to_add = nh->headers[nh->length++] = (struct needed_header) { 
        .header = strdup(header), 
        .head = NULL
    };


found:

    for (struct string_list* curr = to_add.head; curr; curr = curr->next)
        if ( !strcmp(curr->str, symbol) )
            return; 

    struct string_list* new = malloc(sizeof(struct string_list));

    new->next = 0;
    new->str  = strdup(symbol);

}

void print_needed_headers(struct needed_headers*, pthread_mutex_t);

struct needed_headers
_parse_source(struct alloc_page* tokenized_file)
{
    struct needed_headers headers = nh_new();

    struct current_token* ct = ct_begin(tokenized_file);

    char* literal_token;
    const char* found_header;

    while (!ct_eof(ct)) {
        struct token* tk = ct_get_token(ct);
        if (tk->type == TT_PreprocessorLine)
            goto end;

        literal_token = tk_get_str( tk );

        if (!is_known(literal_token)) {
            if ( (found_header = find_header(literal_token)) )
                nh_add_header(&headers, found_header, literal_token);
        }
end:
       ct_next(ct);
    }

    return headers;
}

void 
needed_headers_print(struct needed_headers* nh, const char* filename)
{
    printf("File: '%s' requires: \n", filename);
    for (int i = 0; i < nh->length; i++)
        puts(nh->headers[i].header);
    
}
