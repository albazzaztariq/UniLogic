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
static void __ul_print_float(double v) {
    char _buf[64]; snprintf(_buf, sizeof(_buf), "%.10g", v);
    int _has_dot = 0; for (int _i = 0; _buf[_i]; _i++) if (_buf[_i] == '.' || _buf[_i] == 'e') { _has_dot = 1; break; }
    if (!_has_dot) { int _l = (int)strlen(_buf); _buf[_l] = '.'; _buf[_l+1] = '0'; _buf[_l+2] = 0; }
    printf("%s\n", _buf); }

char* state_name(int s);
int next_state(int current);
int state_is_terminal(int s);
int run_statemachine(int initial, int max_steps);
int main(void);

char* state_name(int s)
{
    if ((s == 0)) {
        return "INIT";
    } else {
        if ((s == 1)) {
            return "CONNECT";
        } else {
            if ((s == 2)) {
                return "RESOLVING";
            } else {
                if ((s == 3)) {
                    return "CONNECTING";
                } else {
                    if ((s == 4)) {
                        return "CONNECTED";
                    } else {
                        if ((s == 5)) {
                            return "SENDPROTOCONNECT";
                        } else {
                            if ((s == 6)) {
                                return "PROTOCONNECT";
                            } else {
                                if ((s == 7)) {
                                    return "DO";
                                } else {
                                    if ((s == 8)) {
                                        return "DOING";
                                    } else {
                                        if ((s == 9)) {
                                            return "DID";
                                        } else {
                                            if ((s == 10)) {
                                                return "PERFORMING";
                                            } else {
                                                if ((s == 11)) {
                                                    return "RATELIMITING";
                                                } else {
                                                    if ((s == 12)) {
                                                        return "DONE";
                                                    } else {
                                                        if ((s == 13)) {
                                                            return "COMPLETED";
                                                        } else {
                                                            if ((s == 14)) {
                                                                return "MSGSENT";
                                                            } else {
                                                                return "UNKNOWN";
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int next_state(int current)
{
    if ((current == 0)) {
        return 1;
    } else {
        if ((current == 1)) {
            return 2;
        } else {
            if ((current == 2)) {
                return 3;
            } else {
                if ((current == 3)) {
                    return 4;
                } else {
                    if ((current == 4)) {
                        return 5;
                    } else {
                        if ((current == 5)) {
                            return 6;
                        } else {
                            if ((current == 6)) {
                                return 7;
                            } else {
                                if ((current == 7)) {
                                    return 8;
                                } else {
                                    if ((current == 8)) {
                                        return 9;
                                    } else {
                                        if ((current == 9)) {
                                            return 10;
                                        } else {
                                            if ((current == 10)) {
                                                return 12;
                                            } else {
                                                if ((current == 12)) {
                                                    return 13;
                                                } else {
                                                    if ((current == 13)) {
                                                        return 14;
                                                    } else {
                                                        return 15;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int state_is_terminal(int s)
{
    return (((s == 13) || (s == 14)) || (s == 15));
}

int run_statemachine(int initial, int max_steps)
{
    int state = initial;
    int steps = 0;
    int done = 0;
    do {
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", steps);
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("  step ", _cast_buf_0), ": "), state_name(state)));
        if (state_is_terminal(state)) {
            done = 1;
        } else {
            state = next_state(state);
            steps = (steps + 1);
        }
    } while (((done == 0) && (steps < max_steps)));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", steps);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("  final: ", state_name(state)), " steps="), _cast_buf_1));
    return steps;
}

int main(void)
{
    int i = 0;
    printf("%s\n", "=== State Machine Traversal ===");
    run_statemachine(0, 20);
    printf("%s\n", "=== State Names ===");
    i = 0;
    while ((i < 15)) {
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", i);
        static char _cast_buf_3[64];
        snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", state_is_terminal(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("state=", _cast_buf_2), " name="), state_name(i)), " terminal="), _cast_buf_3));
        i = (i + 1);
    }
    return 0;
}


