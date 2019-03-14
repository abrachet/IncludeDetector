#include <ID.h>
#include <Export/export.h>

#include <clang-c/Index.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>



enum CXChildVisitResult cursorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
enum CXChildVisitResult 
static parse_header(CXCursor cursor, CXCursor parent, void* _parser_info);

typedef int user_types_vec;

struct parser_info {
    export_t et;
    bool currently_exporting_external;
    const char* filename;
};


void
_parse_header(const char* filename, export_t et)
{
    printf("Parsing headers\n");

    CXIndex index = clang_createIndex(true, false);

    // we are safe to compile as C++ because this is only for headers
    // still, we use no-deprecated because maybe it will complain if it is a c header
    // headers are allowed to have the restrict type qualifier however this doesn't exist in cpp
    // im sure it will complain
    const char *args[] = {
        "-x",
        "c++",
        "-Wno-deprecated",
    };

    CXTranslationUnit parsed = clang_parseTranslationUnit(index, filename, args, 
                                    (sizeof(args) / 8), NULL, 0, CXTranslationUnit_None);

    CXCursor rootCursor = clang_getTranslationUnitCursor(parsed);


    struct parser_info client_data = {
        .et = et,
        .currently_exporting_external = false,
        .filename = filename,
    };

    (void) clang_visitChildren(rootCursor, &parse_header, &client_data);

    

}

#define clang_get_string(cursor) clang_getCString(clang_getCursorSpelling(cursor))

static enum CXChildVisitResult 
parse_header(CXCursor cursor, CXCursor parent, void* client_data)
{
    struct parser_info* parser_info = (struct parser_info*)client_data;

    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation location = clang_getRangeStart(range);
    
    CXFile file;
    {
        unsigned line;
        unsigned column;
        clang_getFileLocation(location, &file, &line, &column, NULL);
    }

    const char* filename = clang_getCString(clang_getFileName(file));

    if (!filename) {
        DEBUG_LOG("Error getting filename from clang cursor");
        return CXChildVisit_Break;
    }

    
    //if (strcmp(parser_info->filename, filename) && !parser_info->currently_exporting_external)
     //   return CXChildVisit_Continue;

    
    const char* current_token = NULL;

    switch ( clang_getCursorKind(cursor) ) {
        
        
        case CXCursor_EnumDecl: 
        case CXCursor_Namespace:
            return CXChildVisit_Recurse;

        current_token = NULL;

        case CXCursor_FunctionDecl:
            
            current_token = clang_get_string(cursor);
            if (!strcmp("__ID_export_begin", current_token))
                parser_info->currently_exporting_external++;
            else if (!strcmp("__ID_export_end", current_token))
                parser_info->currently_exporting_external--;


        case CXCursor_ClassTemplate: case CXCursor_FunctionTemplate:
        case CXCursor_TypedefDecl:   case CXCursor_TypeRef:
        case CXCursor_ClassDecl:     case CXCursor_StructDecl:
        case CXCursor_VarDecl:       case CXCursor_EnumConstantDecl:
        case CXCursor_CallExpr:      case CXCursor_UnionDecl:    
        

        printf("Exporting %s\n from source file %s\n", clang_get_string(cursor),
        filename);
        export(clang_get_string(cursor), parser_info->et);

        #if 0
            if (!current_token)
                export(clang_get_string(cursor), parser_info->et);
            else 
                export(current_token, parser_info->et);
        #endif
        default:;
    }
    return CXChildVisit_Continue;
}
