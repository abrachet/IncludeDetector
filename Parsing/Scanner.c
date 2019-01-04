#include "Scanner.h"
#include <string.h>
#include "TokenStream.h"


static void
add_token(const char* const restrict str, const enum TokenType type,
          struct alloc_page* tokenized_file)
{

    static struct alloc_page* current_page = NULL;
    if (!current_page)
        current_page = tokenized_file;

    if (current_page->length == _CT_LIST_SIZE) {
        current_page->next = (struct alloc_page*) calloc(1, sizeof(struct alloc_page));

        current_page->next->prev = current_page;

        current_page = current_page->next;
    }

    current_page->tokens[current_page->length++] = (struct token) { .str = (uintptr_t)str, .type = (int8_t) type };
}

//curent_token[len] is always ok because it will be behind our string
//even if str is not current_token
#define push_back(str, type, file)  \
    do {                            \
        assert(len < 1024);         \
        if (len) {                  \
            current_token[len] = 0; \
            char* __str = strdup(current_token); \
            add_token(__str, type, file); \
        }                           \
    } while(0)


static void 
scan_line(char* restrict str, const size_t _len, 
          struct alloc_page* tk_file)
{
    // replace '\n' with '\0'
    str[_len - 1] = 0; 

    // quite the big buffer for tokens
    char current_token[1024];
    int  len = 0;

    char* searching = str;
    for (; *searching && (*searching == ' ' || *searching == '\t' ); searching++ );

    if (*searching == '#') {
        searching = strdup(searching);
        add_token(searching, TT_PreprocessorLine, tk_file);
        return;
    }

    bool string = false;

    for (int i = 0; i < _len; i++) {
        char c = str[i];

        if ( c == '"' && string ) {
            string = false;
            current_token[len++] = c;
            push_back(current_token, TT_Literal, tk_file);
            len = 0;
            continue;
        }

        if ( c == '"' && !string ) 
            string = true;

        if (string) 
            goto _else;

        if ( is_operator(&c) ) {
            push_back(current_token, TT_Unknown, tk_file);
            len = 1;
            current_token[0] = c;
            push_back(current_token, TT_Operator, tk_file);
            len = 0;
        } else if ( c == ' ' || c == '\t' ) {
            push_back(current_token, TT_Unknown, tk_file);

            len = 0;
        } else {
            _else:
            current_token[len++] = c;
        }
    }
}

struct alloc_page*
scan_file(FILE* file)
{
    struct alloc_page* tokenized_file = (struct alloc_page*) calloc(1, sizeof(struct alloc_page));

    char* line = NULL;
    size_t linecap = 0;
    ssize_t len;
    while ( (len = getline(&line, &linecap, file)) > 0 )
        scan_line(line, len, tokenized_file);

    return tokenized_file;
}

// tail call
void 
alloc_page_free(struct alloc_page* ap)
{
    if (!ap)
        return;

    for (int i = 0; i < ap->length; i++)
        free(ap->tokens[i].str);

    struct alloc_page* next = ap->next;

    free(ap);

    alloc_page_free(next);
}
