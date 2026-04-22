/* Fake typedefs for pycparser — covers common GCC/MSVC extensions */
#ifndef _FAKE_TYPEDEFS_H
#define _FAKE_TYPEDEFS_H

/* GCC builtins / attributes — make them disappear */
#define __attribute__(x)
#define __extension__
#define __inline__ inline
#define __inline inline
#define __const const
#define __volatile__ volatile
#define __restrict restrict
#define __signed__ signed
#define __asm__(x)
#define __asm(x)
#define __builtin_va_list void*
#define __gnuc_va_list void*
#define __builtin_expect(x,y) (x)
#define __builtin_offsetof(type, member) 0
#define __builtin_types_compatible_p(t1,t2) 0
#define __GNUC__ 10
#define __GNUC_MINOR__ 0
#define __GNUC_PATCHLEVEL__ 0

/* MSVC extensions */
#define __cdecl
#define __stdcall
#define __declspec(x)
#define __forceinline inline
#define _Noreturn
#define __noreturn__

/* Pragmas — strip them */
#define _Pragma(x)

/* stdint types */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef unsigned int uintptr_t;
typedef int intptr_t;
typedef long ptrdiff_t;
typedef unsigned long size_t;
typedef long ssize_t;
typedef int bool;
#define true 1
#define false 0
#define NULL 0

/* stdbool */
#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1
#endif

/* stddef */
#define offsetof(type, member) ((size_t)0)

/* errno */
typedef int errno_t;
extern int errno;

/* File I/O */
typedef int FILE;
typedef int fpos_t;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
#define EOF (-1)

/* limits */
#define INT_MAX 2147483647
#define INT_MIN (-2147483647-1)
#define UINT_MAX 4294967295U
#define LONG_MAX 2147483647L
#define ULONG_MAX 4294967295UL
#define SIZE_MAX 4294967295UL
#define CHAR_BIT 8

/* va_list */
typedef void* va_list;
#define va_start(ap, last) ((void)0)
#define va_end(ap) ((void)0)
#define va_arg(ap, type) (*(type*)ap)

/* common functions */
int printf(const char *fmt, ...);
int fprintf(FILE *f, const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);
int snprintf(char *s, size_t n, const char *fmt, ...);
int scanf(const char *fmt, ...);
int fscanf(FILE *f, const char *fmt, ...);
int sscanf(const char *s, const char *fmt, ...);
int puts(const char *s);
int fputs(const char *s, FILE *f);
int fputc(int c, FILE *f);
int fgetc(FILE *f);
char *fgets(char *s, int n, FILE *f);

size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t n);
char *strstr(const char *hay, const char *needle);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strtok(char *s, const char *delim);
char *strdup(const char *s);
int atoi(const char *s);
long atol(const char *s);
double atof(const char *s);
long strtol(const char *s, char **end, int base);
unsigned long strtoul(const char *s, char **end, int base);
double strtod(const char *s, char **end);

void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *p, size_t size);
void free(void *p);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);

void exit(int code);
void abort(void);
int abs(int x);
long labs(long x);
int rand(void);
void srand(unsigned int seed);

FILE *fopen(const char *path, const char *mode);
int fclose(FILE *f);
size_t fread(void *buf, size_t size, size_t count, FILE *f);
size_t fwrite(const void *buf, size_t size, size_t count, FILE *f);
int fseek(FILE *f, long offset, int whence);
long ftell(FILE *f);
int fflush(FILE *f);

double floor(double x);
double ceil(double x);
double fabs(double x);
double sqrt(double x);
double pow(double x, double y);
double log(double x);
double exp(double x);
double fmod(double x, double y);

/* POSIX */
typedef int pid_t;
typedef unsigned int uid_t;
typedef int off_t;
typedef unsigned int mode_t;

#endif /* _FAKE_TYPEDEFS_H */
