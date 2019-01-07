#include <ID.h>
#include <pthread/pthread.h>
#include <Parsing/Parser.h>
#include "arg_parser.h"
//will be changed to thread_conductor.c source once it is stable
#include "thread_conductor.h"
#include <Export/sym_file.h>

// global for the entire program, many parts need this. 
// no point passing it around, that would be nonsense
struct sym_file* sym_file      = NULL;
pthread_mutex_t sym_file_mutex = PTHREAD_MUTEX_INITIALIZER;
// debug_log_mutex used by DEBUG_LOG
pthread_mutex_t debug_log_mutex  = PTHREAD_MUTEX_INITIALIZER;
// used to print about headers 
pthread_mutex_t std_out_mutex  = PTHREAD_MUTEX_INITIALIZER;

FILE* debug_file;

#ifndef NDEBUG
#include <signal.h>
#endif

int 
main(int argc, char** argv)
{
    // debug logging, etc
    ID_runtime_init();

    sym_file = open_sym_file();

    if (!sym_file) {
        DEBUG_LOG("Error opening sym_file\n");
        return EXIT_FAILURE;
    }

    // command line parsing
    struct file_list_pair* files = parse_args(argc, argv);

    // call the thread conductor to orchestrate the parsing of headers into the sym_table
    //          and then subsequently the searching from source files
    //          it will print out necessary headers
    orchastrate_threads(files, &needed_headers_print);

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
