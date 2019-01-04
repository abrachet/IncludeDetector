#pragma once

#define PARSE_THREAD_EXIT_SUCCESS (void*) 0
// failure will be pointing to malloc(3) allocated failure struct
// that has not been implemented, probably never will be
void orchastrate_threads(struct file_list_pair*, void (*)(struct needed_headers*, const char*));