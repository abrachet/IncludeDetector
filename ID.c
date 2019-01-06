/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * This file provides functions used throughout the entire program 
 * that otherwise have no obvious place
 * 
 */ 
#include "ID.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <Export/sym_file.h>


extern FILE* debug_file;
extern struct sym_file* sym_file;

#if 0
static struct file_path _temp_dir = { .current_length = 0 };

static inline void
assign_temp_dir(void)
{
    if (_temp_dir.current_length) return;

    fp_append(&_temp_dir, getenv("TEMPDIR"));
    fp_append(&_temp_dir, "com.id.IncludeDetector");
    
    if ( access(_temp_dir.path, __FILE_PERMISSIONS__) == -1 )
        (void) mkdir(_temp_dir.path, __FILE_PERMISSIONS__);
    
}

// must be const because nothing should be updating this
const struct file_path*
ID_temp_dir(void)
{
    if (_temp_dir.current_length == 0)
        assign_temp_dir();

    return &_temp_dir;
}

void
init_debugging(void)
{
    if (_temp_dir.current_length == 0)
        assign_temp_dir();
    
    // relatively expensive assignment
    struct file_path debug_dir = _temp_dir;

    fp_append(&debug_dir, "/debug_logs");

    if (access(debug_dir.path, __FILE_PERMISSIONS__) == -1)
        (void) mkdir(debug_dir.path, __FILE_PERMISSIONS__);

    fp_append(&debug_dir, "debug_log" __TIME__);

    debug_file = fopen(debug_dir.path, "rw+");

    // redirecting stderr here as well. likely to remove this later though
    stderr = debug_file;
}
#endif

void
ID_runtime_init(void)
{
    // init_debugging();
    debug_file = stderr;
}
