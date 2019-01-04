
## Include Detector

Parses C/C++ files to determine which header files are needed by the source file.

For example, pthreads.h may include sys/types.h, so although the program will compile without fault
if you use a type such as size_t, it may be seen as cleaner to include sys/types directly. This is useful
because if one no longer needs pthreads.h and remove all immediately defined symbols from pthreads, the build will
never break after removing the header because size_t could still be found in the include of sys/types in the source file.

Of, note, its very difficult to determine to which header files do symbols belong. On my machine for example, sticking with
sys/types.h, size_t is not defined directly but is defined in sys/_types/_size_t.h. Moreover, it also defines types that like 
pthread_mutex_t through sys/_pthread/_pthread_mutex_t.h. This is very difficult to determine under which file the 
pthread_mutex_t belongs given that the man page for pthread does not require that sys/types be included. It is an extrodinarily
difficult problem to solve. Standard library files will be parsed under special rules. Users too can use these by using
'#pragma ID_no_export begin' and '#pragma ID_no_export end' to section off blocks off code where symbols should not be exported,
as well '#pragma ID_export being' and end where symbols would otherwise not be exported belonging to that file. By defualt,
what a header file includes is parsed but indetifiers are not exported as belonging to that file. Using these pragmas may
cause warnings from the compiler because they are unknown to it. Using -Wno-unknown-pragmas should turn these warnings off. 
Moreover, in header files, #pragma GCC system_header turns warnings like this off. 
