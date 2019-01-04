#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread/pthread.h>

// this is just to give context on symbols to be exported to the global symbol table
// it isn't really for compiler but for code readability
#define __GLOBAL __attribute__ ((__visibility__("default")))

extern struct sym_file* sym_file;
extern pthread_mutex_t std_out_mutex;
extern pthread_mutex_t std_err_mutex;


#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

// should change later
#ifndef NDEBUG
#define DEBUG_LOG(...) \
    do {                    \
        pthread_mutex_lock(&std_err_mutex); \
        fprintf(stderr, "\033[91m" "[" __FILE__ ":" LINE_STRING "]\033[0m\t", __VA_ARGS__); \
        pthread_mutex_unlock(&std_err_mutex); \
    } while (0)
#else
#define DEBUG_LOG(msg) ((void)0)
#endif


#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#define noinline __attribute__ ((noinline))

inline void ID_Runtime_Assert(bool expr)
{
    if (!expr) 
        abort();
}


/*
struct files {
    struct ID_file_handler file;
    //for header files
    struct {
        bool export_symbols;
        bool use_includes; // if includes should be parsed as well, and counted from this file
    };

    char* path;

    struct files* next;
};

struct command_line_args {
    char* tmp_dir;  //not used

    struct debug_logger* opaque_debug_logger; //not yet used

    int num_files;
    struct files* head;
};

extern struct command_line_args _gloabl_args;
*/

struct file_path {
    char    path[248];
    size_t  current_length;
};

#define fp_init(fp)                 \
    do {                            \
        (fp).path[0] = 0;           \
        (fp).current_length = 0;    \
    } while(0)

#define fp_append(fp, str)          \
    do {                            \
        size_t __len_ = strlen(str);\
        strcpy((fp)->path + (fp)->current_length, str);  \
        (fp)->current_length += __len_; \
    } while(0)

#define fp_reset(fp) (fp)->current_length = 0

#define fp_remove(fp, str) (fp)->current_length -= strlen(str)
