#define	_STDIO_H_

#include <_stdio.h>

void	 clearerr(FILE *);
int	 fclose(FILE *);
int	 feof(FILE *);
int	 ferror(FILE *);
int	 fflush(FILE *);
int	 fgetc(FILE *);
int	 fgetpos(FILE * __restrict, fpos_t *);
char	*fgets(char * __restrict, int, FILE *);
#if defined(_DARWIN_UNLIMITED_STREAMS) || defined(_DARWIN_C_SOURCE)
FILE	*fopen(const char * __restrict __filename, const char * __restrict __mode) __DARWIN_ALIAS_STARTING(__MAC_10_6, __IPHONE_3_2, __DARWIN_EXTSN(fopen));
#else /* !_DARWIN_UNLIMITED_STREAMS && !_DARWIN_C_SOURCE */
FILE	*fopen(const char * __restrict __filename, const char * __restrict __mode) __DARWIN_ALIAS_STARTING(__MAC_10_6, __IPHONE_2_0, __DARWIN_ALIAS(fopen));
#endif /* (DARWIN_UNLIMITED_STREAMS || _DARWIN_C_SOURCE) */
int	 fprintf(FILE * __restrict, const char * __restrict, ...) __printflike(2, 3);
int	 fputc(int, FILE *);
int	 fputs(const char * __restrict, FILE * __restrict) __DARWIN_ALIAS(fputs);
size_t	 fread(void * __restrict __ptr, size_t __size, size_t __nitems, FILE * __restrict __stream);
FILE	*freopen(const char * __restrict, const char * __restrict,
                 FILE * __restrict) __DARWIN_ALIAS(freopen);
int	 fscanf(FILE * __restrict, const char * __restrict, ...) __scanflike(2, 3);
int	 fseek(FILE *, long, int);
int	 fsetpos(FILE *, const fpos_t *);
long	 ftell(FILE *);
size_t	 fwrite(const void * __restrict __ptr, size_t __size, size_t __nitems, FILE * __restrict __stream) __DARWIN_ALIAS(fwrite);
int	 getc(FILE *);
int	 getchar(void);
char	*gets(char *);
void	 perror(const char *);
int	 printf(const char * __restrict, ...) __printflike(1, 2);
int	 putc(int, FILE *);
int	 putchar(int);
int	 puts(const char *);
int	 remove(const char *);
int	 rename (const char *__old, const char *__new);
void	 rewind(FILE *);
int	 scanf(const char * __restrict, ...) __scanflike(1, 2);
void	 setbuf(FILE * __restrict, char * __restrict);
int	 setvbuf(FILE * __restrict, char * __restrict, int, size_t);
int	 sprintf(char * __restrict, const char * __restrict, ...) __printflike(2, 3) __swift_unavailable("Use snprintf instead.");
int	 sscanf(const char * __restrict, const char * __restrict, ...) __scanflike(2, 3);
FILE	*tmpfile(void);