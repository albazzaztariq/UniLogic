#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_char_from_code(int code) {
    char* r = (char*)__ul_malloc(2); r[0] = (char)code; r[1] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

char* get_state_name(int state);
int next_state(int state, int ped_button);
int state_duration(int state);
void simulate(int* events, int event_count);
int count_transitions(int* events, int event_count);
int main(void);

char* get_state_name(int state)
{
    if ((state == 0)) {
        return "RED";
    }
    if ((state == 1)) {
        return "GREEN";
    }
    if ((state == 2)) {
        return "YELLOW";
    }
    if ((state == 3)) {
        return "PED_WALK";
    }
    if ((state == 4)) {
        return "PED_FLASH";
    }
    return "UNKNOWN";
}

int next_state(int state, int ped_button)
{
    if ((state == 0)) {
        if ((ped_button == 1)) {
            return 3;
        }
        return 1;
    }
    if ((state == 1)) {
        if ((ped_button == 1)) {
            return 2;
        }
        return 2;
    }
    if ((state == 2)) {
        return 0;
    }
    if ((state == 3)) {
        return 4;
    }
    if ((state == 4)) {
        return 0;
    }
    return 0;
}

int state_duration(int state)
{
    if ((state == 0)) {
        return 30;
    }
    if ((state == 1)) {
        return 45;
    }
    if ((state == 2)) {
        return 5;
    }
    if ((state == 3)) {
        return 20;
    }
    if ((state == 4)) {
        return 10;
    }
    return 0;
}

void simulate(int* events, int event_count)
{
    int state = 0;
    int total_time = 0;
    int transitions = 0;
    int time_in_red = 0;
    int time_in_green = 0;
    int time_in_yellow = 0;
    int time_in_ped = 0;
    int i = 0;
    while ((i < event_count)) {
        int ped = events[i];
        int dur = state_duration(state);
        char* name = get_state_name(state);
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", i);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", dur);
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", ped);
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("tick ", _cast_buf_0), ": state="), name), " dur="), _cast_buf_1), " ped="), _cast_buf_2));
        if ((state == 0)) {
            time_in_red = (time_in_red + dur);
        }
        if ((state == 1)) {
            time_in_green = (time_in_green + dur);
        }
        if ((state == 2)) {
            time_in_yellow = (time_in_yellow + dur);
        }
        if (((state == 3) || (state == 4))) {
            time_in_ped = (time_in_ped + dur);
        }
        total_time = (total_time + dur);
        state = next_state(state, ped);
        transitions = (transitions + 1);
        i = (i + 1);
    }
    printf("%s\n", "--- Summary ---");
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", transitions);
    printf("%s\n", __ul_strcat("transitions: ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", total_time);
    printf("%s\n", __ul_strcat("total_time: ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", time_in_red);
    printf("%s\n", __ul_strcat("time_in_red: ", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", time_in_green);
    printf("%s\n", __ul_strcat("time_in_green: ", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", time_in_yellow);
    printf("%s\n", __ul_strcat("time_in_yellow: ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", time_in_ped);
    printf("%s\n", __ul_strcat("time_in_ped: ", _cast_buf_8));
    printf("%s\n", __ul_strcat("final_state: ", get_state_name(state)));
}

int count_transitions(int* events, int event_count)
{
    int state = 0;
    int count = 0;
    int prev_state = (-1);
    int i = 0;
    while ((i < event_count)) {
        if ((state != prev_state)) {
            count = (count + 1);
        }
        prev_state = state;
        state = next_state(state, events[i]);
        i = (i + 1);
    }
    return count;
}

int main(void)
{
    printf("%s\n", "=== Traffic Light State Machine ===");
    printf("%s\n", "--- Run 1: No pedestrians ---");
    int events1[] = {0, 0, 0, 0, 0, 0, 0, 0};
    simulate(events1, 8);
    printf("%s\n", "--- Run 2: Pedestrian at tick 1 ---");
    int events2[] = {0, 1, 0, 0, 0, 0, 0, 0};
    simulate(events2, 8);
    printf("%s\n", "--- Run 3: Multiple pedestrians ---");
    int events3[] = {1, 0, 0, 1, 0, 0, 1, 0, 0, 0};
    simulate(events3, 10);
    printf("%s\n", "--- Transition counts ---");
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", count_transitions(events1, 8));
    printf("%s\n", __ul_strcat("run1: ", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", count_transitions(events2, 8));
    printf("%s\n", __ul_strcat("run2: ", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", count_transitions(events3, 10));
    printf("%s\n", __ul_strcat("run3: ", _cast_buf_11));
    return 0;
}


