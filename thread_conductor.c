/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * This file orchastrates what happens and when. 
 * after command line arguments have been parsed 
 * control goes here
 * 
 * THIS FILE IS IN TESTING, NOT READY FOR THE BUILD
 */ 

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <pthread/pthread.h>

#include <Parsing/Scanner.h>
#include "thread_conductor.h"
#include "arg_parser.h"


struct file_list {
    struct file_list* next;

    char* file_name;    // file name from the command line
                        // or found through a recursive search in a directory
                        // we use the file name and dont open the file elsewhere because
                        // it gets preprocessed with cc -E
};


static struct file_list* header_files = NULL;
static struct file_list* source_files = NULL;
static int num_cpus;

static pthread_mutex_t _stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef __linux
#include <sys/sysinfo.h> /* has get_nprocs defined */
#else
// this is gnu source
// their implementation std::thread::hardware_concurrency
// apple doesn't provide either get_nprocs (linux) or pthread_num_processors_np -_-
// need to rely on sysctl(3)
static inline int 
get_nprocs()
{
    int count;
    size_t size = sizeof(count);
    int mib[] = { CTL_HW, HW_NCPU };
    if (!sysctl(mib, 2, &count, &size, NULL, 0))
        return count;
    return 0;
}
#endif

static void* tc_parse_header(void *);
static void* tc_parse_source(void *);


struct thread {
    bool        available; // if the thread is currently running
    pthread_t   thread;    // its 
};

struct parser_arg {
    char* filename;

    bool* condition_variable;
};

struct parser_arg* file_list_next(struct file_list*);

static void 
conduct(void)
{
    // our thread pool should be one minus the number of cpus because my thread pool
    // is implemented poorly and will take a lot of cpu time. whoops
    // alloca because its faster than malloc we dont need the memory to outlive this function
    struct thread thread_pool[] = alloca(sizeof(struct thread) * ( num_cpus - 1));

    // they are all available because of course they have no currently running threads at this point
    for (int i = 0; i < num_cpus - 1; i++)
        thread_pool[i].available = true;

    // signed is fine because an overflow is fine, worst case a few threads are skipped and will be returned right back
    // going signed saves like 2 instructions 😎
    // forces counting in RCX, maybe should just go with a smaller width howerver doubtful RCX is highly contested anyway
    for (int64_t i = 0; ; i++) {
        const int64_t index = i % (num_cpus - 1);

        if (thread_pool[index].available) {
            struct parser_arg* arg = file_list_next(header_files);
            if (arg) {
                thread_pool[index].available = false;
                arg->condition_variable = &thread_pool[index].available;
                pthread_create(&thread_pool[index].thread, NULL, &tc_parse_header, arg);
            }
        }
    }

    // maybe later I should care about its return value, but right now I dont
    for (int i = 0; i < num_cpus - 1; i++)
        pthread_join(thread_pool[i].thread, NULL);
    
    // I just inlined these functions myself i guess...

    for (int64_t i = 0; ; i++) {
        const int64_t index = i % (num_cpus - 1);

        if (thread_pool[index].available) {
            struct parser_arg* arg = file_list_next(source_files);
            arg->condition_variable = &thread_pool[index].available;
            pthread_create(&thread_pool[index].thread, NULL, &tc_parse_source, arg);
        }
    }
}

/// Possible parse_header implementation
static void * 
tc_parse_header(void* arg)
{
    struct parser_arg* args = (struct parser_arg*) arg;

    parse_header(args->filename);

    args->condition_variable = true;
    return PARSE_THREAD_EXIT_SUCCESS;
}

static void*
tc_parse_source(void* arg)
{
    struct parser_arg* args = (struct parser_arg*) arg;

    parse_source(args->filename);
   
    args->condition_variable = true;
    return PARSE_THREAD_EXIT_SUCCESS;
}

void 
conduct_header_parsing()
{
    
}

void 
orchastrate_threads(struct file_list_pair* pair)
{
    header_files = pair->header_files;
    source_files = pair->source_files;

    num_cpus = get_nprocs();

    conduct();

    /// MAKE THIS IN SEPERATE PARTS
    /// CONDUCT THE HEADER FILES, AND THEN SOURCE FILES
    /// IN DIFFERNT FUNCTIONS
}
