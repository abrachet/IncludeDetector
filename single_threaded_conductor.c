
#include "arg_parser.h"
#include <Parsing/Parser.h>
#include <Export/export.h>

static void
_headers(struct file_list* files)
{
    struct file_list* curr = files;
    while(curr) {
        struct file_list* to_free = curr;
        
        parse_header(curr->file_name);
        
        curr = curr->next;

        free(to_free);
    }
}


static void
_sources(struct file_list* files, void (*print_func)(struct needed_headers*, const char*))
{
    
    struct file_list* curr = files;
    while(curr) {
        struct file_list* to_free = curr;

        struct needed_headers nh = parse_source(curr->file_name);

        pthread_mutex_lock(&std_out_mutex);
        print_func(&nh, curr->file_name);
        pthread_mutex_unlock(&std_out_mutex);

        curr = curr->next;

        free(to_free);
    }
}

void 
orchastrate_threads(struct file_list_pair* pair, void (*print_func)(struct needed_headers*, const char*))
{
    _headers(pair->header_files);

    _sources(pair->source_files, print_func);
}
