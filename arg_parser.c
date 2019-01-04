/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * Parses command line arguments and finds files
 * 
 * There are many better ways that this could work
 * I could create a much more robust way to parse command line arguments
 * but whats the point. I have very few options anyway. Which is not to say
 * that there wont be more in the future or that thats an excuse for creating
 * code of a lesser quality. Its just that i have been working on this for quite some
 * time now and arg handling is the last thing I have to do. I just want it to work.
 * This is not of high code quality, in honesty.
 */ 
#include <ID.h>
#include "arg_parser.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>

static void recursive_dir_search(const char* restrict);
static void add_file(const char* restrict);
static void add_file_exists(const char* restrict);

// this way we can return a pointer, but not have to make a heap allocation
// its ok that this is not as flexible as other options because we will only ever
// need to parse command line args once of course
// thus parse_args should only be called once
static struct file_list_pair _ret = {0};

struct file_list_pair* 
parse_args(int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if ( !strcmp(argv[i], "-recursive") ) {
            recursive_dir_search(argv[++i]);
            break;
        }



        // if we get here we have exhausted all options, so it must be a file
        add_file(argv[i]);
    }
}

static inline void
add_to_list(struct file_list** head, const char* restrict filename)
{
    struct file_list* new = (struct file_list*) malloc(sizeof(struct file_list));

    new->file_name = filename;

    new->next = *head;
    *head = new;
}

// TODO:
// Handle default case when no extension is found
// try file(1)?
static inline void 
add_file(const char* restrict filename)
{
    // we can make this static to not 
    static struct stat stat_buf;

    if (stat(filename, &stat_buf) == -1)
        goto error;

    if ( !S_ISREG(stat_buf.st_mode) )
        goto error;

    add_file_exists(filename);
    
    return;

error:
    DEBUG_LOG("add_file was given %s, determined not a file", filename);
}

// uses file(1) to determine if it is "c program text"
static bool 
c_prog_text(const char* filename)
{
    size_t len = strlen(filename);

    char str[100];
    strcpy(str, "file ");
    strcat(str, filename);

    FILE* proc = popen(str, "r");
    if (!proc)
        return 0;

    char* progname = NULL;
    size_t size = 100;

    getline(&progname, &size, proc);

    // if fscanf failed to match it returns 0
    return !strncmp(progname + len + 2, "c program text", 14);
}

// called when the filepath is known to go to a reachable file
// 
static void 
add_file_exists(const char* restrict filepath)
{
    char extension = get_file_extension(filepath);

    switch (extension) {
        case 'c':
            add_to_list(&_ret.source_files, filepath);
            break;
        case 'h':
            add_to_list(&_ret.header_files, filepath);
            break;
        default:
            // if it has no extension it is checked if file(1) determines it
            // to be "c program text"
            if (c_prog_text(filepath))
                add_to_list(&_ret.header_files, filepath);
            else {
                // no need to lock stdout_mutex because CLA will never be parsed in more than one thread
                printf("File '%s' is not a valid filetype\n", filepath);
                DEBUG_LOG("No extension found, treating as probable header");
            }
    }
}

static void 
recursive_dir_search(const char* restrict path) 
{
    DIR* dir = opendir(path);
    if (!dir)
        return;

    char filepath[1024];

    struct dirent* entry;
    while ( (entry = readdir(dir)) ) {
        // could consider entry->d_name[0] == '.' however, there may be headers behind hidden directories?
        // doubtful but this is what I went with. no reson to not do this I suppose
        if ( !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") )
            continue;

        (void) snprintf(filepath, 1024, "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR)
            recursive_dir_search(filepath);
        else
            add_file_exists(filepath);
    }
    (void) closedir(dir);
}

