/**
 * Alex Brachet-Mialot <abrachet@purdue.edu>
 * 
 * responsible for loading the sym_file from disk into memory and a useable form for the program
 * contains many external functions for dealing with its underlying data structures:
 * the symbol table and header table. 
 */ 
#include <ID.h>

#include "sym_file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <stddef.h>
#include <string.h>

#include <stdio.h>
#include <errno.h>

#include <pthread/pthread.h>


// this is going to need to change
#define __FILE_PERMISSIONS__ 0777

static inline void
get_sym_dir(struct file_path* fp)
{
    fp_reset(fp);

    fp_append(fp, getenv("HOME"));

    fp_append(fp, "./include_detector");

    // this means the path doesnt exist
    if ( access(fp->path, __FILE_PERMISSIONS__) == -1 ) {
        // so we try to create it
        // if it fails we dont have access so we try somewhere else
        if ( mkdir(fp->path, __FILE_PERMISSIONS__) == -1 ) {
            fp_reset(fp);

            fp_append(fp, getenv("TMPDIR"));
            fp_append(fp, "/.include_detector/");

            if (access(fp->path, __FILE_PERMISSIONS__) == -1) {

                if ( mkdir(fp->path, __FILE_PERMISSIONS__) == -1) {
                    DEBUG_LOG("Could not create a directory to put sym_file in");
                    (void) printf("Grant %s permisions to create directories", getenv("_"));
                    // not sure if i should exit but I should assume so
                    // i guess users could use it without saving anything?
                    // and only load headers and source files but that seems weird
                    exit(1);
                }
            }
        } 
    }
}

static char* 
get_sym_file_path()
{
    #if 0
    static struct file_path fp;
    static bool init = false;

    static bool already_found = false;

    if (!init) {
        fp.path[0] = 0;
        fp.current_length = 0;
        init = true;
    } else {
        if (already_found) {
            return fp.path;
        }
    }

    get_sym_dir(&fp);

    fp_append(&fp, ".symfile");

    puts(fp.path);
    
    if ( access(fp.path, __FILE_PERMISSIONS__) == -1) {
        if ( creat(fp.path, __FILE_PERMISSIONS__) == -1 ) {
            DEBUG_LOG("coudln't open file for writing");
            printf("Set permissions for %s to write and create files\n", getenv("_"));
            exit(1);
        } else {
            already_found = true;
        }
    } else {
        already_found = true;
    }

    return fp.path;
    #endif
    return "symfile";
}

static inline struct sf_file
open_file(char* path)
{
    struct sf_file ret;

    ret.fd = open(path, O_RDWR | O_CREAT | O_APPEND, __FILE_PERMISSIONS__);
    
    if (ret.fd == -1) {
        DEBUG_LOG("couldn't open file in open_file()\n"
                "filepath was: %s", path);
        goto error;
    }

    struct stat sb;
    if ( fstat(ret.fd, &sb) == -1 ) {
        char* str = strerror(errno);
        DEBUG_LOG("fstat failure %s", str);
        goto error;
    }
    
    if (sb.st_size == 0) {
        ret.error = 0;
        goto creating;
    }
        
    

    ret.file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, ret.fd, 0);

    if (ret.file == (void*) -1) {
        char* str = strerror(errno);
        DEBUG_LOG("couldn't mmap file in open_file()"
                "with error: %s", str);
        return  (struct sf_file) {0};
    }

    ret.file_size = sb.st_size;
    ret.error = 0;

    return ret;

error:
    ret.error = SF_FILE_ERROR;
creating:
    ret.file_size = 0;
    ret.file = NULL;

    return ret;
}

static inline struct sym_file_header
read_header(mmaped_t* mmaped_file)
{
    typedef struct sym_file_header sfheader_t;
    sfheader_t header = *( (sfheader_t*) mmaped_file );

    if ( unlikely(header.hash_assert != hash("IncludeDetector")) ) {
        DEBUG_LOG("Invalid File");
        return (struct sym_file_header) {0};
    }

    return header;
}

static inline struct sym_table
load_sym_table(mmaped_t* file, struct sym_file_header header)
{
    struct sym_table ret;

    ret.length = header.num_syms;
    ret.arr = (struct symbol_entry*) (file + sizeof(struct sym_file_header));

    return ret;
}

char** 
to_str_arr(char* strings, int num_strings) 
{
    char** ret = (char**) malloc(sizeof(char*) * num_strings );
    if (!ret) {
        DEBUG_LOG("Allocation failure in to_str_arr()");
        return NULL;
    }

    for (int i = 0; i < num_strings; i++) {
        ret[i] = strings;
        for (; *strings; strings++); //find the '/0'
        strings++; //advance once more till the start of the next string
    }

    return ret;
}

struct header_table
load_header_table(mmaped_t* file, struct sym_file_header header)
{
    file += sizeof(struct sym_file_header);
    file += header.num_syms * sizeof(struct symbol_entry);
    // file now pointing to our headers

    struct header_table ret;

    ret.from_file   = to_str_arr(file, header.num_headers);
    ret.ff_num      = header.num_headers;

    ret.new_headers = NULL; 
    ret.new_num     = 0;


    return ret;
}
/*
inline char* 
ht_index(struct header_table ht, int index)
{
    return index < ht.ff_num ? ht.from_file[index] : ht.new_headers[index - ht.ff_num];
}
*/
void
ht_free(struct header_table ht)
{
    free(ht.from_file);
    for (int i = 0; i < ht.new_num; i++)
        free(ht.new_headers[i]);

    free(ht.new_headers);
}


// not sure if string.h :: strlen would be inlined, im forcing inlining by hand writting
// a small optimization, but also a very small time commitment
// took me longer to explain why this function exists than to write it :/
static inline size_t
_strlen(const char* restrict str)
{
    size_t ret = 0;
    for (; *str; str++)
        ret += 1;

    return ret;
}


size_t 
ht_get_size(struct header_table ht)
{
    size_t size = 0;

    for (int i = 0; i < ht.ff_num; i++)
        size += _strlen(ht.from_file[i]) + 1; //+ 1 for '/0' at the end of the string
    
    for (int i = 0; i < ht.new_num; i++)
        size += _strlen(ht.new_headers[i]) + 1;

    return size; 
}

static inline size_t
ht_get_num_headers(struct header_table ht)
{
    return ht.ff_num + ht.new_num;
}

static inline size_t 
sf_get_size(struct sym_file* sf)
{
    size_t ret = sizeof(struct sym_file_header);
    ret += sizeof(struct symbol_entry) * sf->symbols.length;
    ret += ht_get_size(sf->headers);
    return ret;
}


// this is better than strcpy from string.h because it returns the size of what was copied
// this means only one pass if we had needed to do strcpy and strlen
static inline size_t 
_strcpy(char* dst, const char* restrict src)
{
    size_t size = 0;
    do {
        dst[size++] = *src;
    } while(*(src++));

    return size;
}

// this along with many other similar header_table functions could be
// improved using more optimal functions to take less passes through the strings
// for right now I just use the c standard library and make in some cases two passes instead of 1
// its ok for now, but just a small TODO, 
// like an int(strcmp)(char*,char*) that returns the length of the string copied
static inline void
headercpy(char* dest, struct header_table ht)
{
    size_t ff_size = 0;
    for (int i = 0; i < ht.ff_num; i++)
        ff_size += _strlen(ht.from_file[i]) + 1;

    if (ff_size)
        memcpy(dest, ht.from_file[0], ff_size);
    // the first string in ht.from_file is the start in the original mapping
    // we can just memcpy from that mapping to this one

    dest += ff_size;

    size_t next = 0;

    for (int i = 0; i < ht.new_num; i++) {
        dest += next;
        next = _strcpy(dest, ht.new_headers[i]);
    }
}

bool 
write_sym_file(struct sym_file* sf)
{
    char* file = get_sym_file_path();
    if ( rename(file, "old_sym_file.tmp") == -1 ) {
        DEBUG_LOG("rename failed with cause %s", strerror(errno));
        return false;
    }

    int fd = open(file, O_RDWR | O_CREAT, (mode_t)__FILE_PERMISSIONS__);
    if (fd == -1) {
        DEBUG_LOG("couldn't create new sym_file");
        goto rename_back;
    }

    size_t file_size = sf_get_size(sf);

    mmaped_t* mapping = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (mapping == (void*) -1) {
        DEBUG_LOG("Coudln't mmap file");
        goto rename_back;
    }

    // this is fallocate
    // but more portable, apple doesnt provide an implementation -_-
    // also no chance in hell windows has it 😂 
    // (thats a laughing crying emoji for when I open this in a text editor that doesnt support unicode)
    (void) pwrite(fd, "\0", 1, file_size - 1);

    struct sym_file_header header = {
        .num_syms    = sf->symbols.length,
        .num_headers = sf->headers.ff_num + sf->headers.new_num,
        .hash_assert = hash("IncludeDetector"),
    };

    // putting header into file
    memcpy(mapping, &header, sizeof(struct sym_file_header));

    // symbol table
    memcpy(mapping + sizeof(struct sym_file_header), 
           sf->symbols.arr,
           sf->symbols.length * sizeof(struct symbol_entry));

    
    char* dest = mapping + sizeof(struct sym_file_header) + (sf->symbols.length * sizeof(struct symbol_entry));
    headercpy(dest, sf->headers);  
    // async to not wait for system to commit the pages to the filesystem
    // not sure how much this will ever matter, however I expect the files to get big
    // c and c++ standard library should be a lot of symbols
    // moreover, users can add their own header files, including large libraries like boost
    // either way asnyc can't hurt
    if ( msync(mapping, file_size, MS_ASYNC) == -1) {
        DEBUG_LOG("msync failed. %s", strerror(errno));
        goto close_mapping;
    }
    (void) munmap(mapping, file_size);
    (void) close(fd);

    ht_free(sf->headers);
    free(sf);

    // doesnt make a big deal wether it is removed or not in honesty
    (void) remove("old_sym_file.tmp");

    return true;

close_mapping:

    (void) munmap(mapping, file_size);

rename_back:

    (void) rename("old_sym_file.tmp", get_sym_file_path());
    (void) close(fd);

    return false;
}

// TODO:
// need to handle failure, ie freeing the file when this happens
struct sym_file*
open_sym_file()
{
    struct sym_file* file = (struct sym_file*) malloc(sizeof(struct sym_file));

    char* file_path = get_sym_file_path();

    struct sf_file sf_file = open_file(file_path);

    // this is an empty file with no sym table
    // this happens when the file was just created
    if ( !sf_file.file && !sf_file.file_size && !sf_file.error) {
        file->symbols = (struct sym_table)    {0};
        file->headers = (struct header_table) {0};
        return file;
    }

    if ( unlikely(!sf_file.file && sf_file.error == SF_FILE_ERROR) ) {
        DEBUG_LOG("Couldn't mmap file");
        return NULL;
    }

    struct sym_file_header header = read_header(sf_file.file);

    file->symbols = load_sym_table(sf_file.file, header);
    file->headers = load_header_table(sf_file.file, header);

    file->sf_file = sf_file;
    return file;
}

void 
debug_se_list_add(struct se_list* this, struct symbol_entry se)
{
    struct se_list_node* node = (struct se_list_node*) malloc(sizeof(struct se_list_node) );
    this->size++;

    node->entry = se;
    node->next = NULL;

    const hash_t value = node->entry.hash;

    printf("inserting value: %llu\n", value);

    if (!this->head || this->head->entry.hash >= value) {
        node->next = this->head;
        this->head = node;
    } else {
        struct se_list_node* curr = this->head;

        while(curr->next && curr->next->entry.hash < value )
            curr = curr->next;

        node->next = curr->next;
        curr->next = node;
    }
}

void
se_list_add(struct se_list* this, struct symbol_entry se)
{
    struct se_list_node* node = (struct se_list_node*) malloc(sizeof(struct se_list_node) );
    this->size++;

    node->entry = se;
    node->next = NULL;

    const hash_t value = node->entry.hash;

    if (! this->head)
        this->head = node;

    struct se_list_node* curr = this->head;
    struct se_list_node* prev = NULL;
    for (; curr->next && curr->entry.hash < value; curr = curr->next) {
        prev = curr;
    }

    node->next = curr;
    prev->next = node;
}

void 
merge(struct sym_table* st, struct se_list list)
{
    const size_t max_size = st->length + list.size;
    struct symbol_entry* arr = 
    (struct symbol_entry*) malloc( sizeof(struct symbol_entry) * max_size );

    size_t arr_index = 0;
    size_t st_index = 0;

    struct se_list_node* list_index = list.head;

    while (list_index && st_index < st->length ) {

        if (list_index->entry.hash > st->arr[st_index].hash) {
            void* to_free = list_index;

            arr[arr_index] = list_index->entry;
            list_index = list_index->next;
            
            free(to_free);
        } else {
            arr[arr_index] = st->arr[st_index++];
        }
        arr_index++;
    }

    if (list_index) {
        for (; list_index; list_index = list_index->next)
            arr[arr_index++] = list_index->entry;
    } else {
        for (; st_index < st->length; st_index++)
            arr[arr_index++] = st->arr[st_index];
    }

    st->arr = arr;
    st->length = max_size;
    //assert(arr_index == max_size);
}
