#pragma once

struct file_list {
    struct file_list* next;

    const char* file_name;  // file name from the command line
                            // or found through a recursive search in a directory
                            // we use the file name and dont open the file elsewhere because
                            // it gets preprocessed with cc -E
};

struct file_list_pair {
    struct file_list* header_files;
    struct file_list* source_files;
};

struct file_list_pair* parse_args(int, char**);
