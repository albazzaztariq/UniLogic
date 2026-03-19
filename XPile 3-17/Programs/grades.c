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

typedef struct Student Student;
typedef struct {
    int (*average)(Student*);
    int (*highest)(Student*);
    int (*lowest)(Student*);
    char* (*letter_grade)(Student*);
    int (*is_passing)(Student*);
} Student_VTable;

struct Student {
    Student_VTable* _vtable;
    const char* __type;
    char* name;
    int grade1;
    int grade2;
    int grade3;
    int grade4;
    int grade5;
};

void print_student(Student s);
int main(void);
int Student_average(Student* self);
int Student_highest(Student* self);
int Student_lowest(Student* self);
char* Student_letter_grade(Student* self);
int Student_is_passing(Student* self);
static Student_VTable _Student_vtable;

void print_student(Student s)
{
    printf("%s\n", s.name);
    printf("%s\n", "  Average:");
    printf("%d\n", s._vtable->average(&s));
    printf("%s\n", "  Grade:");
    printf("%s\n", s._vtable->letter_grade(&s));
    printf("%s\n", "  Highest:");
    printf("%d\n", s._vtable->highest(&s));
    printf("%s\n", "  Lowest:");
    printf("%d\n", s._vtable->lowest(&s));
    char* status = "PASS";
    if ((s._vtable->is_passing(&s) == 0)) {
        status = "FAIL";
    }
    printf("%s\n", "  Status:");
    printf("%s\n", status);
    printf("%s\n", "");
}

int main(void)
{
    Student s1;
    s1._vtable = &_Student_vtable;
    s1.__type = "Student";
    s1.name = "Alice";
    s1.grade1 = 95;
    s1.grade2 = 88;
    s1.grade3 = 92;
    s1.grade4 = 97;
    s1.grade5 = 90;
    Student s2;
    s2._vtable = &_Student_vtable;
    s2.__type = "Student";
    s2.name = "Bob";
    s2.grade1 = 72;
    s2.grade2 = 65;
    s2.grade3 = 78;
    s2.grade4 = 70;
    s2.grade5 = 68;
    Student s3;
    s3._vtable = &_Student_vtable;
    s3.__type = "Student";
    s3.name = "Charlie";
    s3.grade1 = 85;
    s3.grade2 = 90;
    s3.grade3 = 88;
    s3.grade4 = 82;
    s3.grade5 = 91;
    Student s4;
    s4._vtable = &_Student_vtable;
    s4.__type = "Student";
    s4.name = "Diana";
    s4.grade1 = 55;
    s4.grade2 = 60;
    s4.grade3 = 45;
    s4.grade4 = 58;
    s4.grade5 = 52;
    Student s5;
    s5._vtable = &_Student_vtable;
    s5.__type = "Student";
    s5.name = "Eve";
    s5.grade1 = 98;
    s5.grade2 = 95;
    s5.grade3 = 100;
    s5.grade4 = 97;
    s5.grade5 = 99;
    Student s6;
    s6._vtable = &_Student_vtable;
    s6.__type = "Student";
    s6.name = "Frank";
    s6.grade1 = 40;
    s6.grade2 = 35;
    s6.grade3 = 42;
    s6.grade4 = 38;
    s6.grade5 = 45;
    Student s7;
    s7._vtable = &_Student_vtable;
    s7.__type = "Student";
    s7.name = "Grace";
    s7.grade1 = 78;
    s7.grade2 = 82;
    s7.grade3 = 75;
    s7.grade4 = 80;
    s7.grade5 = 77;
    Student s8;
    s8._vtable = &_Student_vtable;
    s8.__type = "Student";
    s8.name = "Hank";
    s8.grade1 = 88;
    s8.grade2 = 91;
    s8.grade3 = 85;
    s8.grade4 = 90;
    s8.grade5 = 86;
    printf("%s\n", "=== Student Grade Report ===");
    printf("%s\n", "");
    print_student(s1);
    print_student(s2);
    print_student(s3);
    print_student(s4);
    print_student(s5);
    print_student(s6);
    print_student(s7);
    print_student(s8);
    printf("%s\n", "=== Class Statistics ===");
    int total_avg = (((((((s1._vtable->average(&s1) + s2._vtable->average(&s2)) + s3._vtable->average(&s3)) + s4._vtable->average(&s4)) + s5._vtable->average(&s5)) + s6._vtable->average(&s6)) + s7._vtable->average(&s7)) + s8._vtable->average(&s8));
    int cavg = (total_avg / 8);
    printf("%s\n", "Class average:");
    printf("%d\n", cavg);
    int best = s1._vtable->average(&s1);
    char* best_name = "Alice";
    if ((s2._vtable->average(&s2) > best)) {
        best = s2._vtable->average(&s2);
        best_name = "Bob";
    }
    if ((s3._vtable->average(&s3) > best)) {
        best = s3._vtable->average(&s3);
        best_name = "Charlie";
    }
    if ((s4._vtable->average(&s4) > best)) {
        best = s4._vtable->average(&s4);
        best_name = "Diana";
    }
    if ((s5._vtable->average(&s5) > best)) {
        best = s5._vtable->average(&s5);
        best_name = "Eve";
    }
    if ((s6._vtable->average(&s6) > best)) {
        best = s6._vtable->average(&s6);
        best_name = "Frank";
    }
    if ((s7._vtable->average(&s7) > best)) {
        best = s7._vtable->average(&s7);
        best_name = "Grace";
    }
    if ((s8._vtable->average(&s8) > best)) {
        best = s8._vtable->average(&s8);
        best_name = "Hank";
    }
    printf("%s\n", "Top student:");
    printf("%s\n", best_name);
    int failing = 0;
    if ((s1._vtable->is_passing(&s1) == 0)) {
        failing = (failing + 1);
    }
    if ((s2._vtable->is_passing(&s2) == 0)) {
        failing = (failing + 1);
    }
    if ((s3._vtable->is_passing(&s3) == 0)) {
        failing = (failing + 1);
    }
    if ((s4._vtable->is_passing(&s4) == 0)) {
        failing = (failing + 1);
    }
    if ((s5._vtable->is_passing(&s5) == 0)) {
        failing = (failing + 1);
    }
    if ((s6._vtable->is_passing(&s6) == 0)) {
        failing = (failing + 1);
    }
    if ((s7._vtable->is_passing(&s7) == 0)) {
        failing = (failing + 1);
    }
    if ((s8._vtable->is_passing(&s8) == 0)) {
        failing = (failing + 1);
    }
    printf("%s\n", "Failing students:");
    printf("%d\n", failing);
    return 0;
}

int Student_average(Student* self)
{
    int sum = ((((self->grade1 + self->grade2) + self->grade3) + self->grade4) + self->grade5);
    return (sum / 5);
}

int Student_highest(Student* self)
{
    int best = self->grade1;
    if ((self->grade2 > best)) {
        best = self->grade2;
    }
    if ((self->grade3 > best)) {
        best = self->grade3;
    }
    if ((self->grade4 > best)) {
        best = self->grade4;
    }
    if ((self->grade5 > best)) {
        best = self->grade5;
    }
    return best;
}

int Student_lowest(Student* self)
{
    int worst = self->grade1;
    if ((self->grade2 < worst)) {
        worst = self->grade2;
    }
    if ((self->grade3 < worst)) {
        worst = self->grade3;
    }
    if ((self->grade4 < worst)) {
        worst = self->grade4;
    }
    if ((self->grade5 < worst)) {
        worst = self->grade5;
    }
    return worst;
}

char* Student_letter_grade(Student* self)
{
    int avg = ((((self->grade1 + self->grade2) + self->grade3) + self->grade4) + self->grade5);
    avg = (avg / 5);
    if ((avg >= 90)) {
        return "A";
    }
    if ((avg >= 80)) {
        return "B";
    }
    if ((avg >= 70)) {
        return "C";
    }
    if ((avg >= 60)) {
        return "D";
    }
    return "F";
}

int Student_is_passing(Student* self)
{
    int avg = ((((self->grade1 + self->grade2) + self->grade3) + self->grade4) + self->grade5);
    avg = (avg / 5);
    return (avg >= 60);
}

static Student_VTable _Student_vtable = {(void*)Student_average, (void*)Student_highest, (void*)Student_lowest, (void*)Student_letter_grade, (void*)Student_is_passing};

