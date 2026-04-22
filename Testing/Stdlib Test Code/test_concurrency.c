// @dr concurrency = threaded

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }
#include <pthread.h>
#include <stdlib.h>

void worker(int id);
int main(void);

// Thread infrastructure
#define __UL_MAX_THREADS 64
static pthread_t __ul_threads[__UL_MAX_THREADS];
static int __ul_thread_count = 0;
typedef struct { int id; } __ul_worker_args;
void* __ul_worker_wrapper(void* args) {
    __ul_worker_args* a = (__ul_worker_args*)args;
    worker(a->id);
    __ul_free(args);
    return NULL;
}

void worker(int id)
{
    printf("%d\n", id);
}

int main(void)
{
    __ul_worker_args* _args_2 = (__ul_worker_args*)__ul_malloc(sizeof(__ul_worker_args));
    _args_2->id = 1;
    pthread_t _tid_0;
    pthread_create(&_tid_0, NULL, __ul_worker_wrapper, _args_2);
    int _handle_1 = __ul_thread_count;
    __ul_threads[__ul_thread_count++] = _tid_0;
    _handle_1;
    __ul_worker_args* _args_5 = (__ul_worker_args*)__ul_malloc(sizeof(__ul_worker_args));
    _args_5->id = 2;
    pthread_t _tid_3;
    pthread_create(&_tid_3, NULL, __ul_worker_wrapper, _args_5);
    int _handle_4 = __ul_thread_count;
    __ul_threads[__ul_thread_count++] = _tid_3;
    _handle_4;
    __ul_worker_args* _args_8 = (__ul_worker_args*)__ul_malloc(sizeof(__ul_worker_args));
    _args_8->id = 3;
    pthread_t _tid_6;
    pthread_create(&_tid_6, NULL, __ul_worker_wrapper, _args_8);
    int _handle_7 = __ul_thread_count;
    __ul_threads[__ul_thread_count++] = _tid_6;
    _handle_7;
    for (int _i = 0; _i < __ul_thread_count; _i++) pthread_join(__ul_threads[_i], NULL);
    __ul_thread_count = 0;
    0;
    return 0;
}


