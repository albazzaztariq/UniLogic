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

typedef struct {
    int next;
    int prev;
    int child;
    int type_c;
    char* valuestring;
    int valueint;
    float valuedouble;
    char* string_c;
} cJSON;

typedef struct {
} cJSON_Hooks;

typedef struct {
    char* json;
    int position;
} error_c;

typedef struct {
} internal_hooks;

typedef struct {
    char* content;
    int length;
    int offset;
    int depth;
    int hooks;
} parse_buffer;

typedef struct {
    char* buffer;
    int length;
    int offset;
    int depth;
    int noalloc;
    int format;
    int hooks;
} printbuffer;

int global_error = 0;
int global_hooks = 0;
char* cJSON_GetErrorPtr(void);
char* cJSON_GetStringValue(int item);
float cJSON_GetNumberValue(int item);
char* cJSON_Version(void);
int case_insensitive_strcmp(char* string1, char* string2);
char* cJSON_strdup(char* string_c, int hooks);
void cJSON_InitHooks(int hooks);
int cJSON_New_Item(int hooks);
void cJSON_Delete(int item);
int get_decimal_point(void);
int parse_number(int item, int input_buffer);
float cJSON_SetNumberHelper(int object_c, float number);
char* cJSON_SetValuestring(int object_c, char* valuestring);
char* ensure(int p, int needed);
void update_offset(int buffer);
int compare_double(float a, float b);
int print_number(int item, int output_buffer);
int parse_hex4(char* input);
int utf16_literal_to_utf8(char* input_pointer, char* input_end, char* output_pointer);
int parse_string(int item, int input_buffer);
int print_string_ptr(char* input, int output_buffer);
int print_string(int item, int p);
int buffer_skip_whitespace(int buffer);
int skip_utf8_bom(int buffer);
int cJSON_ParseWithOpts(char* value, char* return_parse_end, int require_null_terminated);
int cJSON_ParseWithLengthOpts(char* value, int buffer_length, char* return_parse_end, int require_null_terminated);
int cJSON_Parse(char* value);
int cJSON_ParseWithLength(char* value, int buffer_length);
char* print_c(int item, int format, int hooks);
char* cJSON_Print(int item);
char* cJSON_PrintUnformatted(int item);
char* cJSON_PrintBuffered(int item, int prebuffer, int fmt);
int cJSON_PrintPreallocated(int item, char* buffer, int length, int format);
int parse_value(int item, int input_buffer);
int print_value(int item, int output_buffer);
int parse_array(int item, int input_buffer);
int print_array(int item, int output_buffer);
int parse_object(int item, int input_buffer);
int print_object(int item, int output_buffer);
int cJSON_GetArraySize(int array_c);
int get_array_item(int array_c, int index);
int cJSON_GetArrayItem(int array_c, int index);
int get_object_item(int object_c, char* name, int case_sensitive);
int cJSON_GetObjectItem(int object_c, char* string_c);
int cJSON_GetObjectItemCaseSensitive(int object_c, char* string_c);
int cJSON_HasObjectItem(int object_c, char* string_c);
void suffix_object(int prev, int item);
int create_reference(int item, int hooks);
int add_item_to_array(int array_c, int item);
int cJSON_AddItemToArray(int array_c, int item);
void cast_away_const(void string_c);
int add_item_to_object(int object_c, char* string_c, int item, int hooks, int constant_key);
int cJSON_AddItemToObject(int object_c, char* string_c, int item);
int cJSON_AddItemToObjectCS(int object_c, char* string_c, int item);
int cJSON_AddItemReferenceToArray(int array_c, int item);
int cJSON_AddItemReferenceToObject(int object_c, char* string_c, int item);
int cJSON_AddNullToObject(int object_c, char* name);
int cJSON_AddTrueToObject(int object_c, char* name);
int cJSON_AddFalseToObject(int object_c, char* name);
int cJSON_AddBoolToObject(int object_c, char* name, int boolean);
int cJSON_AddNumberToObject(int object_c, char* name, float number);
int cJSON_AddStringToObject(int object_c, char* name, char* string_c);
int cJSON_AddRawToObject(int object_c, char* name, char* raw);
int cJSON_AddObjectToObject(int object_c, char* name);
int cJSON_AddArrayToObject(int object_c, char* name);
int cJSON_DetachItemViaPointer(int parent, int item);
int cJSON_DetachItemFromArray(int array_c, int which);
void cJSON_DeleteItemFromArray(int array_c, int which);
int cJSON_DetachItemFromObject(int object_c, char* string_c);
int cJSON_DetachItemFromObjectCaseSensitive(int object_c, char* string_c);
void cJSON_DeleteItemFromObject(int object_c, char* string_c);
void cJSON_DeleteItemFromObjectCaseSensitive(int object_c, char* string_c);
int cJSON_InsertItemInArray(int array_c, int which, int newitem);
int cJSON_ReplaceItemViaPointer(int parent, int item, int replacement);
int cJSON_ReplaceItemInArray(int array_c, int which, int newitem);
int replace_item_in_object(int object_c, char* string_c, int replacement, int case_sensitive);
int cJSON_ReplaceItemInObject(int object_c, char* string_c, int newitem);
int cJSON_ReplaceItemInObjectCaseSensitive(int object_c, char* string_c, int newitem);
int cJSON_CreateNull(void);
int cJSON_CreateTrue(void);
int cJSON_CreateFalse(void);
int cJSON_CreateBool(int boolean);
int cJSON_CreateNumber(float num);
int cJSON_CreateString(char* string_c);
int cJSON_CreateStringReference(char* string_c);
int cJSON_CreateObjectReference(int child);
int cJSON_CreateArrayReference(int child);
int cJSON_CreateRaw(char* raw);
int cJSON_CreateArray(void);
int cJSON_CreateObject(void);
int cJSON_CreateIntArray(int numbers, int count);
int cJSON_CreateFloatArray(float numbers, int count);
int cJSON_CreateDoubleArray(float numbers, int count);
int cJSON_CreateStringArray(char* strings, int count);
int cJSON_Duplicate(int item, int recurse);
int cJSON_Duplicate_rec(int item, int depth, int recurse);
void skip_oneline_comment(char* input);
void skip_multiline_comment(char* input);
void minify_string(char* input, char* output);
void cJSON_Minify(char* json);
int cJSON_IsInvalid(int item);
int cJSON_IsFalse(int item);
int cJSON_IsTrue(int item);
int cJSON_IsBool(int item);
int cJSON_IsNull(int item);
int cJSON_IsNumber(int item);
int cJSON_IsString(int item);
int cJSON_IsArray(int item);
int cJSON_IsObject(int item);
int cJSON_IsRaw(int item);
int cJSON_Compare(int a, int b, int case_sensitive);
void cJSON_malloc(int size_c);
void cJSON_free(void object_c);

char* cJSON_GetErrorPtr(void)
{
    return ((int)(global_error.json + global_error.position));
}

char* cJSON_GetStringValue(int item)
{
    if ((!cJSON_IsString(item))) {
        return ((int)0);
    }
    return item.valuestring;
}

float cJSON_GetNumberValue(int item)
{
    if ((!cJSON_IsNumber(item))) {
        return ((int)0);
    }
    return item.valuedouble;
}

char* cJSON_Version(void)
{
    int version = 0;
    sprintf(version, "%i.%i.%i", 1, 7, 19);
    return version;
}

int case_insensitive_strcmp(char* string1, char* string2)
{
    if (((strcmp(string1, ((int)0)) == 0) || (strcmp(string2, ((int)0)) == 0))) {
        return 1;
    }
    if ((strcmp(string1, string2) == 0)) {
        return 0;
    }
    while ((0 == 0)) {
        if ((strcmp(string1, 0) == 0)) {
            return 0;
        }
        ((int)string1);
        string2 = __ul_strcat(string2, 1);
    }
    return (0 - 0);
}

char* cJSON_strdup(char* string_c, int hooks)
{
    int length = 0;
    char* copy = ((int)0);
    if ((strcmp(string_c, ((int)0)) == 0)) {
        return ((int)0);
    }
    length = (sizeof(((int)string_c)) + 0);
    copy = ((int)hooks._vtable->allocate(&hooks, length));
    if ((strcmp(copy, ((int)0)) == 0)) {
        return ((int)0);
    }
    memcpy(copy, string_c, length);
    return copy;
}

void cJSON_InitHooks(int hooks)
{
    if ((hooks == ((int)0))) {
        global_hooks.allocate = 0;
        global_hooks.deallocate = 0;
        global_hooks.reallocate = 0;
        return;
    }
    global_hooks.allocate = 0;
    if ((hooks.malloc_fn != ((int)0))) {
        global_hooks.allocate = hooks.malloc_fn;
    }
    global_hooks.deallocate = 0;
    if ((hooks.free_fn != ((int)0))) {
        global_hooks.deallocate = hooks.free_fn;
    }
    global_hooks.reallocate = ((int)0);
    if (((global_hooks.allocate == 0) && (global_hooks.deallocate == 0))) {
        global_hooks.reallocate = 0;
    }
}

int cJSON_New_Item(int hooks)
{
    int node = ((int)hooks._vtable->allocate(&hooks, 0));
    if (node) {
        memset(node, 0, 0);
    }
    return node;
}

void cJSON_Delete(int item)
{
    int next = ((int)0);
    while ((item != ((int)0))) {
        next = item.next;
        if (((!(item.type & 256)) && (item.child != ((int)0)))) {
            cJSON_Delete(item.child);
        }
        if (((!(item.type & 256)) && (item.valuestring != ((int)0)))) {
            global_hooks._vtable->deallocate(&global_hooks, item.valuestring);
            item.valuestring = ((int)0);
        }
        if (((!(item.type & 512)) && (item.string != ((int)0)))) {
            global_hooks._vtable->deallocate(&global_hooks, item.string);
            item.string = ((int)0);
        }
        global_hooks._vtable->deallocate(&global_hooks, item);
        item = next;
    }
}

int get_decimal_point(void)
{
    return 46;
}

int parse_number(int item, int input_buffer)
{
    float number = 0;
    char* after_end = ((int)0);
    char* number_c_string = "";
    int decimal_point = get_decimal_point();
    int i = 0;
    int number_string_length = 0;
    int has_decimal_point = ((int)0);
    if (((input_buffer == ((int)0)) || (input_buffer.content == ((int)0)))) {
        return ((int)0);
    }
    i = 0;
    while (((input_buffer != ((int)0)) && ((input_buffer.offset + i) < input_buffer.length))) {
        if ((((((((((((((((input_buffer.content + input_buffer.offset)[i] == 48) || ((input_buffer.content + input_buffer.offset)[i] == 49)) || ((input_buffer.content + input_buffer.offset)[i] == 50)) || ((input_buffer.content + input_buffer.offset)[i] == 51)) || ((input_buffer.content + input_buffer.offset)[i] == 52)) || ((input_buffer.content + input_buffer.offset)[i] == 53)) || ((input_buffer.content + input_buffer.offset)[i] == 54)) || ((input_buffer.content + input_buffer.offset)[i] == 55)) || ((input_buffer.content + input_buffer.offset)[i] == 56)) || ((input_buffer.content + input_buffer.offset)[i] == 57)) || ((input_buffer.content + input_buffer.offset)[i] == 43)) || ((input_buffer.content + input_buffer.offset)[i] == 45)) || ((input_buffer.content + input_buffer.offset)[i] == 101)) || ((input_buffer.content + input_buffer.offset)[i] == 69))) {
            number_string_length = (number_string_length + 1);
        } else {
            if (((input_buffer.content + input_buffer.offset)[i] == 46)) {
                number_string_length = (number_string_length + 1);
                has_decimal_point = ((int)1);
            }
        }
        i = (i + 1);
    }
    number_c_string = ((int)input_buffer.hooks._vtable->allocate(&input_buffer.hooks, (number_string_length + 1)));
    if ((strcmp(number_c_string, ((int)0)) == 0)) {
        return ((int)0);
    }
    memcpy(number_c_string, (input_buffer.content + input_buffer.offset), number_string_length);
    str_char_at(number_c_string, number_string_length) = 0;
    if (has_decimal_point) {
        i = 0;
        while ((i < number_string_length)) {
            if ((str_char_at(number_c_string, i) == 46)) {
                str_char_at(number_c_string, i) = decimal_point;
            }
            i = (i + 1);
        }
    }
    number = 0;
    if ((strcmp(number_c_string, after_end) == 0)) {
        input_buffer.hooks._vtable->deallocate(&input_buffer.hooks, number_c_string);
        return ((int)0);
    }
    item.valuedouble = number;
    if ((number >= 2147483647)) {
        item.valueint = 2147483647;
    } else {
        if ((number <= ((int)((-2147483647) - 1)))) {
            item.valueint = ((-2147483647) - 1);
        } else {
            item.valueint = ((int)number);
        }
    }
    item.type = (1 << 3);
    input_buffer.offset = (input_buffer.offset + ((int)(after_end - number_c_string)));
    input_buffer.hooks._vtable->deallocate(&input_buffer.hooks, number_c_string);
    return ((int)1);
}

float cJSON_SetNumberHelper(int object_c, float number)
{
    if ((object_c == ((int)0))) {
        return ((int)0);
    }
    if ((number >= 2147483647)) {
        object_c.valueint = 2147483647;
    } else {
        if ((number <= ((int)((-2147483647) - 1)))) {
            object_c.valueint = ((-2147483647) - 1);
        } else {
            object_c.valueint = ((int)number);
        }
    }
    object_c.valuedouble = number;
    return object_c.valuedouble;
}

char* cJSON_SetValuestring(int object_c, char* valuestring)
{
    char* copy = ((int)0);
    int v1_len = 0;
    int v2_len = 0;
    if ((((object_c == ((int)0)) || (!(object_c.type & (1 << 4)))) || (object_c.type & 256))) {
        return ((int)0);
    }
    if (((object_c.valuestring == ((int)0)) || (strcmp(valuestring, ((int)0)) == 0))) {
        return ((int)0);
    }
    v1_len = sizeof(valuestring);
    v2_len = sizeof(object_c.valuestring);
    if ((v1_len <= v2_len)) {
        if ((!((strcmp(__ul_strcat(valuestring, v1_len), object_c.valuestring) < 0) || (strcmp((object_c.valuestring + v2_len), valuestring) < 0)))) {
            return ((int)0);
        }
        strcpy(object_c.valuestring, valuestring);
        return object_c.valuestring;
    }
    copy = ((int)cJSON_strdup(((int)valuestring), global_hooks));
    if ((strcmp(copy, ((int)0)) == 0)) {
        return ((int)0);
    }
    if ((object_c.valuestring != ((int)0))) {
        cJSON_free(object_c.valuestring);
    }
    object_c.valuestring = copy;
    return copy;
}

char* ensure(int p, int needed)
{
    char* newbuffer = ((int)0);
    int newsize = 0;
    if (((p == ((int)0)) || (p.buffer == ((int)0)))) {
        return ((int)0);
    }
    if (((p.length > 0) && (p.offset >= p.length))) {
        return ((int)0);
    }
    if ((needed > 2147483647)) {
        return ((int)0);
    }
    needed = (needed + (p.offset + 1));
    if ((needed <= p.length)) {
        return (p.buffer + p.offset);
    }
    if (p.noalloc) {
        return ((int)0);
    }
    if ((needed > (2147483647 / 2))) {
        if ((needed <= 2147483647)) {
            newsize = 2147483647;
        } else {
            return ((int)0);
        }
    } else {
        newsize = (needed * 2);
    }
    if ((p.hooks.reallocate != ((int)0))) {
        newbuffer = ((int)p.hooks._vtable->reallocate(&p.hooks, p.buffer, newsize));
        if ((strcmp(newbuffer, ((int)0)) == 0)) {
            p.hooks._vtable->deallocate(&p.hooks, p.buffer);
            p.length = 0;
            p.buffer = ((int)0);
            return ((int)0);
        }
    } else {
        newbuffer = ((int)p.hooks._vtable->allocate(&p.hooks, newsize));
        if ((!newbuffer)) {
            p.hooks._vtable->deallocate(&p.hooks, p.buffer);
            p.length = 0;
            p.buffer = ((int)0);
            return ((int)0);
        }
        memcpy(newbuffer, p.buffer, (p.offset + 1));
        p.hooks._vtable->deallocate(&p.hooks, p.buffer);
    }
    p.length = newsize;
    p.buffer = newbuffer;
    return __ul_strcat(newbuffer, p.offset);
}

void update_offset(int buffer)
{
    char* buffer_pointer = ((int)0);
    if (((buffer == ((int)0)) || (buffer.buffer == ((int)0)))) {
        return;
    }
    buffer_pointer = (buffer.buffer + buffer.offset);
    buffer.offset = (buffer.offset + sizeof(((int)buffer_pointer)));
}

int compare_double(float a, float b)
{
    float maxVal = 0;
    return (0 <= (maxVal * ((int)2.220446049250313e-16)));
}

int print_number(int item, int output_buffer)
{
    char* output_pointer = ((int)0);
    float d = item.valuedouble;
    int length = 0;
    int i = 0;
    int number_buffer[] = {0};
    int decimal_point = get_decimal_point();
    float test = 0.0;
    if ((output_buffer == ((int)0))) {
        return ((int)0);
    }
    if ((0 || (0 == (256 | 1024)))) {
        length = 0;
    } else {
        if ((d == ((int)item.valueint))) {
            length = 0;
        } else {
            length = 0;
            if (((sscanf(((int)number_buffer), "%lg", test) != 1) || (!compare_double(((int)test), d)))) {
                length = 0;
            }
        }
    }
    if (((length < 0) || (length > ((int)(0 - 1))))) {
        return ((int)0);
    }
    output_pointer = ensure(output_buffer, (((int)length) + 0));
    if ((strcmp(output_pointer, ((int)0)) == 0)) {
        return ((int)0);
    }
    i = 0;
    while ((i < ((int)length))) {
        if ((number_buffer[i] == decimal_point)) {
            str_char_at(output_pointer, i) = 46;
            continue;
        }
        str_char_at(output_pointer, i) = number_buffer[i];
        i = (i + 1);
    }
    str_char_at(output_pointer, i) = 0;
    output_buffer.offset = (output_buffer.offset + ((int)length));
    return ((int)1);
}

int parse_hex4(char* input)
{
    int h = 0;
    int i = 0;
    i = 0;
    while ((i < 4)) {
        if (((str_char_at(input, i) >= 48) && (str_char_at(input, i) <= 57))) {
            h = (h + (((int)str_char_at(input, i)) - 48));
        } else {
            if (((str_char_at(input, i) >= 65) && (str_char_at(input, i) <= 70))) {
                h = (h + ((((int)10) + str_char_at(input, i)) - 65));
            } else {
                if (((str_char_at(input, i) >= 97) && (str_char_at(input, i) <= 102))) {
                    h = (h + ((((int)10) + str_char_at(input, i)) - 97));
                } else {
                    return 0;
                }
            }
        }
        if ((i < 3)) {
            h = (h << 4);
        }
        i = (i + 1);
    }
    return h;
}

int utf16_literal_to_utf8(char* input_pointer, char* input_end, char* output_pointer)
{
    int codepoint = 0;
    int first_code = 0;
    char* first_sequence = input_pointer;
    int utf8_length = 0;
    int utf8_position = 0;
    int sequence_length = 0;
    int first_byte_mark = 0;
    if (((input_end - first_sequence) < 6)) {
    }
    first_code = parse_hex4(__ul_strcat(first_sequence, 2));
    if (((first_code >= 56320) && (first_code <= 57343))) {
    }
    if (((first_code >= 55296) && (first_code <= 56319))) {
        char* second_sequence = __ul_strcat(first_sequence, 6);
        int second_code = 0;
        sequence_length = 12;
        if (((input_end - second_sequence) < 6)) {
        }
        if (((str_char_at(second_sequence, 0) != 92) || (str_char_at(second_sequence, 1) != 117))) {
        }
        second_code = parse_hex4(__ul_strcat(second_sequence, 2));
        if (((second_code < 56320) || (second_code > 57343))) {
        }
        codepoint = (65536 + (((first_code & 1023) << 10) | (second_code & 1023)));
    } else {
        sequence_length = 6;
        codepoint = first_code;
    }
    if ((codepoint < 128)) {
        utf8_length = 1;
    } else {
        if ((codepoint < 2048)) {
            utf8_length = 2;
            first_byte_mark = 192;
        } else {
            if ((codepoint < 65536)) {
                utf8_length = 3;
                first_byte_mark = 224;
            } else {
                if ((codepoint <= 1114111)) {
                    utf8_length = 4;
                    first_byte_mark = 240;
                }
            }
        }
    }
    utf8_position = ((int)(utf8_length - 1));
    while ((utf8_position > 0)) {
        str_char_at(output_pointer, utf8_position) = ((int)((codepoint | 128) & 191));
        codepoint = (codepoint >> 6);
        utf8_position = (utf8_position - 1);
    }
    if ((utf8_length > 1)) {
        str_char_at(output_pointer, 0) = ((int)((codepoint | first_byte_mark) & 255));
    } else {
        str_char_at(output_pointer, 0) = ((int)(codepoint & 127));
    }
    output_pointer = __ul_strcat(output_pointer, utf8_length);
    return sequence_length;
    return 0;
}

int parse_string(int item, int input_buffer)
{
    char* input_pointer = ((input_buffer.content + input_buffer.offset) + 1);
    char* input_end = ((input_buffer.content + input_buffer.offset) + 1);
    char* output_pointer = ((int)0);
    char* output = ((int)0);
    if (((input_buffer.content + input_buffer.offset)[0] != 34)) {
    }
    int allocation_length = 0;
    int skipped_bytes = 0;
    while (((((int)(input_end - input_buffer.content)) < input_buffer.length) && (strcmp(input_end, 34) != 0))) {
        if ((str_char_at(input_end, 0) == 92)) {
            if ((((int)(__ul_strcat(input_end, 1) - input_buffer.content)) >= input_buffer.length)) {
            }
            skipped_bytes = (skipped_bytes + 1);
            input_end = __ul_strcat(input_end, 1);
        }
        input_end = __ul_strcat(input_end, 1);
    }
    if (((((int)(input_end - input_buffer.content)) >= input_buffer.length) || (strcmp(input_end, 34) != 0))) {
    }
    allocation_length = (((int)(input_end - (input_buffer.content + input_buffer.offset))) - skipped_bytes);
    output = ((int)input_buffer.hooks._vtable->allocate(&input_buffer.hooks, (allocation_length + 0)));
    if ((strcmp(output, ((int)0)) == 0)) {
    }
    output_pointer = output;
    while ((strcmp(input_pointer, input_end) < 0)) {
        if ((strcmp(input_pointer, 92) != 0)) {
            output_pointer = input_pointer;
        } else {
            int sequence_length = 2;
            if (((input_end - input_pointer) < 1)) {
            }
            if ((str_char_at(input_pointer, 1) == 98)) {
                output_pointer = 8;
            } else {
                if ((str_char_at(input_pointer, 1) == 102)) {
                    output_pointer = 12;
                } else {
                    if ((str_char_at(input_pointer, 1) == 110)) {
                        output_pointer = 10;
                    } else {
                        if ((str_char_at(input_pointer, 1) == 114)) {
                            output_pointer = 13;
                        } else {
                            if ((str_char_at(input_pointer, 1) == 116)) {
                                output_pointer = 9;
                            } else {
                                if ((((str_char_at(input_pointer, 1) == 34) || (str_char_at(input_pointer, 1) == 92)) || (str_char_at(input_pointer, 1) == 47))) {
                                    output_pointer = str_char_at(input_pointer, 1);
                                } else {
                                    if ((str_char_at(input_pointer, 1) == 117)) {
                                        sequence_length = utf16_literal_to_utf8(input_pointer, input_end, output_pointer);
                                        if ((sequence_length == 0)) {
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            input_pointer = __ul_strcat(input_pointer, sequence_length);
        }
    }
    output_pointer = 0;
    item.type = (1 << 4);
    item.valuestring = ((int)output);
    input_buffer.offset = ((int)(input_end - input_buffer.content));
    input_buffer.offset = (input_buffer.offset + 1);
    return ((int)1);
    if ((strcmp(output, ((int)0)) != 0)) {
        input_buffer.hooks._vtable->deallocate(&input_buffer.hooks, output);
        output = ((int)0);
    }
    if ((strcmp(input_pointer, ((int)0)) != 0)) {
        input_buffer.offset = ((int)(input_pointer - input_buffer.content));
    }
    return ((int)0);
}

int print_string_ptr(char* input, int output_buffer)
{
    char* input_pointer = ((int)0);
    char* output = ((int)0);
    char* output_pointer = ((int)0);
    int output_length = 0;
    int escape_characters = 0;
    if ((output_buffer == ((int)0))) {
        return ((int)0);
    }
    if ((strcmp(input, ((int)0)) == 0)) {
        output = ensure(output_buffer, 0);
        if ((strcmp(output, ((int)0)) == 0)) {
            return ((int)0);
        }
        strcpy(((int)output), "\"\"");
        return ((int)1);
    }
    input_pointer = input;
    while (input_pointer) {
        if ((((((((strcmp(input_pointer, 34) == 0) || (strcmp(input_pointer, 92) == 0)) || (strcmp(input_pointer, 8) == 0)) || (strcmp(input_pointer, 12) == 0)) || (strcmp(input_pointer, 10) == 0)) || (strcmp(input_pointer, 13) == 0)) || (strcmp(input_pointer, 9) == 0))) {
            escape_characters = (escape_characters + 1);
        } else {
            if ((strcmp(input_pointer, 32) < 0)) {
                escape_characters = (escape_characters + 5);
            }
        }
        input_pointer = __ul_strcat(input_pointer, 1);
    }
    output_length = (((int)(input_pointer - input)) + escape_characters);
    output = ensure(output_buffer, (output_length + 0));
    if ((strcmp(output, ((int)0)) == 0)) {
        return ((int)0);
    }
    if ((escape_characters == 0)) {
        str_char_at(output, 0) = 34;
        memcpy(__ul_strcat(output, 1), input, output_length);
        str_char_at(output, (output_length + 1)) = 34;
        str_char_at(output, (output_length + 2)) = 0;
        return ((int)1);
    }
    str_char_at(output, 0) = 34;
    output_pointer = __ul_strcat(output, 1);
    input_pointer = input;
    while ((strcmp(input_pointer, 0) != 0)) {
        if ((((strcmp(input_pointer, 31) > 0) && (strcmp(input_pointer, 34) != 0)) && (strcmp(input_pointer, 92) != 0))) {
            output_pointer = input_pointer;
        } else {
            output_pointer = 92;
            if ((strcmp(input_pointer, 92) == 0)) {
                output_pointer = 92;
            } else {
                if ((strcmp(input_pointer, 34) == 0)) {
                    output_pointer = 34;
                } else {
                    if ((strcmp(input_pointer, 8) == 0)) {
                        output_pointer = 98;
                    } else {
                        if ((strcmp(input_pointer, 12) == 0)) {
                            output_pointer = 102;
                        } else {
                            if ((strcmp(input_pointer, 10) == 0)) {
                                output_pointer = 110;
                            } else {
                                if ((strcmp(input_pointer, 13) == 0)) {
                                    output_pointer = 114;
                                } else {
                                    if ((strcmp(input_pointer, 9) == 0)) {
                                        output_pointer = 116;
                                    } else {
                                        sprintf(((int)output_pointer), "u%04x", input_pointer);
                                        output_pointer = __ul_strcat(output_pointer, 4);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        ((int)input_pointer);
        output_pointer = __ul_strcat(output_pointer, 1);
    }
    str_char_at(output, (output_length + 1)) = 34;
    str_char_at(output, (output_length + 2)) = 0;
    return ((int)1);
}

int print_string(int item, int p)
{
    return print_string_ptr(((int)item.valuestring), p);
}

int buffer_skip_whitespace(int buffer)
{
    if (((buffer == ((int)0)) || (buffer.content == ((int)0)))) {
        return ((int)0);
    }
    if ((!((buffer != ((int)0)) && ((buffer.offset + 0) < buffer.length)))) {
        return buffer;
    }
    while ((((buffer != ((int)0)) && ((buffer.offset + 0) < buffer.length)) && ((buffer.content + buffer.offset)[0] <= 32))) {
        buffer.offset = (buffer.offset + 1);
    }
    if ((buffer.offset == buffer.length)) {
        buffer.offset = (buffer.offset - 1);
    }
    return buffer;
}

int skip_utf8_bom(int buffer)
{
    if ((((buffer == ((int)0)) || (buffer.content == ((int)0))) || (buffer.offset != 0))) {
        return ((int)0);
    }
    if ((((buffer != ((int)0)) && ((buffer.offset + 4) < buffer.length)) && (0 == 0))) {
        buffer.offset = (buffer.offset + 3);
    }
    return buffer;
}

int cJSON_ParseWithOpts(char* value, char* return_parse_end, int require_null_terminated)
{
    int buffer_length = 0;
    if ((strcmp(((int)0), value) == 0)) {
        return ((int)0);
    }
    buffer_length = (sizeof(value) + 0);
    return cJSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

int cJSON_ParseWithLengthOpts(char* value, int buffer_length, char* return_parse_end, int require_null_terminated)
{
    int buffer = /* unknown expr ArrayLiteral */;
    int item = ((int)0);
    global_error.json = ((int)0);
    global_error.position = 0;
    if (((strcmp(value, ((int)0)) == 0) || (0 == buffer_length))) {
    }
    buffer.content = ((int)value);
    buffer.length = buffer_length;
    buffer.offset = 0;
    buffer.hooks = global_hooks;
    item = cJSON_New_Item(global_hooks);
    if ((item == ((int)0))) {
    }
    if ((!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(buffer))))) {
    }
    if (require_null_terminated) {
        buffer_skip_whitespace(buffer);
        if (((buffer.offset >= buffer.length) || ((buffer.content + buffer.offset)[0] != 0))) {
        }
    }
    if (return_parse_end) {
        return_parse_end = ((int)(buffer.content + buffer.offset));
    }
    return item;
    if ((item != ((int)0))) {
        cJSON_Delete(item);
    }
    if ((strcmp(value, ((int)0)) != 0)) {
        int local_error = 0;
        local_error.json = ((int)value);
        local_error.position = 0;
        if ((buffer.offset < buffer.length)) {
            local_error.position = buffer.offset;
        } else {
            if ((buffer.length > 0)) {
                local_error.position = (buffer.length - 1);
            }
        }
        if ((strcmp(return_parse_end, ((int)0)) != 0)) {
            return_parse_end = (((int)local_error.json) + local_error.position);
        }
        global_error = local_error;
    }
    return ((int)0);
}

int cJSON_Parse(char* value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}

int cJSON_ParseWithLength(char* value, int buffer_length)
{
    return cJSON_ParseWithLengthOpts(value, buffer_length, 0, 0);
}

char* print_c(int item, int format, int hooks)
{
    int default_buffer_size = 256;
    int buffer = 0;
    char* printed = ((int)0);
    memset(buffer, 0, 0);
    buffer.buffer = ((int)hooks._vtable->allocate(&hooks, default_buffer_size));
    buffer.length = default_buffer_size;
    buffer.format = format;
    buffer.hooks = hooks;
    if ((buffer.buffer == ((int)0))) {
    }
    if ((!print_value(item, buffer))) {
    }
    update_offset(buffer);
    if ((hooks.reallocate != ((int)0))) {
        printed = ((int)hooks._vtable->reallocate(&hooks, buffer.buffer, (buffer.offset + 1)));
        if ((strcmp(printed, ((int)0)) == 0)) {
        }
        buffer.buffer = ((int)0);
    } else {
        printed = ((int)hooks._vtable->allocate(&hooks, (buffer.offset + 1)));
        if ((strcmp(printed, ((int)0)) == 0)) {
        }
        memcpy(printed, buffer.buffer, buffer.length);
        str_char_at(printed, buffer.offset) = 0;
        hooks._vtable->deallocate(&hooks, buffer.buffer);
        buffer.buffer = ((int)0);
    }
    return printed;
    if ((buffer.buffer != ((int)0))) {
        hooks._vtable->deallocate(&hooks, buffer.buffer);
        buffer.buffer = ((int)0);
    }
    if ((strcmp(printed, ((int)0)) != 0)) {
        hooks._vtable->deallocate(&hooks, printed);
        printed = ((int)0);
    }
    return ((int)0);
}

char* cJSON_Print(int item)
{
    return ((int)print_c(item, ((int)1), global_hooks));
}

char* cJSON_PrintUnformatted(int item)
{
    return ((int)print_c(item, ((int)0), global_hooks));
}

char* cJSON_PrintBuffered(int item, int prebuffer, int fmt)
{
    int p = /* unknown expr ArrayLiteral */;
    if ((prebuffer < 0)) {
        return ((int)0);
    }
    p.buffer = ((int)global_hooks._vtable->allocate(&global_hooks, ((int)prebuffer)));
    if ((!p.buffer)) {
        return ((int)0);
    }
    p.length = ((int)prebuffer);
    p.offset = 0;
    p.noalloc = ((int)0);
    p.format = fmt;
    p.hooks = global_hooks;
    if ((!print_value(item, p))) {
        global_hooks._vtable->deallocate(&global_hooks, p.buffer);
        p.buffer = ((int)0);
        return ((int)0);
    }
    return ((int)p.buffer);
}

int cJSON_PrintPreallocated(int item, char* buffer, int length, int format)
{
    int p = /* unknown expr ArrayLiteral */;
    if (((length < 0) || (strcmp(buffer, ((int)0)) == 0))) {
        return ((int)0);
    }
    p.buffer = ((int)buffer);
    p.length = ((int)length);
    p.offset = 0;
    p.noalloc = ((int)1);
    p.format = format;
    p.hooks = global_hooks;
    return print_value(item, p);
}

int parse_value(int item, int input_buffer)
{
    if (((input_buffer == ((int)0)) || (input_buffer.content == ((int)0)))) {
        return ((int)0);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 4) <= input_buffer.length)) && (0 == 0))) {
        item.type = (1 << 2);
        input_buffer.offset = (input_buffer.offset + 4);
        return ((int)1);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 5) <= input_buffer.length)) && (0 == 0))) {
        item.type = (1 << 0);
        input_buffer.offset = (input_buffer.offset + 5);
        return ((int)1);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 4) <= input_buffer.length)) && (0 == 0))) {
        item.type = (1 << 1);
        item.valueint = 1;
        input_buffer.offset = (input_buffer.offset + 4);
        return ((int)1);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 34))) {
        return parse_string(item, input_buffer);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && (((input_buffer.content + input_buffer.offset)[0] == 45) || (((input_buffer.content + input_buffer.offset)[0] >= 48) && ((input_buffer.content + input_buffer.offset)[0] <= 57))))) {
        return parse_number(item, input_buffer);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 91))) {
        return parse_array(item, input_buffer);
    }
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 123))) {
        return parse_object(item, input_buffer);
    }
    return ((int)0);
}

int print_value(int item, int output_buffer)
{
    char* output = ((int)0);
    if (((item == ((int)0)) || (output_buffer == ((int)0)))) {
        return ((int)0);
    }
    if (((item.type & 255) == (1 << 2))) {
        output = ensure(output_buffer, 5);
        if ((strcmp(output, ((int)0)) == 0)) {
            return ((int)0);
        }
        strcpy(((int)output), "null");
        return ((int)1);
    } else {
        if (((item.type & 255) == (1 << 0))) {
            output = ensure(output_buffer, 6);
            if ((strcmp(output, ((int)0)) == 0)) {
                return ((int)0);
            }
            strcpy(((int)output), "false");
            return ((int)1);
        } else {
            if (((item.type & 255) == (1 << 1))) {
                output = ensure(output_buffer, 5);
                if ((strcmp(output, ((int)0)) == 0)) {
                    return ((int)0);
                }
                strcpy(((int)output), "true");
                return ((int)1);
            } else {
                if (((item.type & 255) == (1 << 3))) {
                    return print_number(item, output_buffer);
                } else {
                    if (((item.type & 255) == (1 << 7))) {
                        int raw_length = 0;
                        if ((item.valuestring == ((int)0))) {
                            return ((int)0);
                        }
                        raw_length = (sizeof(item.valuestring) + 0);
                        output = ensure(output_buffer, raw_length);
                        if ((strcmp(output, ((int)0)) == 0)) {
                            return ((int)0);
                        }
                        memcpy(output, item.valuestring, raw_length);
                        return ((int)1);
                    } else {
                        if (((item.type & 255) == (1 << 4))) {
                            return print_string(item, output_buffer);
                        } else {
                            if (((item.type & 255) == (1 << 5))) {
                                return print_array(item, output_buffer);
                            } else {
                                if (((item.type & 255) == (1 << 6))) {
                                    return print_object(item, output_buffer);
                                } else {
                                    return ((int)0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int parse_array(int item, int input_buffer)
{
    int head = ((int)0);
    int current_item = ((int)0);
    if ((input_buffer.depth >= 1000)) {
        return ((int)0);
    }
    input_buffer.depth = (input_buffer.depth + 1);
    if (((input_buffer.content + input_buffer.offset)[0] != 91)) {
    }
    input_buffer.offset = (input_buffer.offset + 1);
    buffer_skip_whitespace(input_buffer);
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 93))) {
    }
    if ((!((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)))) {
        input_buffer.offset = (input_buffer.offset - 1);
    }
    input_buffer.offset = (input_buffer.offset - 1);
    do {
        int new_item = cJSON_New_Item(input_buffer.hooks);
        if ((new_item == ((int)0))) {
        }
        if ((head == ((int)0))) {
            head = new_item;
            current_item = head;
        } else {
            current_item.next = new_item;
            new_item.prev = current_item;
            current_item = new_item;
        }
        input_buffer.offset = (input_buffer.offset + 1);
        buffer_skip_whitespace(input_buffer);
        if ((!parse_value(current_item, input_buffer))) {
        }
        buffer_skip_whitespace(input_buffer);
    } while ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 44)));
    if (((!((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length))) || ((input_buffer.content + input_buffer.offset)[0] != 93))) {
    }
    input_buffer.depth = (input_buffer.depth - 1);
    if ((head != ((int)0))) {
        head.prev = current_item;
    }
    item.type = (1 << 5);
    item.child = head;
    input_buffer.offset = (input_buffer.offset + 1);
    return ((int)1);
    if ((head != ((int)0))) {
        cJSON_Delete(head);
    }
    return ((int)0);
}

int print_array(int item, int output_buffer)
{
    char* output_pointer = ((int)0);
    int length = 0;
    int current_element = item.child;
    if ((output_buffer == ((int)0))) {
        return ((int)0);
    }
    if ((output_buffer.depth >= 1000)) {
        return ((int)0);
    }
    output_pointer = ensure(output_buffer, 1);
    if ((strcmp(output_pointer, ((int)0)) == 0)) {
        return ((int)0);
    }
    output_pointer = 91;
    output_buffer.offset = (output_buffer.offset + 1);
    output_buffer.depth = (output_buffer.depth + 1);
    while ((current_element != ((int)0))) {
        if ((!print_value(current_element, output_buffer))) {
            return ((int)0);
        }
        update_offset(output_buffer);
        if (current_element.next) {
            length = ((int)2);
            output_pointer = ensure(output_buffer, (length + 1));
            if ((strcmp(output_pointer, ((int)0)) == 0)) {
                return ((int)0);
            }
            output_pointer = 44;
            if (output_buffer.format) {
                output_pointer = 32;
            }
            output_pointer = 0;
            output_buffer.offset = (output_buffer.offset + length);
        }
        current_element = current_element.next;
    }
    output_pointer = ensure(output_buffer, 2);
    if ((strcmp(output_pointer, ((int)0)) == 0)) {
        return ((int)0);
    }
    output_pointer = 93;
    output_pointer = 0;
    output_buffer.depth = (output_buffer.depth - 1);
    return ((int)1);
}

int parse_object(int item, int input_buffer)
{
    int head = ((int)0);
    int current_item = ((int)0);
    if ((input_buffer.depth >= 1000)) {
        return ((int)0);
    }
    input_buffer.depth = (input_buffer.depth + 1);
    if (((!((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length))) || ((input_buffer.content + input_buffer.offset)[0] != 123))) {
    }
    input_buffer.offset = (input_buffer.offset + 1);
    buffer_skip_whitespace(input_buffer);
    if ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 125))) {
    }
    if ((!((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)))) {
        input_buffer.offset = (input_buffer.offset - 1);
    }
    input_buffer.offset = (input_buffer.offset - 1);
    do {
        int new_item = cJSON_New_Item(input_buffer.hooks);
        if ((new_item == ((int)0))) {
        }
        if ((head == ((int)0))) {
            head = new_item;
            current_item = head;
        } else {
            current_item.next = new_item;
            new_item.prev = current_item;
            current_item = new_item;
        }
        if ((!((input_buffer != ((int)0)) && ((input_buffer.offset + 1) < input_buffer.length)))) {
        }
        input_buffer.offset = (input_buffer.offset + 1);
        buffer_skip_whitespace(input_buffer);
        if ((!parse_string(current_item, input_buffer))) {
        }
        buffer_skip_whitespace(input_buffer);
        current_item.string = current_item.valuestring;
        current_item.valuestring = ((int)0);
        if (((!((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length))) || ((input_buffer.content + input_buffer.offset)[0] != 58))) {
        }
        input_buffer.offset = (input_buffer.offset + 1);
        buffer_skip_whitespace(input_buffer);
        if ((!parse_value(current_item, input_buffer))) {
        }
        buffer_skip_whitespace(input_buffer);
    } while ((((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length)) && ((input_buffer.content + input_buffer.offset)[0] == 44)));
    if (((!((input_buffer != ((int)0)) && ((input_buffer.offset + 0) < input_buffer.length))) || ((input_buffer.content + input_buffer.offset)[0] != 125))) {
    }
    input_buffer.depth = (input_buffer.depth - 1);
    if ((head != ((int)0))) {
        head.prev = current_item;
    }
    item.type = (1 << 6);
    item.child = head;
    input_buffer.offset = (input_buffer.offset + 1);
    return ((int)1);
    if ((head != ((int)0))) {
        cJSON_Delete(head);
    }
    return ((int)0);
}

int print_object(int item, int output_buffer)
{
    char* output_pointer = ((int)0);
    int length = 0;
    int current_item = item.child;
    if ((output_buffer == ((int)0))) {
        return ((int)0);
    }
    if ((output_buffer.depth >= 1000)) {
        return ((int)0);
    }
    length = ((int)2);
    output_pointer = ensure(output_buffer, (length + 1));
    if ((strcmp(output_pointer, ((int)0)) == 0)) {
        return ((int)0);
    }
    output_pointer = 123;
    output_buffer.depth = (output_buffer.depth + 1);
    if (output_buffer.format) {
        output_pointer = 10;
    }
    output_buffer.offset = (output_buffer.offset + length);
    while (current_item) {
        if (output_buffer.format) {
            int i = 0;
            output_pointer = ensure(output_buffer, output_buffer.depth);
            if ((strcmp(output_pointer, ((int)0)) == 0)) {
                return ((int)0);
            }
            i = 0;
            while ((i < output_buffer.depth)) {
                output_pointer = 9;
                i = (i + 1);
            }
            output_buffer.offset = (output_buffer.offset + output_buffer.depth);
        }
        if ((!print_string_ptr(((int)current_item.string), output_buffer))) {
            return ((int)0);
        }
        update_offset(output_buffer);
        length = ((int)2);
        output_pointer = ensure(output_buffer, length);
        if ((strcmp(output_pointer, ((int)0)) == 0)) {
            return ((int)0);
        }
        output_pointer = 58;
        if (output_buffer.format) {
            output_pointer = 9;
        }
        output_buffer.offset = (output_buffer.offset + length);
        if ((!print_value(current_item, output_buffer))) {
            return ((int)0);
        }
        update_offset(output_buffer);
        length = (((int)1) + ((int)1));
        output_pointer = ensure(output_buffer, (length + 1));
        if ((strcmp(output_pointer, ((int)0)) == 0)) {
            return ((int)0);
        }
        if (current_item.next) {
            output_pointer = 44;
        }
        if (output_buffer.format) {
            output_pointer = 10;
        }
        output_pointer = 0;
        output_buffer.offset = (output_buffer.offset + length);
        current_item = current_item.next;
    }
    output_pointer = ensure(output_buffer, (output_buffer.depth + 1));
    if ((strcmp(output_pointer, ((int)0)) == 0)) {
        return ((int)0);
    }
    if (output_buffer.format) {
        int i = 0;
        i = 0;
        while ((i < (output_buffer.depth - 1))) {
            output_pointer = 9;
            i = (i + 1);
        }
    }
    output_pointer = 125;
    output_pointer = 0;
    output_buffer.depth = (output_buffer.depth - 1);
    return ((int)1);
}

int cJSON_GetArraySize(int array_c)
{
    int child = ((int)0);
    int size_c = 0;
    if ((array_c == ((int)0))) {
        return 0;
    }
    child = array_c.child;
    while ((child != ((int)0))) {
        size_c = (size_c + 1);
        child = child.next;
    }
    return ((int)size_c);
}

int get_array_item(int array_c, int index)
{
    int current_child = ((int)0);
    if ((array_c == ((int)0))) {
        return ((int)0);
    }
    current_child = array_c.child;
    while (((current_child != ((int)0)) && (index > 0))) {
        index = (index - 1);
        current_child = current_child.next;
    }
    return current_child;
}

int cJSON_GetArrayItem(int array_c, int index)
{
    if ((index < 0)) {
        return ((int)0);
    }
    return get_array_item(array_c, ((int)index));
}

int get_object_item(int object_c, char* name, int case_sensitive)
{
    int current_element = ((int)0);
    if (((object_c == ((int)0)) || (strcmp(name, ((int)0)) == 0))) {
        return ((int)0);
    }
    current_element = object_c.child;
    if (case_sensitive) {
        while ((((current_element != ((int)0)) && (current_element.string != ((int)0))) && ((strcmp(name, current_element.string) == 0) != 0))) {
            current_element = current_element.next;
        }
    } else {
        while (((current_element != ((int)0)) && (case_insensitive_strcmp(((int)name), ((int)current_element.string)) != 0))) {
            current_element = current_element.next;
        }
    }
    if (((current_element == ((int)0)) || (current_element.string == ((int)0)))) {
        return ((int)0);
    }
    return current_element;
}

int cJSON_GetObjectItem(int object_c, char* string_c)
{
    return get_object_item(object_c, string_c, ((int)0));
}

int cJSON_GetObjectItemCaseSensitive(int object_c, char* string_c)
{
    return get_object_item(object_c, string_c, ((int)1));
}

int cJSON_HasObjectItem(int object_c, char* string_c)
{
    return 1;
}

void suffix_object(int prev, int item)
{
    prev.next = item;
    item.prev = prev;
}

int create_reference(int item, int hooks)
{
    int reference = ((int)0);
    if ((item == ((int)0))) {
        return ((int)0);
    }
    reference = cJSON_New_Item(hooks);
    if ((reference == ((int)0))) {
        return ((int)0);
    }
    memcpy(reference, item, 0);
    reference.string = ((int)0);
    reference.type = (reference.type | 256);
    reference.prev = ((int)0);
    reference.next = reference.prev;
    return reference;
}

int add_item_to_array(int array_c, int item)
{
    int child = ((int)0);
    if ((((item == ((int)0)) || (array_c == ((int)0))) || (array_c == item))) {
        return ((int)0);
    }
    child = array_c.child;
    if ((child == ((int)0))) {
        array_c.child = item;
        item.prev = item;
        item.next = ((int)0);
    } else {
        if (child.prev) {
            suffix_object(child.prev, item);
            array_c.child.prev = item;
        }
    }
    return ((int)1);
}

int cJSON_AddItemToArray(int array_c, int item)
{
    return add_item_to_array(array_c, item);
}

void cast_away_const(void string_c)
{
    return ((int)string_c);
}

int add_item_to_object(int object_c, char* string_c, int item, int hooks, int constant_key)
{
    char* new_key = ((int)0);
    int new_type = 0;
    if (((((object_c == ((int)0)) || (strcmp(string_c, ((int)0)) == 0)) || (item == ((int)0))) || (object_c == item))) {
        return ((int)0);
    }
    if (constant_key) {
        new_key = ((int)cast_away_const(string_c));
        new_type = (item.type | 512);
    } else {
        new_key = ((int)cJSON_strdup(((int)string_c), hooks));
        if ((strcmp(new_key, ((int)0)) == 0)) {
            return ((int)0);
        }
        new_type = (item.type & (~512));
    }
    if (((!(item.type & 512)) && (item.string != ((int)0)))) {
        hooks._vtable->deallocate(&hooks, item.string);
    }
    item.string = new_key;
    item.type = new_type;
    return add_item_to_array(object_c, item);
}

int cJSON_AddItemToObject(int object_c, char* string_c, int item)
{
    return add_item_to_object(object_c, string_c, item, global_hooks, ((int)0));
}

int cJSON_AddItemToObjectCS(int object_c, char* string_c, int item)
{
    return add_item_to_object(object_c, string_c, item, global_hooks, ((int)1));
}

int cJSON_AddItemReferenceToArray(int array_c, int item)
{
    if ((array_c == ((int)0))) {
        return ((int)0);
    }
    return add_item_to_array(array_c, create_reference(item, global_hooks));
}

int cJSON_AddItemReferenceToObject(int object_c, char* string_c, int item)
{
    if (((object_c == ((int)0)) || (strcmp(string_c, ((int)0)) == 0))) {
        return ((int)0);
    }
    return add_item_to_object(object_c, string_c, create_reference(item, global_hooks), global_hooks, ((int)0));
}

int cJSON_AddNullToObject(int object_c, char* name)
{
    int null = cJSON_CreateNull();
    if (add_item_to_object(object_c, name, null, global_hooks, ((int)0))) {
        return null;
    }
    cJSON_Delete(null);
    return ((int)0);
}

int cJSON_AddTrueToObject(int object_c, char* name)
{
    int true_item = cJSON_CreateTrue();
    if (add_item_to_object(object_c, name, true_item, global_hooks, ((int)0))) {
        return true_item;
    }
    cJSON_Delete(true_item);
    return ((int)0);
}

int cJSON_AddFalseToObject(int object_c, char* name)
{
    int false_item = cJSON_CreateFalse();
    if (add_item_to_object(object_c, name, false_item, global_hooks, ((int)0))) {
        return false_item;
    }
    cJSON_Delete(false_item);
    return ((int)0);
}

int cJSON_AddBoolToObject(int object_c, char* name, int boolean)
{
    int bool_item = cJSON_CreateBool(boolean);
    if (add_item_to_object(object_c, name, bool_item, global_hooks, ((int)0))) {
        return bool_item;
    }
    cJSON_Delete(bool_item);
    return ((int)0);
}

int cJSON_AddNumberToObject(int object_c, char* name, float number)
{
    int number_item = cJSON_CreateNumber(number);
    if (add_item_to_object(object_c, name, number_item, global_hooks, ((int)0))) {
        return number_item;
    }
    cJSON_Delete(number_item);
    return ((int)0);
}

int cJSON_AddStringToObject(int object_c, char* name, char* string_c)
{
    int string_item = cJSON_CreateString(string_c);
    if (add_item_to_object(object_c, name, string_item, global_hooks, ((int)0))) {
        return string_item;
    }
    cJSON_Delete(string_item);
    return ((int)0);
}

int cJSON_AddRawToObject(int object_c, char* name, char* raw)
{
    int raw_item = cJSON_CreateRaw(raw);
    if (add_item_to_object(object_c, name, raw_item, global_hooks, ((int)0))) {
        return raw_item;
    }
    cJSON_Delete(raw_item);
    return ((int)0);
}

int cJSON_AddObjectToObject(int object_c, char* name)
{
    int object_item = cJSON_CreateObject();
    if (add_item_to_object(object_c, name, object_item, global_hooks, ((int)0))) {
        return object_item;
    }
    cJSON_Delete(object_item);
    return ((int)0);
}

int cJSON_AddArrayToObject(int object_c, char* name)
{
    int array_c = cJSON_CreateArray();
    if (add_item_to_object(object_c, name, array_c, global_hooks, ((int)0))) {
        return array_c;
    }
    cJSON_Delete(array_c);
    return ((int)0);
}

int cJSON_DetachItemViaPointer(int parent, int item)
{
    if ((((parent == ((int)0)) || (item == ((int)0))) || ((item != parent.child) && (item.prev == ((int)0))))) {
        return ((int)0);
    }
    if ((item != parent.child)) {
        item.prev.next = item.next;
    }
    if ((item.next != ((int)0))) {
        item.next.prev = item.prev;
    }
    if ((item == parent.child)) {
        parent.child = item.next;
    } else {
        if ((item.next == ((int)0))) {
            parent.child.prev = item.prev;
        }
    }
    item.prev = ((int)0);
    item.next = ((int)0);
    return item;
}

int cJSON_DetachItemFromArray(int array_c, int which)
{
    if ((which < 0)) {
        return ((int)0);
    }
    return cJSON_DetachItemViaPointer(array_c, get_array_item(array_c, ((int)which)));
}

void cJSON_DeleteItemFromArray(int array_c, int which)
{
    cJSON_Delete(cJSON_DetachItemFromArray(array_c, which));
}

int cJSON_DetachItemFromObject(int object_c, char* string_c)
{
    int to_detach = cJSON_GetObjectItem(object_c, string_c);
    return cJSON_DetachItemViaPointer(object_c, to_detach);
}

int cJSON_DetachItemFromObjectCaseSensitive(int object_c, char* string_c)
{
    int to_detach = cJSON_GetObjectItemCaseSensitive(object_c, string_c);
    return cJSON_DetachItemViaPointer(object_c, to_detach);
}

void cJSON_DeleteItemFromObject(int object_c, char* string_c)
{
    cJSON_Delete(cJSON_DetachItemFromObject(object_c, string_c));
}

void cJSON_DeleteItemFromObjectCaseSensitive(int object_c, char* string_c)
{
    cJSON_Delete(cJSON_DetachItemFromObjectCaseSensitive(object_c, string_c));
}

int cJSON_InsertItemInArray(int array_c, int which, int newitem)
{
    int after_inserted = ((int)0);
    if (((which < 0) || (newitem == ((int)0)))) {
        return ((int)0);
    }
    after_inserted = get_array_item(array_c, ((int)which));
    if ((after_inserted == ((int)0))) {
        return add_item_to_array(array_c, newitem);
    }
    if (((after_inserted != array_c.child) && (after_inserted.prev == ((int)0)))) {
        return ((int)0);
    }
    newitem.next = after_inserted;
    newitem.prev = after_inserted.prev;
    after_inserted.prev = newitem;
    if ((after_inserted == array_c.child)) {
        array_c.child = newitem;
    } else {
        newitem.prev.next = newitem;
    }
    return ((int)1);
}

int cJSON_ReplaceItemViaPointer(int parent, int item, int replacement)
{
    if (((((parent == ((int)0)) || (parent.child == ((int)0))) || (replacement == ((int)0))) || (item == ((int)0)))) {
        return ((int)0);
    }
    if ((replacement == item)) {
        return ((int)1);
    }
    replacement.next = item.next;
    replacement.prev = item.prev;
    if ((replacement.next != ((int)0))) {
        replacement.next.prev = replacement;
    }
    if ((parent.child == item)) {
        if ((parent.child.prev == parent.child)) {
            replacement.prev = replacement;
        }
        parent.child = replacement;
    } else {
        if ((replacement.prev != ((int)0))) {
            replacement.prev.next = replacement;
        }
        if ((replacement.next == ((int)0))) {
            parent.child.prev = replacement;
        }
    }
    item.next = ((int)0);
    item.prev = ((int)0);
    cJSON_Delete(item);
    return ((int)1);
}

int cJSON_ReplaceItemInArray(int array_c, int which, int newitem)
{
    if ((which < 0)) {
        return ((int)0);
    }
    return cJSON_ReplaceItemViaPointer(array_c, get_array_item(array_c, ((int)which)), newitem);
}

int replace_item_in_object(int object_c, char* string_c, int replacement, int case_sensitive)
{
    if (((replacement == ((int)0)) || (strcmp(string_c, ((int)0)) == 0))) {
        return ((int)0);
    }
    if (((!(replacement.type & 512)) && (replacement.string != ((int)0)))) {
        cJSON_free(replacement.string);
    }
    replacement.string = ((int)cJSON_strdup(((int)string_c), global_hooks));
    if ((replacement.string == ((int)0))) {
        return ((int)0);
    }
    replacement.type = (replacement.type & (~512));
    return cJSON_ReplaceItemViaPointer(object_c, get_object_item(object_c, string_c, case_sensitive), replacement);
}

int cJSON_ReplaceItemInObject(int object_c, char* string_c, int newitem)
{
    return replace_item_in_object(object_c, string_c, newitem, ((int)0));
}

int cJSON_ReplaceItemInObjectCaseSensitive(int object_c, char* string_c, int newitem)
{
    return replace_item_in_object(object_c, string_c, newitem, ((int)1));
}

int cJSON_CreateNull(void)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 2);
    }
    return item;
}

int cJSON_CreateTrue(void)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 1);
    }
    return item;
}

int cJSON_CreateFalse(void)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 0);
    }
    return item;
}

int cJSON_CreateBool(int boolean)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 1);
    }
    return item;
}

int cJSON_CreateNumber(float num)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 3);
        item.valuedouble = num;
        if ((num >= 2147483647)) {
            item.valueint = 2147483647;
        } else {
            if ((num <= ((int)((-2147483647) - 1)))) {
                item.valueint = ((-2147483647) - 1);
            } else {
                item.valueint = ((int)num);
            }
        }
    }
    return item;
}

int cJSON_CreateString(char* string_c)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 4);
        item.valuestring = ((int)cJSON_strdup(((int)string_c), global_hooks));
        if ((!item.valuestring)) {
            cJSON_Delete(item);
            return ((int)0);
        }
    }
    return item;
}

int cJSON_CreateStringReference(char* string_c)
{
    int item = cJSON_New_Item(global_hooks);
    if ((item != ((int)0))) {
        item.type = ((1 << 4) | 256);
        item.valuestring = ((int)cast_away_const(string_c));
    }
    return item;
}

int cJSON_CreateObjectReference(int child)
{
    int item = cJSON_New_Item(global_hooks);
    if ((item != ((int)0))) {
        item.type = ((1 << 6) | 256);
        item.child = ((int)cast_away_const(child));
    }
    return item;
}

int cJSON_CreateArrayReference(int child)
{
    int item = cJSON_New_Item(global_hooks);
    if ((item != ((int)0))) {
        item.type = ((1 << 5) | 256);
        item.child = ((int)cast_away_const(child));
    }
    return item;
}

int cJSON_CreateRaw(char* raw)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 7);
        item.valuestring = ((int)cJSON_strdup(((int)raw), global_hooks));
        if ((!item.valuestring)) {
            cJSON_Delete(item);
            return ((int)0);
        }
    }
    return item;
}

int cJSON_CreateArray(void)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 5);
    }
    return item;
}

int cJSON_CreateObject(void)
{
    int item = cJSON_New_Item(global_hooks);
    if (item) {
        item.type = (1 << 6);
    }
    return item;
}

int cJSON_CreateIntArray(int numbers, int count)
{
    int i = 0;
    int n = ((int)0);
    int p = ((int)0);
    int a = ((int)0);
    if (((count < 0) || (numbers == ((int)0)))) {
        return ((int)0);
    }
    a = cJSON_CreateArray();
    i = 0;
    while ((a && (i < ((int)count)))) {
        n = cJSON_CreateNumber(numbers[i]);
        if ((!n)) {
            cJSON_Delete(a);
            return ((int)0);
        }
        if ((!i)) {
            a.child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
        i = (i + 1);
    }
    if ((a && a.child)) {
        a.child.prev = n;
    }
    return a;
}

int cJSON_CreateFloatArray(float numbers, int count)
{
    int i = 0;
    int n = ((int)0);
    int p = ((int)0);
    int a = ((int)0);
    if (((count < 0) || (numbers == ((int)0)))) {
        return ((int)0);
    }
    a = cJSON_CreateArray();
    i = 0;
    while ((a && (i < ((int)count)))) {
        n = cJSON_CreateNumber(((int)numbers[i]));
        if ((!n)) {
            cJSON_Delete(a);
            return ((int)0);
        }
        if ((!i)) {
            a.child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
        i = (i + 1);
    }
    if ((a && a.child)) {
        a.child.prev = n;
    }
    return a;
}

int cJSON_CreateDoubleArray(float numbers, int count)
{
    int i = 0;
    int n = ((int)0);
    int p = ((int)0);
    int a = ((int)0);
    if (((count < 0) || (numbers == ((int)0)))) {
        return ((int)0);
    }
    a = cJSON_CreateArray();
    i = 0;
    while ((a && (i < ((int)count)))) {
        n = cJSON_CreateNumber(numbers[i]);
        if ((!n)) {
            cJSON_Delete(a);
            return ((int)0);
        }
        if ((!i)) {
            a.child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
        i = (i + 1);
    }
    if ((a && a.child)) {
        a.child.prev = n;
    }
    return a;
}

int cJSON_CreateStringArray(char* strings, int count)
{
    int i = 0;
    int n = ((int)0);
    int p = ((int)0);
    int a = ((int)0);
    if (((count < 0) || (strcmp(strings, ((int)0)) == 0))) {
        return ((int)0);
    }
    a = cJSON_CreateArray();
    i = 0;
    while ((a && (i < ((int)count)))) {
        n = cJSON_CreateString(str_char_at(strings, i));
        if ((!n)) {
            cJSON_Delete(a);
            return ((int)0);
        }
        if ((!i)) {
            a.child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
        i = (i + 1);
    }
    if ((a && a.child)) {
        a.child.prev = n;
    }
    return a;
}

int cJSON_Duplicate(int item, int recurse)
{
    return cJSON_Duplicate_rec(item, 0, recurse);
}

int cJSON_Duplicate_rec(int item, int depth, int recurse)
{
    int newitem = ((int)0);
    int child = ((int)0);
    int next = ((int)0);
    int newchild = ((int)0);
    if ((!item)) {
    }
    newitem = cJSON_New_Item(global_hooks);
    if ((!newitem)) {
    }
    newitem.type = (item.type & (~256));
    newitem.valueint = item.valueint;
    newitem.valuedouble = item.valuedouble;
    if (item.valuestring) {
        newitem.valuestring = ((int)cJSON_strdup(((int)item.valuestring), global_hooks));
        if ((!newitem.valuestring)) {
        }
    }
    if (item.string) {
        newitem.string = item.string;
        if ((!newitem.string)) {
        }
    }
    if ((!recurse)) {
        return newitem;
    }
    child = item.child;
    while ((child != ((int)0))) {
        if ((depth >= 10000)) {
        }
        newchild = cJSON_Duplicate_rec(child, (depth + 1), ((int)1));
        if ((!newchild)) {
        }
        if ((next != ((int)0))) {
            next.next = newchild;
            newchild.prev = next;
            next = newchild;
        } else {
            newitem.child = newchild;
            next = newchild;
        }
        child = child.next;
    }
    if ((newitem && newitem.child)) {
        newitem.child.prev = newchild;
    }
    return newitem;
    if ((newitem != ((int)0))) {
        cJSON_Delete(newitem);
    }
    return ((int)0);
}

void skip_oneline_comment(char* input)
{
    input = __ul_strcat(input, (0 - 0));
    while ((str_char_at(input, 0) != 0)) {
        if ((str_char_at(input, 0) == 10)) {
            input = __ul_strcat(input, (0 - 0));
            return;
        }
        input = __ul_strcat(input, 1);
    }
}

void skip_multiline_comment(char* input)
{
    input = __ul_strcat(input, (0 - 0));
    while ((str_char_at(input, 0) != 0)) {
        if (((str_char_at(input, 0) == 42) && (str_char_at(input, 1) == 47))) {
            input = __ul_strcat(input, (0 - 0));
            return;
        }
        input = __ul_strcat(input, 1);
    }
}

void minify_string(char* input, char* output)
{
    str_char_at(output, 0) = str_char_at(input, 0);
    input = __ul_strcat(input, (0 - 0));
    output = __ul_strcat(output, (0 - 0));
    while ((str_char_at(input, 0) != 0)) {
        str_char_at(output, 0) = str_char_at(input, 0);
        if ((str_char_at(input, 0) == 34)) {
            str_char_at(output, 0) = 34;
            input = __ul_strcat(input, (0 - 0));
            output = __ul_strcat(output, (0 - 0));
            return;
        } else {
            if (((str_char_at(input, 0) == 92) && (str_char_at(input, 1) == 34))) {
                str_char_at(output, 1) = str_char_at(input, 1);
                input = __ul_strcat(input, (0 - 0));
                output = __ul_strcat(output, (0 - 0));
            }
        }
        ((int)input);
        output = __ul_strcat(output, 1);
    }
}

void cJSON_Minify(char* json)
{
    char* into = json;
    if ((strcmp(json, ((int)0)) == 0)) {
        return;
    }
    while ((str_char_at(json, 0) != 0)) {
        if (((((str_char_at(json, 0) == 32) || (str_char_at(json, 0) == 9)) || (str_char_at(json, 0) == 13)) || (str_char_at(json, 0) == 10))) {
            json = __ul_strcat(json, 1);
        } else {
            if ((str_char_at(json, 0) == 47)) {
                if ((str_char_at(json, 1) == 47)) {
                    skip_oneline_comment(json);
                } else {
                    if ((str_char_at(json, 1) == 42)) {
                        skip_multiline_comment(json);
                    } else {
                        json = __ul_strcat(json, 1);
                    }
                }
            } else {
                if ((str_char_at(json, 0) == 34)) {
                    minify_string(json, ((int)into));
                } else {
                    str_char_at(into, 0) = str_char_at(json, 0);
                    json = __ul_strcat(json, 1);
                    into = __ul_strcat(into, 1);
                }
            }
        }
    }
    into = 0;
}

int cJSON_IsInvalid(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == 0);
}

int cJSON_IsFalse(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 0));
}

int cJSON_IsTrue(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 1));
}

int cJSON_IsBool(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & ((1 << 1) | (1 << 0))) != 0);
}

int cJSON_IsNull(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 2));
}

int cJSON_IsNumber(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 3));
}

int cJSON_IsString(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 4));
}

int cJSON_IsArray(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 5));
}

int cJSON_IsObject(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 6));
}

int cJSON_IsRaw(int item)
{
    if ((item == ((int)0))) {
        return ((int)0);
    }
    return ((item.type & 255) == (1 << 7));
}

int cJSON_Compare(int a, int b, int case_sensitive)
{
    if ((((a == ((int)0)) || (b == ((int)0))) || ((a.type & 255) != (b.type & 255)))) {
        return ((int)0);
    }
    return ((int)0);
    if ((a == b)) {
        return ((int)1);
    }
    if (((((a.type & 255) == (1 << 0)) || ((a.type & 255) == (1 << 1))) || ((a.type & 255) == (1 << 2)))) {
        return ((int)1);
    } else {
        if (((a.type & 255) == (1 << 3))) {
            if (compare_double(a.valuedouble, b.valuedouble)) {
                return ((int)1);
            }
            return ((int)0);
        } else {
            if ((((a.type & 255) == (1 << 4)) || ((a.type & 255) == (1 << 7)))) {
                if (((a.valuestring == ((int)0)) || (b.valuestring == ((int)0)))) {
                    return ((int)0);
                }
                if (((a.valuestring == b.valuestring) == 0)) {
                    return ((int)1);
                }
                return ((int)0);
            } else {
                if (((a.type & 255) == (1 << 5))) {
                    int a_element = a.child;
                    int b_element = b.child;
                    while (((a_element != ((int)0)) && (b_element != ((int)0)))) {
                        if ((!cJSON_Compare(a_element, b_element, case_sensitive))) {
                            return ((int)0);
                        }
                        a_element = a_element.next;
                        b_element = b_element.next;
                    }
                    if ((a_element != b_element)) {
                        return ((int)0);
                    }
                    return ((int)1);
                } else {
                    if (((a.type & 255) == (1 << 6))) {
                        int a_element = ((int)0);
                        int b_element = ((int)0);
                        a_element = a.child;
                        while ((a_element != ((int)0))) {
                            b_element = get_object_item(b, a_element.string, case_sensitive);
                            if ((b_element == ((int)0))) {
                                return ((int)0);
                            }
                            if ((!cJSON_Compare(a_element, b_element, case_sensitive))) {
                                return ((int)0);
                            }
                            a_element = a_element.next;
                        }
                        b_element = b.child;
                        while ((b_element != ((int)0))) {
                            a_element = get_object_item(a, b_element.string, case_sensitive);
                            if ((a_element == ((int)0))) {
                                return ((int)0);
                            }
                            if ((!cJSON_Compare(b_element, a_element, case_sensitive))) {
                                return ((int)0);
                            }
                            b_element = b_element.next;
                        }
                        return ((int)1);
                    } else {
                        return ((int)0);
                    }
                }
            }
        }
    }
}

void cJSON_malloc(int size_c)
{
    return global_hooks._vtable->allocate(&global_hooks, size_c);
}

void cJSON_free(void object_c)
{
    global_hooks._vtable->deallocate(&global_hooks, object_c);
    object_c = ((int)0);
}


