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

int shared_counter = 0;
void worker(void);
int main(void);

// Thread infrastructure
#define __UL_MAX_THREADS 64
static pthread_t __ul_threads[__UL_MAX_THREADS];
static int __ul_thread_count = 0;
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void worker(void)
{
    int i = 0;
    while ((i < 1000)) {
        pthread_mutex_lock(&counter_lock);
        shared_counter = (shared_counter + 1);
        pthread_mutex_unlock(&counter_lock);
        i = (i + 1);
    }
}

int main(void)
{
    pthread_t _tid_0;
    pthread_create(&_tid_0, NULL, (void*(*)(void*))worker, NULL);
    __ul_threads[__ul_thread_count++] = _tid_0;
    0;
    pthread_t _tid_1;
    pthread_create(&_tid_1, NULL, (void*(*)(void*))worker, NULL);
    __ul_threads[__ul_thread_count++] = _tid_1;
    0;
    for (int _i = 0; _i < __ul_thread_count; _i++) pthread_join(__ul_threads[_i], NULL);
    __ul_thread_count = 0;
    0;
    printf("%d\n", shared_counter);
    return 0;
}


