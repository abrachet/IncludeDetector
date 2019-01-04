#include <ID.h>

#include <pthread/pthread.h>

#include "arg_parser.h"
#include "thread_conductor.h"
#include <Export/sym_file.h>

void needed_headers_print(struct needed_headers*, const char*);

// global for the entire program, many parts need this. 
// no point passing it around, that would be nonsense
struct sym_file* sym_file     = NULL;
// std_err_mutex used by DEBUG_LOG
pthread_mutex_t std_err_mutex = PTHREAD_MUTEX_INITIALIZER;

// used to print about headers 
pthread_mutex_t std_out_mutex = PTHREAD_MUTEX_INITIALIZER;

int 
main(int argc, char** argv)
{
    // command line parsing
    struct file_list_pair* files = parse_args(argc, argv);

    // call the thread conductor to orchestrate the parsing of headers into the sym_table
    //          and then subsequently the searching from source files
    //          it will print out necessary headers
    orchastrate_threads(files, needed_headers_print);

    // commit the sym_file back to disk
    write_sym_file(sym_file);
}

/**
 * Notes: parse_args should return more than just a file_list_pair
 *          it should have more detailed information about the arguments passed
 * 
 *        the thread conductor should return a list of needed headers, or print them out
 *          as that respective process returns
 * 
 *        commiting the new sym_file back should be a choice from command line arguments
 * 
 */ 