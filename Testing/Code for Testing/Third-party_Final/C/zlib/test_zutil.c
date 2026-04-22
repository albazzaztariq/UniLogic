/* test_zutil.c -- zutil utility function patterns for c2ul
 * Extracted from madler/zlib zutil.c
 * Exercises:
 *   - typedef chains (typedef int-to-type, typedef struct)
 *   - switch/case (zlibCompileFlags pattern) - includes case N: break; (no-op case)
 *   - string table (z_errmsg pattern - const char* array)
 *   - nested if/else if
 *   - global const string
 *   - do-while loop
 *   - struct typedef with field access
 * NOTE: function pointer callbacks hit UL wall (no fn-pointer type) - documented separately.
 */

#include <stdio.h>

/* Typedef chain: like zlib's uLong, uInt, Bytef */
typedef unsigned long uLong;
typedef unsigned int  uInt;
typedef unsigned char Bytef;

/* Error message table -- like z_errmsg[] */
static const char *errmsg[5] = {
    "ok",
    "stream error",
    "data error",
    "mem error",
    "buf error"
};

/* ERR_MSG macro pattern -- inline index computation */
const char *get_err_msg(int code) {
    int idx;
    if (code < 0 || code > 4)
        idx = 4;
    else
        idx = code;
    return errmsg[idx];
}

/* Version string -- like zlibVersion() */
static const char MYLIB_VERSION[] = "1.2.3.4";

const char *my_version(void) {
    return MYLIB_VERSION;
}

/* Switch/case pattern -- like zlibCompileFlags()
 * case 2: break; is a no-op (explicit break with no body -- must NOT fall-through).
 */
int size_flag(int sz) {
    int flag = 0;
    switch (sz) {
    case 2:  break;           /* no-op: sz==2 -> flag stays 0 */
    case 4:  flag = 1; break;
    case 8:  flag = 2; break;
    default: flag = 3; break;
    }
    return flag;
}

/* Fall-through switch pattern: case A: case B: body; break; */
int category(int c) {
    int result = 0;
    switch (c) {
    case 1:
    case 2:
    case 3:
        result = 10; break;
    case 4:
    case 5:
        result = 20; break;
    default:
        result = 99; break;
    }
    return result;
}

/* Nested if/else-if -- like zError() ERR_MSG expansion */
const char *decode_code(int code) {
    if (code == 0)
        return "ok";
    else if (code == 1)
        return "stream end";
    else if (code == -1)
        return "errno";
    else if (code == -2)
        return "stream error";
    else if (code == -3)
        return "data error";
    else
        return "unknown";
}

/* Do-while loop -- compact byte-processing pattern */
int sum_bytes_dowhile(int count) {
    int n = 0;
    int sum = 0;
    do {
        sum = sum + n;
        n = n + 1;
    } while (n < count);
    return sum;
}

/* Struct typedef -- like ptr_table_s in zutil.c */
typedef struct point_s {
    int x;
    int y;
} Point;

int point_dist_sq(Point p) {
    return p.x * p.x + p.y * p.y;
}

/* Struct with multiple operations */
Point point_add(Point a, Point b) {
    Point result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

/* Integer math function -- test typedef int usage */
uLong combine_flags(uInt a, uInt b) {
    return (uLong)a + (uLong)b;
}

int main(void) {
    /* Test 1: version string */
    printf("version: %s\n", my_version());

    /* Test 2: error message table */
    printf("errmsg[0]: %s\n", get_err_msg(0));
    printf("errmsg[2]: %s\n", get_err_msg(2));
    printf("errmsg[-1]: %s\n", get_err_msg(-1));

    /* Test 3: switch/case with no-op case (case 2: break) */
    printf("size_flag(2)=%d\n", size_flag(2));
    printf("size_flag(4)=%d\n", size_flag(4));
    printf("size_flag(8)=%d\n", size_flag(8));
    printf("size_flag(1)=%d\n", size_flag(1));

    /* Test 4: fall-through switch */
    printf("category(1)=%d\n", category(1));
    printf("category(2)=%d\n", category(2));
    printf("category(4)=%d\n", category(4));
    printf("category(9)=%d\n", category(9));

    /* Test 5: nested if/else-if */
    printf("decode(0): %s\n", decode_code(0));
    printf("decode(1): %s\n", decode_code(1));
    printf("decode(-2): %s\n", decode_code(-2));
    printf("decode(99): %s\n", decode_code(99));

    /* Test 6: do-while */
    printf("sum_bytes_dowhile(5) = %d\n", sum_bytes_dowhile(5));
    printf("sum_bytes_dowhile(1) = %d\n", sum_bytes_dowhile(1));

    /* Test 7: struct typedef */
    Point p;
    p.x = 3;
    p.y = 4;
    printf("dist_sq(3,4) = %d\n", point_dist_sq(p));

    /* Test 8: struct operations */
    Point a;
    a.x = 1;
    a.y = 2;
    Point b;
    b.x = 10;
    b.y = 20;
    Point c = point_add(a, b);
    printf("point_add: (%d,%d)\n", c.x, c.y);

    /* Test 9: typedef int alias */
    uLong flags = combine_flags(100, 200);
    printf("combine_flags(100,200) = %lu\n", flags);

    return 0;
}
