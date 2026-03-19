// @dr concurrency = threaded

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
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
    free(args);
    return NULL;
}

void worker(int id)
{
    printf("%d\n", id);
}

int main(void)
{
    __ul_worker_args* _args_1 = (__ul_worker_args*)malloc(sizeof(__ul_worker_args));
    _args_1->id = 1;
    pthread_t _tid_0;
    pthread_create(&_tid_0, NULL, __ul_worker_wrapper, _args_1);
    __ul_threads[__ul_thread_count++] = _tid_0;
    0;
    __ul_worker_args* _args_3 = (__ul_worker_args*)malloc(sizeof(__ul_worker_args));
    _args_3->id = 2;
    pthread_t _tid_2;
    pthread_create(&_tid_2, NULL, __ul_worker_wrapper, _args_3);
    __ul_threads[__ul_thread_count++] = _tid_2;
    0;
    __ul_worker_args* _args_5 = (__ul_worker_args*)malloc(sizeof(__ul_worker_args));
    _args_5->id = 3;
    pthread_t _tid_4;
    pthread_create(&_tid_4, NULL, __ul_worker_wrapper, _args_5);
    __ul_threads[__ul_thread_count++] = _tid_4;
    0;
    for (int _i = 0; _i < __ul_thread_count; _i++) pthread_join(__ul_threads[_i], NULL);
    __ul_thread_count = 0;
    0;
    return 0;
}


