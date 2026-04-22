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
#include <stdarg.h>

typedef struct {
    char* chars;
    int length;
} JSON_String;

typedef struct {
    int* parent;
    int type_c;
    int value;
} json_value_t;

typedef struct {
    int* wrapping_value;
    int* cells;
    uint64_t* hashes;
    char* names;
    int* values;
    int* cell_ixs;
    int count;
    int item_capacity;
    int cell_capacity;
} json_object_t;

typedef struct {
    int* wrapping_value;
    int* items;
    int count;
    int capacity;
} json_array_t;

int parson_malloc = (&malloc);
int parson_free = (&free);
int parson_escape_slashes = 1;
char parson_float_format = ((void)0);
int parson_number_serialization_function = ((void)0);
char* read_file(char* filename);
void remove_comments(char* string_c, char* start_token, char* end_token);
char* parson_strndup(char* string_c, int n);
char* parson_strdup(char* string_c);
int parson_sprintf(char* s, char* format, ...);
int hex_char_to_int(char c);
int parse_utf16_hex(char* s, uint32_t* result);
int num_bytes_in_utf8_sequence(uint8_t c);
int verify_utf8_sequence(uint8_t* string_c, int* len);
int is_valid_utf8(char* string_c, int string_len);
int is_decimal(char* string_c, int length);
uint64_t hash_string(char* string_c, int n);
int* json_object_make(int* wrapping_value);
int json_object_init(int* object_c, int capacity);
void json_object_deinit(int* object_c, int free_keys, int free_values);
int json_object_grow_and_rehash(int* object_c);
int json_object_get_cell_ix(int* object_c, char* key, int key_len, uint64_t hash, int* out_found);
int json_object_add(int* object_c, char* name, int* value);
int* json_object_getn_value(int* object_c, char* name, int name_len);
int json_object_remove_internal(int* object_c, char* name, int free_value);
int json_object_dotremove_internal(int* object_c, char* name, int free_value);
void json_object_free(int* object_c);
int* json_array_make(int* wrapping_value);
int json_array_add(int* array_c, int* value);
int json_array_resize(int* array_c, int new_capacity);
void json_array_free(int* array_c);
int* json_value_init_string_no_copy(char* string_c, int length);
int skip_quotes(char* string_c);
int parse_utf16(char* unprocessed, char* processed);
char* process_string(char* input, int input_len, int* output_len);
char* get_quoted_string(char* string_c, int* output_string_len);
int* parse_value(char* string_c, int nesting);
int* parse_object_value(char* string_c, int nesting);
int* parse_array_value(char* string_c, int nesting);
int* parse_string_value(char* string_c);
int* parse_boolean_value(char* string_c);
int* parse_number_value(char* string_c);
int* parse_null_value(char* string_c);
int json_serialize_to_buffer_r(int* value, char* buf, int level, int is_pretty, char* num_buf);
int json_serialize_string(char* string_c, int len, char* buf);
int* json_parse_file(char* filename);
int* json_parse_file_with_comments(char* filename);
int* json_parse_string(char* string_c);
int* json_parse_string_with_comments(char* string_c);
int* json_object_get_value(int* object_c, char* name);
char* json_object_get_string(int* object_c, char* name);
int json_object_get_string_len(int* object_c, char* name);
double json_object_get_number(int* object_c, char* name);
int* json_object_get_object(int* object_c, char* name);
int* json_object_get_array(int* object_c, char* name);
int json_object_get_boolean(int* object_c, char* name);
int* json_object_dotget_value(int* object_c, char* name);
char* json_object_dotget_string(int* object_c, char* name);
int json_object_dotget_string_len(int* object_c, char* name);
double json_object_dotget_number(int* object_c, char* name);
int* json_object_dotget_object(int* object_c, char* name);
int* json_object_dotget_array(int* object_c, char* name);
int json_object_dotget_boolean(int* object_c, char* name);
int json_object_get_count(int* object_c);
char* json_object_get_name(int* object_c, int index);
int* json_object_get_value_at(int* object_c, int index);
int* json_object_get_wrapping_value(int* object_c);
int json_object_has_value(int* object_c, char* name);
int json_object_has_value_of_type(int* object_c, char* name, int type_c);
int json_object_dothas_value(int* object_c, char* name);
int json_object_dothas_value_of_type(int* object_c, char* name, int type_c);
int* json_array_get_value(int* array_c, int index);
char* json_array_get_string(int* array_c, int index);
int json_array_get_string_len(int* array_c, int index);
double json_array_get_number(int* array_c, int index);
int* json_array_get_object(int* array_c, int index);
int* json_array_get_array(int* array_c, int index);
int json_array_get_boolean(int* array_c, int index);
int json_array_get_count(int* array_c);
int* json_array_get_wrapping_value(int* array_c);
int json_value_get_type(int* value);
int* json_value_get_object(int* value);
int* json_value_get_array(int* value);
JSON_String* json_value_get_string_desc(int* value);
char* json_value_get_string(int* value);
int json_value_get_string_len(int* value);
double json_value_get_number(int* value);
int json_value_get_boolean(int* value);
int* json_value_get_parent(int* value);
void json_value_free(int* value);
int* json_value_init_object(void);
int* json_value_init_array(void);
int* json_value_init_string(char* string_c);
int* json_value_init_string_with_len(char* string_c, int length);
int* json_value_init_number(double number);
int* json_value_init_boolean(int boolean);
int* json_value_init_null(void);
int* json_value_deep_copy(int* value);
int json_serialization_size(int* value);
int json_serialize_to_buffer(int* value, char* buf, int buf_size_in_bytes);
int json_serialize_to_file(int* value, char* filename);
char* json_serialize_to_string(int* value);
int json_serialization_size_pretty(int* value);
int json_serialize_to_buffer_pretty(int* value, char* buf, int buf_size_in_bytes);
int json_serialize_to_file_pretty(int* value, char* filename);
char* json_serialize_to_string_pretty(int* value);
void json_free_serialized_string(char* string_c);
int json_array_remove(int* array_c, int ix);
int json_array_replace_value(int* array_c, int ix, int* value);
int json_array_replace_string(int* array_c, int i, char* string_c);
int json_array_replace_string_with_len(int* array_c, int i, char* string_c, int len);
int json_array_replace_number(int* array_c, int i, double number);
int json_array_replace_boolean(int* array_c, int i, int boolean);
int json_array_replace_null(int* array_c, int i);
int json_array_clear(int* array_c);
int json_array_append_value(int* array_c, int* value);
int json_array_append_string(int* array_c, char* string_c);
int json_array_append_string_with_len(int* array_c, char* string_c, int len);
int json_array_append_number(int* array_c, double number);
int json_array_append_boolean(int* array_c, int boolean);
int json_array_append_null(int* array_c);
int json_object_set_value(int* object_c, char* name, int* value);
int json_object_set_string(int* object_c, char* name, char* string_c);
int json_object_set_string_with_len(int* object_c, char* name, char* string_c, int len);
int json_object_set_number(int* object_c, char* name, double number);
int json_object_set_boolean(int* object_c, char* name, int boolean);
int json_object_set_null(int* object_c, char* name);
int json_object_dotset_value(int* object_c, char* name, int* value);
int json_object_dotset_string(int* object_c, char* name, char* string_c);
int json_object_dotset_string_with_len(int* object_c, char* name, char* string_c, int len);
int json_object_dotset_number(int* object_c, char* name, double number);
int json_object_dotset_boolean(int* object_c, char* name, int boolean);
int json_object_dotset_null(int* object_c, char* name);
int json_object_remove(int* object_c, char* name);
int json_object_dotremove(int* object_c, char* name);
int json_object_clear(int* object_c);
int json_validate(int* schema, int* value);
int json_value_equals(int* a, int* b);
int json_type(int* value);
int* json_object(int* value);
int* json_array(int* value);
char* json_string(int* value);
int json_string_len(int* value);
double json_number(int* value);
int json_boolean(int* value);
void json_set_allocation_functions(int malloc_fun, int free_fun);
void json_set_escape_slashes(int escape_slashes);
void json_set_float_serialization_format(char* format);
void json_set_number_serialization_function(int func);

char* read_file(char* filename)
{
    int* fp = fopen(filename, "r");
    int size_to_read = 0;
    int size_read = 0;
    int pos = 0;
    char* file_contents = 0;
    if ((!fp)) {
        return ((void)0);
    }
    fseek(fp, 0, 2);
    pos = ftell(fp);
    if ((pos < 0)) {
        fclose(fp);
        return ((void)0);
    }
    size_to_read = pos;
    rewind(fp);
    file_contents = ((char)parson_malloc((sizeof(char) * (size_to_read + 1))));
    if ((!file_contents)) {
        fclose(fp);
        return ((void)0);
    }
    size_read = fread(file_contents, 1, size_to_read, fp);
    if (((size_read == 0) || ferror(fp))) {
        fclose(fp);
        parson_free(file_contents);
        return ((void)0);
    }
    fclose(fp);
    file_contents[size_read] = 0;
    return file_contents;
}

void remove_comments(char* string_c, char* start_token, char* end_token)
{
    int in_string = 0;
    int escaped = 0;
    int i = 0;
    char* ptr = ((void)0);
    char current_char = 0;
    int start_token_len = sizeof(start_token);
    int end_token_len = sizeof(end_token);
    if (((start_token_len == 0) || (end_token_len == 0))) {
        return;
    }
    current_char = (*string_c);
    while ((current_char != 0)) {
        if (((current_char == 92) && (!escaped))) {
            escaped = 1;
            string_c = (string_c + 1);
            continue;
        } else {
            if (((current_char == 34) && (!escaped))) {
                in_string = (!in_string);
            } else {
                if (((!in_string) && (strncmp(string_c, start_token, start_token_len) == 0))) {
                    i = 0;
                    while ((i < start_token_len)) {
                        string_c[i] = 32;
                        i = (i + 1);
                    }
                    string_c = (string_c + start_token_len);
                    ptr = strstr(string_c, end_token);
                    if ((!ptr)) {
                        return;
                    }
                    i = 0;
                    while ((i < ((ptr - string_c) + end_token_len))) {
                        string_c[i] = 32;
                        i = (i + 1);
                    }
                    string_c = ((ptr + end_token_len) - 1);
                }
            }
        }
        escaped = 0;
        string_c = (string_c + 1);
    }
}

char* parson_strndup(char* string_c, int n)
{
    char* output_string = ((char)parson_malloc((n + 1)));
    if ((!output_string)) {
        return ((void)0);
    }
    output_string[n] = 0;
    memcpy(output_string, string_c, n);
    return output_string;
}

char* parson_strdup(char* string_c)
{
    return parson_strndup(string_c, sizeof(string_c));
}

int parson_sprintf(char* s, char* format, ...)
{
    int result = 0;
    int args_c = 0;
    0;
    result = 0;
    0;
    return result;
}

int hex_char_to_int(char c)
{
    if (((c >= 48) && (c <= 57))) {
        return (c - 48);
    } else {
        if (((c >= 97) && (c <= 102))) {
            return ((c - 97) + 10);
        } else {
            if (((c >= 65) && (c <= 70))) {
                return ((c - 65) + 10);
            }
        }
    }
    return (-1);
}

int parse_utf16_hex(char* s, uint32_t* result)
{
    int x1 = 0;
    int x2 = 0;
    int x3 = 0;
    int x4 = 0;
    if (((((s[0] == 0) || (s[1] == 0)) || (s[2] == 0)) || (s[3] == 0))) {
        return JSONFailure;
    }
    x1 = hex_char_to_int(s[0]);
    x2 = hex_char_to_int(s[1]);
    x3 = hex_char_to_int(s[2]);
    x4 = hex_char_to_int(s[3]);
    if (((((x1 == (-1)) || (x2 == (-1))) || (x3 == (-1))) || (x4 == (-1)))) {
        return JSONFailure;
    }
    (*result) = ((uint32_t)((((x1 << 12) | (x2 << 8)) | (x3 << 4)) | x4));
    return JSONSuccess;
}

int num_bytes_in_utf8_sequence(uint8_t c)
{
    if (((((c == 192) || (c == 193)) || (c > 244)) || ((((uint8_t)c) & 192) == 128))) {
        return 0;
    } else {
        if (((c & 128) == 0)) {
            return 1;
        } else {
            if (((c & 224) == 192)) {
                return 2;
            } else {
                if (((c & 240) == 224)) {
                    return 3;
                } else {
                    if (((c & 248) == 240)) {
                        return 4;
                    }
                }
            }
        }
    }
    return 0;
}

int verify_utf8_sequence(uint8_t* string_c, int* len)
{
    uint32_t cp = 0;
    (*len) = num_bytes_in_utf8_sequence(string_c[0]);
    if (((*len) == 1)) {
        cp = string_c[0];
    } else {
        if ((((*len) == 2) && ((((uint8_t)string_c[1]) & 192) == 128))) {
            cp = (string_c[0] & 31);
            cp = ((cp << 6) | (string_c[1] & 63));
        } else {
            if (((((*len) == 3) && ((((uint8_t)string_c[1]) & 192) == 128)) && ((((uint8_t)string_c[2]) & 192) == 128))) {
                cp = (((uint8_t)string_c[0]) & 15);
                cp = ((cp << 6) | (string_c[1] & 63));
                cp = ((cp << 6) | (string_c[2] & 63));
            } else {
                if ((((((*len) == 4) && ((((uint8_t)string_c[1]) & 192) == 128)) && ((((uint8_t)string_c[2]) & 192) == 128)) && ((((uint8_t)string_c[3]) & 192) == 128))) {
                    cp = (string_c[0] & 7);
                    cp = ((cp << 6) | (string_c[1] & 63));
                    cp = ((cp << 6) | (string_c[2] & 63));
                    cp = ((cp << 6) | (string_c[3] & 63));
                } else {
                    return JSONFailure;
                }
            }
        }
    }
    if (((((cp < 128) && ((*len) > 1)) || ((cp < 2048) && ((*len) > 2))) || ((cp < 65536) && ((*len) > 3)))) {
        return JSONFailure;
    }
    if ((cp > 1114111)) {
        return JSONFailure;
    }
    if (((cp >= 55296) && (cp <= 57343))) {
        return JSONFailure;
    }
    return JSONSuccess;
}

int is_valid_utf8(char* string_c, int string_len)
{
    int len = 0;
    char* string_end = (string_c + string_len);
    while ((string_c < string_end)) {
        if ((verify_utf8_sequence(((uint8_t)string_c), (&len)) != JSONSuccess)) {
            return 0;
        }
        string_c = (string_c + len);
    }
    return 1;
}

int is_decimal(char* string_c, int length)
{
    if ((((length > 1) && (string_c[0] == 48)) && (string_c[1] != 46))) {
        return 0;
    }
    if ((((length > 2) && (!strncmp(string_c, "-0", 2))) && (string_c[2] != 46))) {
        return 0;
    }
    while (length) {
        if (strchr("xX", string_c[length])) {
            return 0;
        }
    }
    return 1;
}

uint64_t hash_string(char* string_c, int n)
{
    uint64_t hash = 5381;
    uint8_t c = 0;
    int i = 0;
    int _brk_0 = 0;
    i = 0;
    while (((i < n) && (!_brk_0))) {
        c = string_c[i];
        if ((c == 0)) {
            _brk_0 = 1;
        }
        if ((!_brk_0)) {
            hash = (((hash << 5) + hash) + c);
        }
        if ((!_brk_0)) {
            i = (i + 1);
        }
    }
    return hash;
}

int* json_object_make(int* wrapping_value)
{
    int res = JSONFailure;
    int* new_obj = ((int)parson_malloc(sizeof(int)));
    if ((new_obj == ((void)0))) {
        return ((void)0);
    }
    new_obj->wrapping_value = wrapping_value;
    res = json_object_init(new_obj, 0);
    if ((res != JSONSuccess)) {
        parson_free(new_obj);
        return ((void)0);
    }
    return new_obj;
}

int json_object_init(int* object_c, int capacity)
{
    uint32_t i = 0;
    object_c->cells = ((void)0);
    object_c->names = ((void)0);
    object_c->values = ((void)0);
    object_c->cell_ixs = ((void)0);
    object_c->hashes = ((void)0);
    object_c->count = 0;
    object_c->cell_capacity = capacity;
    object_c->item_capacity = ((uint32_t)((capacity * 7) / 10));
    if ((capacity == 0)) {
        return JSONSuccess;
    }
    object_c->cells = ((int)parson_malloc((object_c->cell_capacity * sizeof((*object_c->cells)))));
    object_c->names = ((char)parson_malloc((object_c->item_capacity * sizeof((*object_c->names)))));
    object_c->values = ((int)parson_malloc((object_c->item_capacity * sizeof((*object_c->values)))));
    object_c->cell_ixs = ((int)parson_malloc((object_c->item_capacity * sizeof((*object_c->cell_ixs)))));
    object_c->hashes = ((uint64_t)parson_malloc((object_c->item_capacity * sizeof((*object_c->hashes)))));
    if ((((((object_c->cells == ((void)0)) || (object_c->names == ((void)0))) || (object_c->values == ((void)0))) || (object_c->cell_ixs == ((void)0))) || (object_c->hashes == ((void)0)))) {
        goto error_c;
    }
    i = 0;
    while ((i < object_c->cell_capacity)) {
        object_c->cells[i] = ((int)(-1));
        i = (i + 1);
    }
    return JSONSuccess;
    error_c:;
    parson_free(object_c->cells);
    parson_free(object_c->names);
    parson_free(object_c->values);
    parson_free(object_c->cell_ixs);
    parson_free(object_c->hashes);
    return JSONFailure;
}

void json_object_deinit(int* object_c, int free_keys, int free_values)
{
    uint32_t i = 0;
    i = 0;
    while ((i < object_c->count)) {
        if (free_keys) {
            parson_free(object_c->names[i]);
        }
        if (free_values) {
            json_value_free(object_c->values[i]);
        }
        i = (i + 1);
    }
    object_c->count = 0;
    object_c->item_capacity = 0;
    object_c->cell_capacity = 0;
    parson_free(object_c->cells);
    parson_free(object_c->names);
    parson_free(object_c->values);
    parson_free(object_c->cell_ixs);
    parson_free(object_c->hashes);
    object_c->cells = ((void)0);
    object_c->names = ((void)0);
    object_c->values = ((void)0);
    object_c->cell_ixs = ((void)0);
    object_c->hashes = ((void)0);
}

int json_object_grow_and_rehash(int* object_c)
{
    int* wrapping_value = ((void)0);
    int new_object = 0;
    char* key = ((void)0);
    int* value = ((void)0);
    uint32_t i = 0;
    __auto_type _tern_1 = 16;
    if (((object_c->cell_capacity * 2) > 16)) {
        _tern_1 = (object_c->cell_capacity * 2);
    }
    int new_capacity = _tern_1;
    int res = json_object_init((&new_object), new_capacity);
    if ((res != JSONSuccess)) {
        return JSONFailure;
    }
    wrapping_value = json_object_get_wrapping_value(object_c);
    new_object.wrapping_value = wrapping_value;
    i = 0;
    while ((i < object_c->count)) {
        key = object_c->names[i];
        value = object_c->values[i];
        res = json_object_add((&new_object), key, value);
        if ((res != JSONSuccess)) {
            json_object_deinit((&new_object), 0, 0);
            return JSONFailure;
        }
        value->parent = wrapping_value;
        i = (i + 1);
    }
    json_object_deinit(object_c, 0, 0);
    (*object_c) = new_object;
    return JSONSuccess;
}

int json_object_get_cell_ix(int* object_c, char* key, int key_len, uint64_t hash, int* out_found)
{
    int cell_ix = (hash & (object_c->cell_capacity - 1));
    int cell = 0;
    int ix = 0;
    uint32_t i = 0;
    uint64_t hash_to_check = 0;
    char* key_to_check = ((void)0);
    int key_to_check_len = 0;
    (*out_found) = 0;
    i = 0;
    while ((i < object_c->cell_capacity)) {
        ix = ((cell_ix + i) & (object_c->cell_capacity - 1));
        cell = object_c->cells[ix];
        if ((cell == ((int)(-1)))) {
            return ix;
        }
        hash_to_check = object_c->hashes[cell];
        if ((hash != hash_to_check)) {
            continue;
        }
        key_to_check = object_c->names[cell];
        key_to_check_len = sizeof(key_to_check);
        if (((key_to_check_len == key_len) && (strncmp(key, key_to_check, key_len) == 0))) {
            (*out_found) = 1;
            return ix;
        }
        i = (i + 1);
    }
    return ((int)(-1));
}

int json_object_add(int* object_c, char* name, int* value)
{
    uint64_t hash = 0;
    int found = 0;
    int cell_ix = 0;
    int res = JSONFailure;
    if ((((!object_c) || (!name)) || (!value))) {
        return JSONFailure;
    }
    hash = hash_string(name, sizeof(name));
    found = 0;
    cell_ix = json_object_get_cell_ix(object_c, name, sizeof(name), hash, (&found));
    if (found) {
        return JSONFailure;
    }
    if ((object_c->count >= object_c->item_capacity)) {
        res = json_object_grow_and_rehash(object_c);
        if ((res != JSONSuccess)) {
            return JSONFailure;
        }
        cell_ix = json_object_get_cell_ix(object_c, name, sizeof(name), hash, (&found));
    }
    object_c->names[object_c->count] = name;
    object_c->cells[cell_ix] = object_c->count;
    object_c->values[object_c->count] = value;
    object_c->cell_ixs[object_c->count] = cell_ix;
    object_c->hashes[object_c->count] = hash;
    object_c->count = (object_c->count + 1);
    value->parent = json_object_get_wrapping_value(object_c);
    return JSONSuccess;
}

int* json_object_getn_value(int* object_c, char* name, int name_len)
{
    uint64_t hash = 0;
    int found = 0;
    int cell_ix = 0;
    int item_ix = 0;
    if (((!object_c) || (!name))) {
        return ((void)0);
    }
    hash = hash_string(name, name_len);
    found = 0;
    cell_ix = json_object_get_cell_ix(object_c, name, name_len, hash, (&found));
    if ((!found)) {
        return ((void)0);
    }
    item_ix = object_c->cells[cell_ix];
    return object_c->values[item_ix];
}

int json_object_remove_internal(int* object_c, char* name, int free_value)
{
    uint64_t hash = 0;
    int found = 0;
    int cell = 0;
    int item_ix = 0;
    int last_item_ix = 0;
    int i = 0;
    int j = 0;
    int x = 0;
    int k = 0;
    int* val = ((void)0);
    if ((object_c == ((void)0))) {
        return JSONFailure;
    }
    hash = hash_string(name, sizeof(name));
    found = 0;
    cell = json_object_get_cell_ix(object_c, name, sizeof(name), hash, (&found));
    if ((!found)) {
        return JSONFailure;
    }
    item_ix = object_c->cells[cell];
    if (free_value) {
        val = object_c->values[item_ix];
        json_value_free(val);
        val = ((void)0);
    }
    parson_free(object_c->names[item_ix]);
    last_item_ix = (object_c->count - 1);
    if ((item_ix < last_item_ix)) {
        object_c->names[item_ix] = object_c->names[last_item_ix];
        object_c->values[item_ix] = object_c->values[last_item_ix];
        object_c->cell_ixs[item_ix] = object_c->cell_ixs[last_item_ix];
        object_c->hashes[item_ix] = object_c->hashes[last_item_ix];
        object_c->cells[object_c->cell_ixs[item_ix]] = item_ix;
    }
    object_c->count = (object_c->count - 1);
    i = cell;
    j = i;
    int _brk_1 = 0;
    x = 0;
    while (((x < (object_c->cell_capacity - 1)) && (!_brk_1))) {
        j = ((j + 1) & (object_c->cell_capacity - 1));
        if ((object_c->cells[j] == ((int)(-1)))) {
            _brk_1 = 1;
        }
        if ((!_brk_1)) {
            k = (object_c->hashes[object_c->cells[j]] & (object_c->cell_capacity - 1));
            if ((((j > i) && ((k <= i) || (k > j))) || ((j < i) && ((k <= i) && (k > j))))) {
                object_c->cell_ixs[object_c->cells[j]] = i;
                object_c->cells[i] = object_c->cells[j];
                i = j;
            }
        }
        if ((!_brk_1)) {
            x = (x + 1);
        }
    }
    object_c->cells[i] = ((int)(-1));
    return JSONSuccess;
}

int json_object_dotremove_internal(int* object_c, char* name, int free_value)
{
    int* temp_value = ((void)0);
    int* temp_object = ((void)0);
    char* dot_pos = strchr(name, 46);
    if ((!dot_pos)) {
        return json_object_remove_internal(object_c, name, free_value);
    }
    temp_value = json_object_getn_value(object_c, name, (dot_pos - name));
    if ((json_value_get_type(temp_value) != JSONObject)) {
        return JSONFailure;
    }
    temp_object = json_value_get_object(temp_value);
    return json_object_dotremove_internal(temp_object, (dot_pos + 1), free_value);
}

void json_object_free(int* object_c)
{
    json_object_deinit(object_c, 1, 1);
    parson_free(object_c);
}

int* json_array_make(int* wrapping_value)
{
    int* new_array = ((int)parson_malloc(sizeof(int)));
    if ((new_array == ((void)0))) {
        return ((void)0);
    }
    new_array->wrapping_value = wrapping_value;
    new_array->items = ((int)((void)0));
    new_array->capacity = 0;
    new_array->count = 0;
    return new_array;
}

int json_array_add(int* array_c, int* value)
{
    if ((array_c->count >= array_c->capacity)) {
        __auto_type _tern_2 = 16;
        if (((array_c->capacity * 2) > 16)) {
            _tern_2 = (array_c->capacity * 2);
        }
        int new_capacity = _tern_2;
        if ((json_array_resize(array_c, new_capacity) != JSONSuccess)) {
            return JSONFailure;
        }
    }
    value->parent = json_array_get_wrapping_value(array_c);
    array_c->items[array_c->count] = value;
    array_c->count = (array_c->count + 1);
    return JSONSuccess;
}

int json_array_resize(int* array_c, int new_capacity)
{
    int* new_items = ((void)0);
    if ((new_capacity == 0)) {
        return JSONFailure;
    }
    new_items = ((int)parson_malloc((new_capacity * sizeof(int))));
    if ((new_items == ((void)0))) {
        return JSONFailure;
    }
    if (((array_c->items != ((void)0)) && (array_c->count > 0))) {
        memcpy(new_items, array_c->items, (array_c->count * sizeof(int)));
    }
    parson_free(array_c->items);
    array_c->items = new_items;
    array_c->capacity = new_capacity;
    return JSONSuccess;
}

void json_array_free(int* array_c)
{
    int i = 0;
    i = 0;
    while ((i < array_c->count)) {
        json_value_free(array_c->items[i]);
        i = (i + 1);
    }
    parson_free(array_c->items);
    parson_free(array_c);
}

int* json_value_init_string_no_copy(char* string_c, int length)
{
    int* new_value = ((int)parson_malloc(sizeof(int)));
    if ((!new_value)) {
        return ((void)0);
    }
    new_value->parent = ((void)0);
    new_value->type_c = JSONString;
    new_value->value.string_c.chars = string_c;
    new_value->value.string_c.length = length;
    return new_value;
}

int skip_quotes(char* string_c)
{
    if (((*(*string_c)) != 34)) {
        return JSONFailure;
    }
    (*string_c) = ((*string_c) + 1);
    while (((*(*string_c)) != 34)) {
        if (((*(*string_c)) == 0)) {
            return JSONFailure;
        } else {
            if (((*(*string_c)) == 92)) {
                (*string_c) = ((*string_c) + 1);
                if (((*(*string_c)) == 0)) {
                    return JSONFailure;
                }
            }
        }
        (*string_c) = ((*string_c) + 1);
    }
    (*string_c) = ((*string_c) + 1);
    return JSONSuccess;
}

int parse_utf16(char* unprocessed, char* processed)
{
    uint32_t cp = 0;
    uint32_t lead = 0;
    uint32_t trail = 0;
    char* processed_ptr = (*processed);
    char* unprocessed_ptr = (*unprocessed);
    int status = JSONFailure;
    unprocessed_ptr = (unprocessed_ptr + 1);
    status = parse_utf16_hex(unprocessed_ptr, (&cp));
    if ((status != JSONSuccess)) {
        return JSONFailure;
    }
    if ((cp < 128)) {
        processed_ptr[0] = ((char)cp);
    } else {
        if ((cp < 2048)) {
            processed_ptr[0] = (((cp >> 6) & 31) | 192);
            processed_ptr[1] = ((cp & 63) | 128);
            processed_ptr = (processed_ptr + 1);
        } else {
            if (((cp < 55296) || (cp > 57343))) {
                processed_ptr[0] = (((cp >> 12) & 15) | 224);
                processed_ptr[1] = (((cp >> 6) & 63) | 128);
                processed_ptr[2] = ((cp & 63) | 128);
                processed_ptr = (processed_ptr + 2);
            } else {
                if (((cp >= 55296) && (cp <= 56319))) {
                    lead = cp;
                    unprocessed_ptr = (unprocessed_ptr + 4);
                    if ((((*unprocessed_ptr) != 92) || ((*unprocessed_ptr) != 117))) {
                        return JSONFailure;
                    }
                    status = parse_utf16_hex(unprocessed_ptr, (&trail));
                    if ((((status != JSONSuccess) || (trail < 56320)) || (trail > 57343))) {
                        return JSONFailure;
                    }
                    cp = (((((lead - 55296) & 1023) << 10) | ((trail - 56320) & 1023)) + 65536);
                    processed_ptr[0] = (((cp >> 18) & 7) | 240);
                    processed_ptr[1] = (((cp >> 12) & 63) | 128);
                    processed_ptr[2] = (((cp >> 6) & 63) | 128);
                    processed_ptr[3] = ((cp & 63) | 128);
                    processed_ptr = (processed_ptr + 3);
                } else {
                    return JSONFailure;
                }
            }
        }
    }
    unprocessed_ptr = (unprocessed_ptr + 3);
    (*processed) = processed_ptr;
    (*unprocessed) = unprocessed_ptr;
    return JSONSuccess;
}

char* process_string(char* input, int input_len, int* output_len)
{
    char* input_ptr = input;
    int initial_size = ((input_len + 1) * sizeof(char));
    int final_size = 0;
    char* output = ((void)0);
    char* output_ptr = ((void)0);
    char* resized_output = ((void)0);
    output = ((char)parson_malloc(initial_size));
    if ((output == ((void)0))) {
        goto error_c;
    }
    output_ptr = output;
    while ((((*input_ptr) != 0) && (((int)(input_ptr - input)) < input_len))) {
        if (((*input_ptr) == 92)) {
            input_ptr = (input_ptr + 1);
            if (((*input_ptr) == 34)) {
                (*output_ptr) = 34;
            } else {
                if (((*input_ptr) == 92)) {
                    (*output_ptr) = 92;
                } else {
                    if (((*input_ptr) == 47)) {
                        (*output_ptr) = 47;
                    } else {
                        if (((*input_ptr) == 98)) {
                            (*output_ptr) = 8;
                        } else {
                            if (((*input_ptr) == 102)) {
                                (*output_ptr) = 12;
                            } else {
                                if (((*input_ptr) == 110)) {
                                    (*output_ptr) = 10;
                                } else {
                                    if (((*input_ptr) == 114)) {
                                        (*output_ptr) = 13;
                                    } else {
                                        if (((*input_ptr) == 116)) {
                                            (*output_ptr) = 9;
                                        } else {
                                            if (((*input_ptr) == 117)) {
                                                if ((parse_utf16((&input_ptr), (&output_ptr)) != JSONSuccess)) {
                                                    goto error_c;
                                                }
                                            } else {
                                                goto error_c;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if ((((uint8_t)(*input_ptr)) < 32)) {
                goto error_c;
            } else {
                (*output_ptr) = (*input_ptr);
            }
        }
        output_ptr = (output_ptr + 1);
        input_ptr = (input_ptr + 1);
    }
    (*output_ptr) = 0;
    final_size = (((int)(output_ptr - output)) + 1);
    resized_output = ((char)parson_malloc(final_size));
    if ((resized_output == ((void)0))) {
        goto error_c;
    }
    memcpy(resized_output, output, final_size);
    (*output_len) = (final_size - 1);
    parson_free(output);
    return resized_output;
    error_c:;
    parson_free(output);
    return ((void)0);
}

char* get_quoted_string(char* string_c, int* output_string_len)
{
    char* string_start = (*string_c);
    int input_string_len = 0;
    int status = skip_quotes(string_c);
    if ((status != JSONSuccess)) {
        return ((void)0);
    }
    input_string_len = (((*string_c) - string_start) - 2);
    return process_string((string_start + 1), input_string_len, output_string_len);
}

int* parse_value(char* string_c, int nesting)
{
    if ((nesting > 2048)) {
        return ((void)0);
    }
    while (isspace(((uint8_t)(*(*string_c))))) {
        (*string_c) = ((*string_c) + 1);
    }
    if (((*(*string_c)) == 123)) {
        return parse_object_value(string_c, (nesting + 1));
    } else {
        if (((*(*string_c)) == 91)) {
            return parse_array_value(string_c, (nesting + 1));
        } else {
            if (((*(*string_c)) == 34)) {
                return parse_string_value(string_c);
            } else {
                if ((((*(*string_c)) == 102) || ((*(*string_c)) == 116))) {
                    return parse_boolean_value(string_c);
                } else {
                    if (((((((((((((*(*string_c)) == 45) || ((*(*string_c)) == 48)) || ((*(*string_c)) == 49)) || ((*(*string_c)) == 50)) || ((*(*string_c)) == 51)) || ((*(*string_c)) == 52)) || ((*(*string_c)) == 53)) || ((*(*string_c)) == 54)) || ((*(*string_c)) == 55)) || ((*(*string_c)) == 56)) || ((*(*string_c)) == 57))) {
                        return parse_number_value(string_c);
                    } else {
                        if (((*(*string_c)) == 110)) {
                            return parse_null_value(string_c);
                        } else {
                            return ((void)0);
                        }
                    }
                }
            }
        }
    }
}

int* parse_object_value(char* string_c, int nesting)
{
    int status = JSONFailure;
    int* output_value = ((void)0);
    int* new_value = ((void)0);
    int* output_object = ((void)0);
    char* new_key = ((void)0);
    output_value = json_value_init_object();
    if ((output_value == ((void)0))) {
        return ((void)0);
    }
    if (((*(*string_c)) != 123)) {
        json_value_free(output_value);
        return ((void)0);
    }
    output_object = json_value_get_object(output_value);
    (*string_c) = ((*string_c) + 1);
    while (isspace(((uint8_t)(*(*string_c))))) {
        (*string_c) = ((*string_c) + 1);
    }
    if (((*(*string_c)) == 125)) {
        (*string_c) = ((*string_c) + 1);
        return output_value;
    }
    int _brk_2 = 0;
    while ((((*(*string_c)) != 0) && (!_brk_2))) {
        int key_len = 0;
        new_key = get_quoted_string(string_c, (&key_len));
        if ((!new_key)) {
            json_value_free(output_value);
            return ((void)0);
        }
        if ((key_len != sizeof(new_key))) {
            parson_free(new_key);
            json_value_free(output_value);
            return ((void)0);
        }
        while (isspace(((uint8_t)(*(*string_c))))) {
            (*string_c) = ((*string_c) + 1);
        }
        if (((*(*string_c)) != 58)) {
            parson_free(new_key);
            json_value_free(output_value);
            return ((void)0);
        }
        (*string_c) = ((*string_c) + 1);
        new_value = parse_value(string_c, nesting);
        if ((new_value == ((void)0))) {
            parson_free(new_key);
            json_value_free(output_value);
            return ((void)0);
        }
        status = json_object_add(output_object, new_key, new_value);
        if ((status != JSONSuccess)) {
            parson_free(new_key);
            json_value_free(new_value);
            json_value_free(output_value);
            return ((void)0);
        }
        while (isspace(((uint8_t)(*(*string_c))))) {
            (*string_c) = ((*string_c) + 1);
        }
        if (((*(*string_c)) != 44)) {
            _brk_2 = 1;
        }
        if ((!_brk_2)) {
            (*string_c) = ((*string_c) + 1);
            while (isspace(((uint8_t)(*(*string_c))))) {
                (*string_c) = ((*string_c) + 1);
            }
            if (((*(*string_c)) == 125)) {
                _brk_2 = 1;
            }
        }
    }
    while (isspace(((uint8_t)(*(*string_c))))) {
        (*string_c) = ((*string_c) + 1);
    }
    if (((*(*string_c)) != 125)) {
        json_value_free(output_value);
        return ((void)0);
    }
    (*string_c) = ((*string_c) + 1);
    return output_value;
}

int* parse_array_value(char* string_c, int nesting)
{
    int* output_value = ((void)0);
    int* new_array_value = ((void)0);
    int* output_array = ((void)0);
    output_value = json_value_init_array();
    if ((output_value == ((void)0))) {
        return ((void)0);
    }
    if (((*(*string_c)) != 91)) {
        json_value_free(output_value);
        return ((void)0);
    }
    output_array = json_value_get_array(output_value);
    (*string_c) = ((*string_c) + 1);
    while (isspace(((uint8_t)(*(*string_c))))) {
        (*string_c) = ((*string_c) + 1);
    }
    if (((*(*string_c)) == 93)) {
        (*string_c) = ((*string_c) + 1);
        return output_value;
    }
    int _brk_3 = 0;
    while ((((*(*string_c)) != 0) && (!_brk_3))) {
        new_array_value = parse_value(string_c, nesting);
        if ((new_array_value == ((void)0))) {
            json_value_free(output_value);
            return ((void)0);
        }
        if ((json_array_add(output_array, new_array_value) != JSONSuccess)) {
            json_value_free(new_array_value);
            json_value_free(output_value);
            return ((void)0);
        }
        while (isspace(((uint8_t)(*(*string_c))))) {
            (*string_c) = ((*string_c) + 1);
        }
        if (((*(*string_c)) != 44)) {
            _brk_3 = 1;
        }
        if ((!_brk_3)) {
            (*string_c) = ((*string_c) + 1);
            while (isspace(((uint8_t)(*(*string_c))))) {
                (*string_c) = ((*string_c) + 1);
            }
            if (((*(*string_c)) == 93)) {
                _brk_3 = 1;
            }
        }
    }
    while (isspace(((uint8_t)(*(*string_c))))) {
        (*string_c) = ((*string_c) + 1);
    }
    if ((((*(*string_c)) != 93) || (json_array_resize(output_array, json_array_get_count(output_array)) != JSONSuccess))) {
        json_value_free(output_value);
        return ((void)0);
    }
    (*string_c) = ((*string_c) + 1);
    return output_value;
}

int* parse_string_value(char* string_c)
{
    int* value = ((void)0);
    int new_string_len = 0;
    char* new_string = get_quoted_string(string_c, (&new_string_len));
    if ((new_string == ((void)0))) {
        return ((void)0);
    }
    value = json_value_init_string_no_copy(new_string, new_string_len);
    if ((value == ((void)0))) {
        parson_free(new_string);
        return ((void)0);
    }
    return value;
}

int* parse_boolean_value(char* string_c)
{
    int true_token_size = (sizeof("true") - 1);
    int false_token_size = (sizeof("false") - 1);
    if ((strncmp("true", (*string_c), true_token_size) == 0)) {
        (*string_c) = ((*string_c) + true_token_size);
        return json_value_init_boolean(1);
    } else {
        if ((strncmp("false", (*string_c), false_token_size) == 0)) {
            (*string_c) = ((*string_c) + false_token_size);
            return json_value_init_boolean(0);
        }
    }
    return ((void)0);
}

int* parse_number_value(char* string_c)
{
    char* end_c = 0;
    double number = 0;
    (*_errno()) = 0;
    number = ((double)(*string_c));
    if ((((*_errno()) == 34) && ((number <= (-0)) || (number >= 0)))) {
        return ((void)0);
    }
    if ((((*_errno()) && ((*_errno()) != 34)) || (!is_decimal((*string_c), (end_c - (*string_c)))))) {
        return ((void)0);
    }
    (*string_c) = end_c;
    return json_value_init_number(number);
}

int* parse_null_value(char* string_c)
{
    int token_size = (sizeof("null") - 1);
    if ((strncmp("null", (*string_c), token_size) == 0)) {
        (*string_c) = ((*string_c) + token_size);
        return json_value_init_null();
    }
    return ((void)0);
}

int json_serialize_to_buffer_r(int* value, char* buf, int level, int is_pretty, char* num_buf)
{
    char* key = ((void)0);
    char* string_c = ((void)0);
    int* temp_value = ((void)0);
    int* array_c = ((void)0);
    int* object_c = ((void)0);
    int i = 0;
    int count = 0;
    double num = 0.0;
    int written = (-1);
    int written_total = 0;
    int len = 0;
    if ((json_value_get_type(value) == JSONArray)) {
        array_c = json_value_get_array(value);
        count = json_array_get_count(array_c);
        written = (sizeof("[") - 1);
        if ((buf != ((void)0))) {
            memcpy(buf, "[", written);
            buf[written] = 0;
            buf = (buf + written);
        }
        written_total = (written_total + written);
        if (((count > 0) && is_pretty)) {
            written = (sizeof("\n") - 1);
            if ((buf != ((void)0))) {
                memcpy(buf, "\n", written);
                buf[written] = 0;
                buf = (buf + written);
            }
            written_total = (written_total + written);
        }
        i = 0;
        while ((i < count)) {
            if (is_pretty) {
                int level_i = 0;
                level_i = 0;
                while ((level_i < (level + 1))) {
                    written = (sizeof("    ") - 1);
                    if ((buf != ((void)0))) {
                        memcpy(buf, "    ", written);
                        buf[written] = 0;
                        buf = (buf + written);
                    }
                    written_total = (written_total + written);
                    level_i = (level_i + 1);
                }
            }
            temp_value = json_array_get_value(array_c, i);
            written = json_serialize_to_buffer_r(temp_value, buf, (level + 1), is_pretty, num_buf);
            if ((written < 0)) {
                return (-1);
            }
            if ((buf != ((void)0))) {
                buf = (buf + written);
            }
            written_total = (written_total + written);
            if ((i < (count - 1))) {
                written = (sizeof(",") - 1);
                if ((buf != ((void)0))) {
                    memcpy(buf, ",", written);
                    buf[written] = 0;
                    buf = (buf + written);
                }
                written_total = (written_total + written);
            }
            if (is_pretty) {
                written = (sizeof("\n") - 1);
                if ((buf != ((void)0))) {
                    memcpy(buf, "\n", written);
                    buf[written] = 0;
                    buf = (buf + written);
                }
                written_total = (written_total + written);
            }
            i = (i + 1);
        }
        if (((count > 0) && is_pretty)) {
            level_i = 0;
            level_i = 0;
            while ((level_i < level)) {
                written = (sizeof("    ") - 1);
                if ((buf != ((void)0))) {
                    memcpy(buf, "    ", written);
                    buf[written] = 0;
                    buf = (buf + written);
                }
                written_total = (written_total + written);
                level_i = (level_i + 1);
            }
        }
        written = (sizeof("]") - 1);
        if ((buf != ((void)0))) {
            memcpy(buf, "]", written);
            buf[written] = 0;
            buf = (buf + written);
        }
        written_total = (written_total + written);
        return written_total;
    } else {
        if ((json_value_get_type(value) == JSONObject)) {
            object_c = json_value_get_object(value);
            count = json_object_get_count(object_c);
            written = (sizeof("{") - 1);
            if ((buf != ((void)0))) {
                memcpy(buf, "{", written);
                buf[written] = 0;
                buf = (buf + written);
            }
            written_total = (written_total + written);
            if (((count > 0) && is_pretty)) {
                written = (sizeof("\n") - 1);
                if ((buf != ((void)0))) {
                    memcpy(buf, "\n", written);
                    buf[written] = 0;
                    buf = (buf + written);
                }
                written_total = (written_total + written);
            }
            i = 0;
            while ((i < count)) {
                key = json_object_get_name(object_c, i);
                if ((key == ((void)0))) {
                    return (-1);
                }
                if (is_pretty) {
                    level_i = 0;
                    level_i = 0;
                    while ((level_i < (level + 1))) {
                        written = (sizeof("    ") - 1);
                        if ((buf != ((void)0))) {
                            memcpy(buf, "    ", written);
                            buf[written] = 0;
                            buf = (buf + written);
                        }
                        written_total = (written_total + written);
                        level_i = (level_i + 1);
                    }
                }
                written = json_serialize_string(key, sizeof(key), buf);
                if ((written < 0)) {
                    return (-1);
                }
                if ((buf != ((void)0))) {
                    buf = (buf + written);
                }
                written_total = (written_total + written);
                written = (sizeof(":") - 1);
                if ((buf != ((void)0))) {
                    memcpy(buf, ":", written);
                    buf[written] = 0;
                    buf = (buf + written);
                }
                written_total = (written_total + written);
                if (is_pretty) {
                    written = (sizeof(" ") - 1);
                    if ((buf != ((void)0))) {
                        memcpy(buf, " ", written);
                        buf[written] = 0;
                        buf = (buf + written);
                    }
                    written_total = (written_total + written);
                }
                temp_value = json_object_get_value_at(object_c, i);
                written = json_serialize_to_buffer_r(temp_value, buf, (level + 1), is_pretty, num_buf);
                if ((written < 0)) {
                    return (-1);
                }
                if ((buf != ((void)0))) {
                    buf = (buf + written);
                }
                written_total = (written_total + written);
                if ((i < (count - 1))) {
                    written = (sizeof(",") - 1);
                    if ((buf != ((void)0))) {
                        memcpy(buf, ",", written);
                        buf[written] = 0;
                        buf = (buf + written);
                    }
                    written_total = (written_total + written);
                }
                if (is_pretty) {
                    written = (sizeof("\n") - 1);
                    if ((buf != ((void)0))) {
                        memcpy(buf, "\n", written);
                        buf[written] = 0;
                        buf = (buf + written);
                    }
                    written_total = (written_total + written);
                }
                i = (i + 1);
            }
            if (((count > 0) && is_pretty)) {
                level_i = 0;
                level_i = 0;
                while ((level_i < level)) {
                    written = (sizeof("    ") - 1);
                    if ((buf != ((void)0))) {
                        memcpy(buf, "    ", written);
                        buf[written] = 0;
                        buf = (buf + written);
                    }
                    written_total = (written_total + written);
                    level_i = (level_i + 1);
                }
            }
            written = (sizeof("}") - 1);
            if ((buf != ((void)0))) {
                memcpy(buf, "}", written);
                buf[written] = 0;
                buf = (buf + written);
            }
            written_total = (written_total + written);
            return written_total;
        } else {
            if ((json_value_get_type(value) == JSONString)) {
                string_c = json_value_get_string(value);
                if ((string_c == ((void)0))) {
                    return (-1);
                }
                len = json_value_get_string_len(value);
                written = json_serialize_string(string_c, len, buf);
                if ((written < 0)) {
                    return (-1);
                }
                if ((buf != ((void)0))) {
                    buf = (buf + written);
                }
                written_total = (written_total + written);
                return written_total;
            } else {
                if ((json_value_get_type(value) == JSONBoolean)) {
                    if (json_value_get_boolean(value)) {
                        written = (sizeof("true") - 1);
                        if ((buf != ((void)0))) {
                            memcpy(buf, "true", written);
                            buf[written] = 0;
                            buf = (buf + written);
                        }
                        written_total = (written_total + written);
                    } else {
                        written = (sizeof("false") - 1);
                        if ((buf != ((void)0))) {
                            memcpy(buf, "false", written);
                            buf[written] = 0;
                            buf = (buf + written);
                        }
                        written_total = (written_total + written);
                    }
                    return written_total;
                } else {
                    if ((json_value_get_type(value) == JSONNumber)) {
                        num = json_value_get_number(value);
                        if ((buf != ((void)0))) {
                            num_buf = buf;
                        }
                        if (parson_number_serialization_function) {
                            written = parson_number_serialization_function(num, num_buf);
                        } else {
                            __auto_type _tern_3 = "%1.17g";
                            if (parson_float_format) {
                                _tern_3 = parson_float_format;
                            }
                            char* float_format = _tern_3;
                            written = parson_sprintf(num_buf, float_format, num);
                        }
                        if ((written < 0)) {
                            return (-1);
                        }
                        if ((buf != ((void)0))) {
                            buf = (buf + written);
                        }
                        written_total = (written_total + written);
                        return written_total;
                    } else {
                        if ((json_value_get_type(value) == JSONNull)) {
                            written = (sizeof("null") - 1);
                            if ((buf != ((void)0))) {
                                memcpy(buf, "null", written);
                                buf[written] = 0;
                                buf = (buf + written);
                            }
                            written_total = (written_total + written);
                            return written_total;
                        } else {
                            if ((json_value_get_type(value) == JSONError)) {
                                return (-1);
                            } else {
                                return (-1);
                            }
                        }
                    }
                }
            }
        }
    }
}

int json_serialize_string(char* string_c, int len, char* buf)
{
    int i = 0;
    char c = 0;
    int written = (-1);
    int written_total = 0;
    written = (sizeof("\"") - 1);
    if ((buf != ((void)0))) {
        memcpy(buf, "\"", written);
        buf[written] = 0;
        buf = (buf + written);
    }
    written_total = (written_total + written);
    i = 0;
    while ((i < len)) {
        c = string_c[i];
        if ((c == 34)) {
            written = (sizeof("\\\"") - 1);
            if ((buf != ((void)0))) {
                memcpy(buf, "\\\"", written);
                buf[written] = 0;
                buf = (buf + written);
            }
            written_total = (written_total + written);
        } else {
            if ((c == 92)) {
                written = (sizeof("\\\\") - 1);
                if ((buf != ((void)0))) {
                    memcpy(buf, "\\\\", written);
                    buf[written] = 0;
                    buf = (buf + written);
                }
                written_total = (written_total + written);
            } else {
                if ((c == 8)) {
                    written = (sizeof("\\b") - 1);
                    if ((buf != ((void)0))) {
                        memcpy(buf, "\\b", written);
                        buf[written] = 0;
                        buf = (buf + written);
                    }
                    written_total = (written_total + written);
                } else {
                    if ((c == 12)) {
                        written = (sizeof("\\f") - 1);
                        if ((buf != ((void)0))) {
                            memcpy(buf, "\\f", written);
                            buf[written] = 0;
                            buf = (buf + written);
                        }
                        written_total = (written_total + written);
                    } else {
                        if ((c == 10)) {
                            written = (sizeof("\\n") - 1);
                            if ((buf != ((void)0))) {
                                memcpy(buf, "\\n", written);
                                buf[written] = 0;
                                buf = (buf + written);
                            }
                            written_total = (written_total + written);
                        } else {
                            if ((c == 13)) {
                                written = (sizeof("\\r") - 1);
                                if ((buf != ((void)0))) {
                                    memcpy(buf, "\\r", written);
                                    buf[written] = 0;
                                    buf = (buf + written);
                                }
                                written_total = (written_total + written);
                            } else {
                                if ((c == 9)) {
                                    written = (sizeof("\\t") - 1);
                                    if ((buf != ((void)0))) {
                                        memcpy(buf, "\\t", written);
                                        buf[written] = 0;
                                        buf = (buf + written);
                                    }
                                    written_total = (written_total + written);
                                } else {
                                    if ((c == 0)) {
                                        written = (sizeof("\\u0000") - 1);
                                        if ((buf != ((void)0))) {
                                            memcpy(buf, "\\u0000", written);
                                            buf[written] = 0;
                                            buf = (buf + written);
                                        }
                                        written_total = (written_total + written);
                                    } else {
                                        if ((c == 1)) {
                                            written = (sizeof("\\u0001") - 1);
                                            if ((buf != ((void)0))) {
                                                memcpy(buf, "\\u0001", written);
                                                buf[written] = 0;
                                                buf = (buf + written);
                                            }
                                            written_total = (written_total + written);
                                        } else {
                                            if ((c == 2)) {
                                                written = (sizeof("\\u0002") - 1);
                                                if ((buf != ((void)0))) {
                                                    memcpy(buf, "\\u0002", written);
                                                    buf[written] = 0;
                                                    buf = (buf + written);
                                                }
                                                written_total = (written_total + written);
                                            } else {
                                                if ((c == 3)) {
                                                    written = (sizeof("\\u0003") - 1);
                                                    if ((buf != ((void)0))) {
                                                        memcpy(buf, "\\u0003", written);
                                                        buf[written] = 0;
                                                        buf = (buf + written);
                                                    }
                                                    written_total = (written_total + written);
                                                } else {
                                                    if ((c == 4)) {
                                                        written = (sizeof("\\u0004") - 1);
                                                        if ((buf != ((void)0))) {
                                                            memcpy(buf, "\\u0004", written);
                                                            buf[written] = 0;
                                                            buf = (buf + written);
                                                        }
                                                        written_total = (written_total + written);
                                                    } else {
                                                        if ((c == 5)) {
                                                            written = (sizeof("\\u0005") - 1);
                                                            if ((buf != ((void)0))) {
                                                                memcpy(buf, "\\u0005", written);
                                                                buf[written] = 0;
                                                                buf = (buf + written);
                                                            }
                                                            written_total = (written_total + written);
                                                        } else {
                                                            if ((c == 6)) {
                                                                written = (sizeof("\\u0006") - 1);
                                                                if ((buf != ((void)0))) {
                                                                    memcpy(buf, "\\u0006", written);
                                                                    buf[written] = 0;
                                                                    buf = (buf + written);
                                                                }
                                                                written_total = (written_total + written);
                                                            } else {
                                                                if ((c == 7)) {
                                                                    written = (sizeof("\\u0007") - 1);
                                                                    if ((buf != ((void)0))) {
                                                                        memcpy(buf, "\\u0007", written);
                                                                        buf[written] = 0;
                                                                        buf = (buf + written);
                                                                    }
                                                                    written_total = (written_total + written);
                                                                } else {
                                                                    if ((c == 11)) {
                                                                        written = (sizeof("\\u000b") - 1);
                                                                        if ((buf != ((void)0))) {
                                                                            memcpy(buf, "\\u000b", written);
                                                                            buf[written] = 0;
                                                                            buf = (buf + written);
                                                                        }
                                                                        written_total = (written_total + written);
                                                                    } else {
                                                                        if ((c == 14)) {
                                                                            written = (sizeof("\\u000e") - 1);
                                                                            if ((buf != ((void)0))) {
                                                                                memcpy(buf, "\\u000e", written);
                                                                                buf[written] = 0;
                                                                                buf = (buf + written);
                                                                            }
                                                                            written_total = (written_total + written);
                                                                        } else {
                                                                            if ((c == 15)) {
                                                                                written = (sizeof("\\u000f") - 1);
                                                                                if ((buf != ((void)0))) {
                                                                                    memcpy(buf, "\\u000f", written);
                                                                                    buf[written] = 0;
                                                                                    buf = (buf + written);
                                                                                }
                                                                                written_total = (written_total + written);
                                                                            } else {
                                                                                if ((c == 16)) {
                                                                                    written = (sizeof("\\u0010") - 1);
                                                                                    if ((buf != ((void)0))) {
                                                                                        memcpy(buf, "\\u0010", written);
                                                                                        buf[written] = 0;
                                                                                        buf = (buf + written);
                                                                                    }
                                                                                    written_total = (written_total + written);
                                                                                } else {
                                                                                    if ((c == 17)) {
                                                                                        written = (sizeof("\\u0011") - 1);
                                                                                        if ((buf != ((void)0))) {
                                                                                            memcpy(buf, "\\u0011", written);
                                                                                            buf[written] = 0;
                                                                                            buf = (buf + written);
                                                                                        }
                                                                                        written_total = (written_total + written);
                                                                                    } else {
                                                                                        if ((c == 18)) {
                                                                                            written = (sizeof("\\u0012") - 1);
                                                                                            if ((buf != ((void)0))) {
                                                                                                memcpy(buf, "\\u0012", written);
                                                                                                buf[written] = 0;
                                                                                                buf = (buf + written);
                                                                                            }
                                                                                            written_total = (written_total + written);
                                                                                        } else {
                                                                                            if ((c == 19)) {
                                                                                                written = (sizeof("\\u0013") - 1);
                                                                                                if ((buf != ((void)0))) {
                                                                                                    memcpy(buf, "\\u0013", written);
                                                                                                    buf[written] = 0;
                                                                                                    buf = (buf + written);
                                                                                                }
                                                                                                written_total = (written_total + written);
                                                                                            } else {
                                                                                                if ((c == 20)) {
                                                                                                    written = (sizeof("\\u0014") - 1);
                                                                                                    if ((buf != ((void)0))) {
                                                                                                        memcpy(buf, "\\u0014", written);
                                                                                                        buf[written] = 0;
                                                                                                        buf = (buf + written);
                                                                                                    }
                                                                                                    written_total = (written_total + written);
                                                                                                } else {
                                                                                                    if ((c == 21)) {
                                                                                                        written = (sizeof("\\u0015") - 1);
                                                                                                        if ((buf != ((void)0))) {
                                                                                                            memcpy(buf, "\\u0015", written);
                                                                                                            buf[written] = 0;
                                                                                                            buf = (buf + written);
                                                                                                        }
                                                                                                        written_total = (written_total + written);
                                                                                                    } else {
                                                                                                        if ((c == 22)) {
                                                                                                            written = (sizeof("\\u0016") - 1);
                                                                                                            if ((buf != ((void)0))) {
                                                                                                                memcpy(buf, "\\u0016", written);
                                                                                                                buf[written] = 0;
                                                                                                                buf = (buf + written);
                                                                                                            }
                                                                                                            written_total = (written_total + written);
                                                                                                        } else {
                                                                                                            if ((c == 23)) {
                                                                                                                written = (sizeof("\\u0017") - 1);
                                                                                                                if ((buf != ((void)0))) {
                                                                                                                    memcpy(buf, "\\u0017", written);
                                                                                                                    buf[written] = 0;
                                                                                                                    buf = (buf + written);
                                                                                                                }
                                                                                                                written_total = (written_total + written);
                                                                                                            } else {
                                                                                                                if ((c == 24)) {
                                                                                                                    written = (sizeof("\\u0018") - 1);
                                                                                                                    if ((buf != ((void)0))) {
                                                                                                                        memcpy(buf, "\\u0018", written);
                                                                                                                        buf[written] = 0;
                                                                                                                        buf = (buf + written);
                                                                                                                    }
                                                                                                                    written_total = (written_total + written);
                                                                                                                } else {
                                                                                                                    if ((c == 25)) {
                                                                                                                        written = (sizeof("\\u0019") - 1);
                                                                                                                        if ((buf != ((void)0))) {
                                                                                                                            memcpy(buf, "\\u0019", written);
                                                                                                                            buf[written] = 0;
                                                                                                                            buf = (buf + written);
                                                                                                                        }
                                                                                                                        written_total = (written_total + written);
                                                                                                                    } else {
                                                                                                                        if ((c == 26)) {
                                                                                                                            written = (sizeof("\\u001a") - 1);
                                                                                                                            if ((buf != ((void)0))) {
                                                                                                                                memcpy(buf, "\\u001a", written);
                                                                                                                                buf[written] = 0;
                                                                                                                                buf = (buf + written);
                                                                                                                            }
                                                                                                                            written_total = (written_total + written);
                                                                                                                        } else {
                                                                                                                            if ((c == 27)) {
                                                                                                                                written = (sizeof("\\u001b") - 1);
                                                                                                                                if ((buf != ((void)0))) {
                                                                                                                                    memcpy(buf, "\\u001b", written);
                                                                                                                                    buf[written] = 0;
                                                                                                                                    buf = (buf + written);
                                                                                                                                }
                                                                                                                                written_total = (written_total + written);
                                                                                                                            } else {
                                                                                                                                if ((c == 28)) {
                                                                                                                                    written = (sizeof("\\u001c") - 1);
                                                                                                                                    if ((buf != ((void)0))) {
                                                                                                                                        memcpy(buf, "\\u001c", written);
                                                                                                                                        buf[written] = 0;
                                                                                                                                        buf = (buf + written);
                                                                                                                                    }
                                                                                                                                    written_total = (written_total + written);
                                                                                                                                } else {
                                                                                                                                    if ((c == 29)) {
                                                                                                                                        written = (sizeof("\\u001d") - 1);
                                                                                                                                        if ((buf != ((void)0))) {
                                                                                                                                            memcpy(buf, "\\u001d", written);
                                                                                                                                            buf[written] = 0;
                                                                                                                                            buf = (buf + written);
                                                                                                                                        }
                                                                                                                                        written_total = (written_total + written);
                                                                                                                                    } else {
                                                                                                                                        if ((c == 30)) {
                                                                                                                                            written = (sizeof("\\u001e") - 1);
                                                                                                                                            if ((buf != ((void)0))) {
                                                                                                                                                memcpy(buf, "\\u001e", written);
                                                                                                                                                buf[written] = 0;
                                                                                                                                                buf = (buf + written);
                                                                                                                                            }
                                                                                                                                            written_total = (written_total + written);
                                                                                                                                        } else {
                                                                                                                                            if ((c == 31)) {
                                                                                                                                                written = (sizeof("\\u001f") - 1);
                                                                                                                                                if ((buf != ((void)0))) {
                                                                                                                                                    memcpy(buf, "\\u001f", written);
                                                                                                                                                    buf[written] = 0;
                                                                                                                                                    buf = (buf + written);
                                                                                                                                                }
                                                                                                                                                written_total = (written_total + written);
                                                                                                                                            } else {
                                                                                                                                                if ((c == 47)) {
                                                                                                                                                    if (parson_escape_slashes) {
                                                                                                                                                        written = (sizeof("\\/") - 1);
                                                                                                                                                        if ((buf != ((void)0))) {
                                                                                                                                                            memcpy(buf, "\\/", written);
                                                                                                                                                            buf[written] = 0;
                                                                                                                                                            buf = (buf + written);
                                                                                                                                                        }
                                                                                                                                                        written_total = (written_total + written);
                                                                                                                                                    } else {
                                                                                                                                                        written = (sizeof("/") - 1);
                                                                                                                                                        if ((buf != ((void)0))) {
                                                                                                                                                            memcpy(buf, "/", written);
                                                                                                                                                            buf[written] = 0;
                                                                                                                                                            buf = (buf + written);
                                                                                                                                                        }
                                                                                                                                                        written_total = (written_total + written);
                                                                                                                                                    }
                                                                                                                                                } else {
                                                                                                                                                    if ((buf != ((void)0))) {
                                                                                                                                                        buf[0] = c;
                                                                                                                                                        buf = (buf + 1);
                                                                                                                                                    }
                                                                                                                                                    written_total = (written_total + 1);
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
                }
            }
        }
        i = (i + 1);
    }
    written = (sizeof("\"") - 1);
    if ((buf != ((void)0))) {
        memcpy(buf, "\"", written);
        buf[written] = 0;
        buf = (buf + written);
    }
    written_total = (written_total + written);
    return written_total;
}

int* json_parse_file(char* filename)
{
    char* file_contents = read_file(filename);
    int* output_value = ((void)0);
    if ((file_contents == ((void)0))) {
        return ((void)0);
    }
    output_value = json_parse_string(file_contents);
    parson_free(file_contents);
    return output_value;
}

int* json_parse_file_with_comments(char* filename)
{
    char* file_contents = read_file(filename);
    int* output_value = ((void)0);
    if ((file_contents == ((void)0))) {
        return ((void)0);
    }
    output_value = json_parse_string_with_comments(file_contents);
    parson_free(file_contents);
    return output_value;
}

int* json_parse_string(char* string_c)
{
    if ((string_c == ((void)0))) {
        return ((void)0);
    }
    if ((((string_c[0] == 239) && (string_c[1] == 187)) && (string_c[2] == 191))) {
        string_c = (string_c + 3);
    }
    return parse_value(((char)(&string_c)), 0);
}

int* json_parse_string_with_comments(char* string_c)
{
    int* result = ((void)0);
    char* string_mutable_copy = ((void)0);
    char* string_mutable_copy_ptr = ((void)0);
    string_mutable_copy = parson_strdup(string_c);
    if ((string_mutable_copy == ((void)0))) {
        return ((void)0);
    }
    remove_comments(string_mutable_copy, "/*", "*/");
    remove_comments(string_mutable_copy, "//", "\n");
    string_mutable_copy_ptr = string_mutable_copy;
    result = parse_value(((char)(&string_mutable_copy_ptr)), 0);
    parson_free(string_mutable_copy);
    return result;
}

int* json_object_get_value(int* object_c, char* name)
{
    if (((object_c == ((void)0)) || (name == ((void)0)))) {
        return ((void)0);
    }
    return json_object_getn_value(object_c, name, sizeof(name));
}

char* json_object_get_string(int* object_c, char* name)
{
    return json_value_get_string(json_object_get_value(object_c, name));
}

int json_object_get_string_len(int* object_c, char* name)
{
    return json_value_get_string_len(json_object_get_value(object_c, name));
}

double json_object_get_number(int* object_c, char* name)
{
    return json_value_get_number(json_object_get_value(object_c, name));
}

int* json_object_get_object(int* object_c, char* name)
{
    return json_value_get_object(json_object_get_value(object_c, name));
}

int* json_object_get_array(int* object_c, char* name)
{
    return json_value_get_array(json_object_get_value(object_c, name));
}

int json_object_get_boolean(int* object_c, char* name)
{
    return json_value_get_boolean(json_object_get_value(object_c, name));
}

int* json_object_dotget_value(int* object_c, char* name)
{
    char* dot_position = strchr(name, 46);
    if ((!dot_position)) {
        return json_object_get_value(object_c, name);
    }
    object_c = json_value_get_object(json_object_getn_value(object_c, name, (dot_position - name)));
    return json_object_dotget_value(object_c, (dot_position + 1));
}

char* json_object_dotget_string(int* object_c, char* name)
{
    return json_value_get_string(json_object_dotget_value(object_c, name));
}

int json_object_dotget_string_len(int* object_c, char* name)
{
    return json_value_get_string_len(json_object_dotget_value(object_c, name));
}

double json_object_dotget_number(int* object_c, char* name)
{
    return json_value_get_number(json_object_dotget_value(object_c, name));
}

int* json_object_dotget_object(int* object_c, char* name)
{
    return json_value_get_object(json_object_dotget_value(object_c, name));
}

int* json_object_dotget_array(int* object_c, char* name)
{
    return json_value_get_array(json_object_dotget_value(object_c, name));
}

int json_object_dotget_boolean(int* object_c, char* name)
{
    return json_value_get_boolean(json_object_dotget_value(object_c, name));
}

int json_object_get_count(int* object_c)
{
    __auto_type _tern_4 = 0;
    if (object_c) {
        _tern_4 = object_c->count;
    }
    return _tern_4;
}

char* json_object_get_name(int* object_c, int index)
{
    if (((object_c == ((void)0)) || (index >= json_object_get_count(object_c)))) {
        return ((void)0);
    }
    return object_c->names[index];
}

int* json_object_get_value_at(int* object_c, int index)
{
    if (((object_c == ((void)0)) || (index >= json_object_get_count(object_c)))) {
        return ((void)0);
    }
    return object_c->values[index];
}

int* json_object_get_wrapping_value(int* object_c)
{
    if ((!object_c)) {
        return ((void)0);
    }
    return object_c->wrapping_value;
}

int json_object_has_value(int* object_c, char* name)
{
    return (json_object_get_value(object_c, name) != ((void)0));
}

int json_object_has_value_of_type(int* object_c, char* name, int type_c)
{
    int* val = json_object_get_value(object_c, name);
    return ((val != ((void)0)) && (json_value_get_type(val) == type_c));
}

int json_object_dothas_value(int* object_c, char* name)
{
    return (json_object_dotget_value(object_c, name) != ((void)0));
}

int json_object_dothas_value_of_type(int* object_c, char* name, int type_c)
{
    int* val = json_object_dotget_value(object_c, name);
    return ((val != ((void)0)) && (json_value_get_type(val) == type_c));
}

int* json_array_get_value(int* array_c, int index)
{
    if (((array_c == ((void)0)) || (index >= json_array_get_count(array_c)))) {
        return ((void)0);
    }
    return array_c->items[index];
}

char* json_array_get_string(int* array_c, int index)
{
    return json_value_get_string(json_array_get_value(array_c, index));
}

int json_array_get_string_len(int* array_c, int index)
{
    return json_value_get_string_len(json_array_get_value(array_c, index));
}

double json_array_get_number(int* array_c, int index)
{
    return json_value_get_number(json_array_get_value(array_c, index));
}

int* json_array_get_object(int* array_c, int index)
{
    return json_value_get_object(json_array_get_value(array_c, index));
}

int* json_array_get_array(int* array_c, int index)
{
    return json_value_get_array(json_array_get_value(array_c, index));
}

int json_array_get_boolean(int* array_c, int index)
{
    return json_value_get_boolean(json_array_get_value(array_c, index));
}

int json_array_get_count(int* array_c)
{
    __auto_type _tern_5 = 0;
    if (array_c) {
        _tern_5 = array_c->count;
    }
    return _tern_5;
}

int* json_array_get_wrapping_value(int* array_c)
{
    if ((!array_c)) {
        return ((void)0);
    }
    return array_c->wrapping_value;
}

int json_value_get_type(int* value)
{
    __auto_type _tern_6 = JSONError;
    if (value) {
        _tern_6 = value->type_c;
    }
    return _tern_6;
}

int* json_value_get_object(int* value)
{
    __auto_type _tern_7 = ((void)0);
    if ((json_value_get_type(value) == JSONObject)) {
        _tern_7 = value->value.object_c;
    }
    return _tern_7;
}

int* json_value_get_array(int* value)
{
    __auto_type _tern_8 = ((void)0);
    if ((json_value_get_type(value) == JSONArray)) {
        _tern_8 = value->value.array_c;
    }
    return _tern_8;
}

JSON_String* json_value_get_string_desc(int* value)
{
    __auto_type _tern_9 = ((void)0);
    if ((json_value_get_type(value) == JSONString)) {
        _tern_9 = (&value->value.string_c);
    }
    return _tern_9;
}

char* json_value_get_string(int* value)
{
    JSON_String* str = json_value_get_string_desc(value);
    __auto_type _tern_10 = ((void)0);
    if (str) {
        _tern_10 = str->chars;
    }
    return _tern_10;
}

int json_value_get_string_len(int* value)
{
    JSON_String* str = json_value_get_string_desc(value);
    __auto_type _tern_11 = 0;
    if (str) {
        _tern_11 = str->length;
    }
    return _tern_11;
}

double json_value_get_number(int* value)
{
    __auto_type _tern_12 = 0;
    if ((json_value_get_type(value) == JSONNumber)) {
        _tern_12 = value->value.number;
    }
    return _tern_12;
}

int json_value_get_boolean(int* value)
{
    __auto_type _tern_13 = (-1);
    if ((json_value_get_type(value) == JSONBoolean)) {
        _tern_13 = value->value.boolean;
    }
    return _tern_13;
}

int* json_value_get_parent(int* value)
{
    __auto_type _tern_14 = ((void)0);
    if (value) {
        _tern_14 = value->parent;
    }
    return _tern_14;
}

void json_value_free(int* value)
{
    if ((json_value_get_type(value) == JSONObject)) {
        json_object_free(value->value.object_c);
    } else {
        if ((json_value_get_type(value) == JSONString)) {
            parson_free(value->value.string_c.chars);
        } else {
            if ((json_value_get_type(value) == JSONArray)) {
                json_array_free(value->value.array_c);
            }
        }
    }
    parson_free(value);
}

int* json_value_init_object(void)
{
    int* new_value = ((int)parson_malloc(sizeof(int)));
    if ((!new_value)) {
        return ((void)0);
    }
    new_value->parent = ((void)0);
    new_value->type_c = JSONObject;
    new_value->value.object_c = json_object_make(new_value);
    if ((!new_value->value.object_c)) {
        parson_free(new_value);
        return ((void)0);
    }
    return new_value;
}

int* json_value_init_array(void)
{
    int* new_value = ((int)parson_malloc(sizeof(int)));
    if ((!new_value)) {
        return ((void)0);
    }
    new_value->parent = ((void)0);
    new_value->type_c = JSONArray;
    new_value->value.array_c = json_array_make(new_value);
    if ((!new_value->value.array_c)) {
        parson_free(new_value);
        return ((void)0);
    }
    return new_value;
}

int* json_value_init_string(char* string_c)
{
    if ((string_c == ((void)0))) {
        return ((void)0);
    }
    return json_value_init_string_with_len(string_c, sizeof(string_c));
}

int* json_value_init_string_with_len(char* string_c, int length)
{
    char* copy = ((void)0);
    int* value = 0;
    if ((string_c == ((void)0))) {
        return ((void)0);
    }
    if ((!is_valid_utf8(string_c, length))) {
        return ((void)0);
    }
    copy = parson_strndup(string_c, length);
    if ((copy == ((void)0))) {
        return ((void)0);
    }
    value = json_value_init_string_no_copy(copy, length);
    if ((value == ((void)0))) {
        parson_free(copy);
    }
    return value;
}

int* json_value_init_number(double number)
{
    int* new_value = ((void)0);
    if ((0 || (0 == (256 | 1024)))) {
        return ((void)0);
    }
    new_value = ((int)parson_malloc(sizeof(int)));
    if ((new_value == ((void)0))) {
        return ((void)0);
    }
    new_value->parent = ((void)0);
    new_value->type_c = JSONNumber;
    new_value->value.number = number;
    return new_value;
}

int* json_value_init_boolean(int boolean)
{
    int* new_value = ((int)parson_malloc(sizeof(int)));
    if ((!new_value)) {
        return ((void)0);
    }
    new_value->parent = ((void)0);
    new_value->type_c = JSONBoolean;
    __auto_type _tern_15 = 0;
    if (boolean) {
        _tern_15 = 1;
    }
    new_value->value.boolean = _tern_15;
    return new_value;
}

int* json_value_init_null(void)
{
    int* new_value = ((int)parson_malloc(sizeof(int)));
    if ((!new_value)) {
        return ((void)0);
    }
    new_value->parent = ((void)0);
    new_value->type_c = JSONNull;
    return new_value;
}

int* json_value_deep_copy(int* value)
{
    int i = 0;
    int* return_value = ((void)0);
    int* temp_value_copy = ((void)0);
    int* temp_value = ((void)0);
    JSON_String* temp_string = ((void)0);
    char* temp_key = ((void)0);
    char* temp_string_copy = ((void)0);
    int* temp_array = ((void)0);
    int* temp_array_copy = ((void)0);
    int* temp_object = ((void)0);
    int* temp_object_copy = ((void)0);
    int res = JSONFailure;
    char* key_copy = ((void)0);
    if ((json_value_get_type(value) == JSONArray)) {
        temp_array = json_value_get_array(value);
        return_value = json_value_init_array();
        if ((return_value == ((void)0))) {
            return ((void)0);
        }
        temp_array_copy = json_value_get_array(return_value);
        i = 0;
        while ((i < json_array_get_count(temp_array))) {
            temp_value = json_array_get_value(temp_array, i);
            temp_value_copy = json_value_deep_copy(temp_value);
            if ((temp_value_copy == ((void)0))) {
                json_value_free(return_value);
                return ((void)0);
            }
            if ((json_array_add(temp_array_copy, temp_value_copy) != JSONSuccess)) {
                json_value_free(return_value);
                json_value_free(temp_value_copy);
                return ((void)0);
            }
            i = (i + 1);
        }
        return return_value;
    } else {
        if ((json_value_get_type(value) == JSONObject)) {
            temp_object = json_value_get_object(value);
            return_value = json_value_init_object();
            if ((!return_value)) {
                return ((void)0);
            }
            temp_object_copy = json_value_get_object(return_value);
            i = 0;
            while ((i < json_object_get_count(temp_object))) {
                temp_key = json_object_get_name(temp_object, i);
                temp_value = json_object_get_value(temp_object, temp_key);
                temp_value_copy = json_value_deep_copy(temp_value);
                if ((!temp_value_copy)) {
                    json_value_free(return_value);
                    return ((void)0);
                }
                key_copy = parson_strdup(temp_key);
                if ((!key_copy)) {
                    json_value_free(temp_value_copy);
                    json_value_free(return_value);
                    return ((void)0);
                }
                res = json_object_add(temp_object_copy, key_copy, temp_value_copy);
                if ((res != JSONSuccess)) {
                    parson_free(key_copy);
                    json_value_free(temp_value_copy);
                    json_value_free(return_value);
                    return ((void)0);
                }
                i = (i + 1);
            }
            return return_value;
        } else {
            if ((json_value_get_type(value) == JSONBoolean)) {
                return json_value_init_boolean(json_value_get_boolean(value));
            } else {
                if ((json_value_get_type(value) == JSONNumber)) {
                    return json_value_init_number(json_value_get_number(value));
                } else {
                    if ((json_value_get_type(value) == JSONString)) {
                        temp_string = json_value_get_string_desc(value);
                        if ((temp_string == ((void)0))) {
                            return ((void)0);
                        }
                        temp_string_copy = parson_strndup(temp_string->chars, temp_string->length);
                        if ((temp_string_copy == ((void)0))) {
                            return ((void)0);
                        }
                        return_value = json_value_init_string_no_copy(temp_string_copy, temp_string->length);
                        if ((return_value == ((void)0))) {
                            parson_free(temp_string_copy);
                        }
                        return return_value;
                    } else {
                        if ((json_value_get_type(value) == JSONNull)) {
                            return json_value_init_null();
                        } else {
                            if ((json_value_get_type(value) == JSONError)) {
                                return ((void)0);
                            } else {
                                return ((void)0);
                            }
                        }
                    }
                }
            }
        }
    }
}

int json_serialization_size(int* value)
{
    char num_buf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int res = json_serialize_to_buffer_r(value, ((void)0), 0, 0, num_buf);
    __auto_type _tern_16 = (((int)res) + 1);
    if ((res < 0)) {
        _tern_16 = 0;
    }
    return _tern_16;
}

int json_serialize_to_buffer(int* value, char* buf, int buf_size_in_bytes)
{
    int written = (-1);
    int needed_size_in_bytes = json_serialization_size(value);
    if (((needed_size_in_bytes == 0) || (buf_size_in_bytes < needed_size_in_bytes))) {
        return JSONFailure;
    }
    written = json_serialize_to_buffer_r(value, buf, 0, 0, ((void)0));
    if ((written < 0)) {
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_serialize_to_file(int* value, char* filename)
{
    int return_code = JSONSuccess;
    int* fp = ((void)0);
    char* serialized_string = json_serialize_to_string(value);
    if ((serialized_string == ((void)0))) {
        return JSONFailure;
    }
    fp = fopen(filename, "w");
    if ((fp == ((void)0))) {
        json_free_serialized_string(serialized_string);
        return JSONFailure;
    }
    if ((fputs(serialized_string, fp) == (-1))) {
        return_code = JSONFailure;
    }
    if ((fclose(fp) == (-1))) {
        return_code = JSONFailure;
    }
    json_free_serialized_string(serialized_string);
    return return_code;
}

char* json_serialize_to_string(int* value)
{
    int serialization_result = JSONFailure;
    int buf_size_bytes = json_serialization_size(value);
    char* buf = ((void)0);
    if ((buf_size_bytes == 0)) {
        return ((void)0);
    }
    buf = ((char)parson_malloc(buf_size_bytes));
    if ((buf == ((void)0))) {
        return ((void)0);
    }
    serialization_result = json_serialize_to_buffer(value, buf, buf_size_bytes);
    if ((serialization_result != JSONSuccess)) {
        json_free_serialized_string(buf);
        return ((void)0);
    }
    return buf;
}

int json_serialization_size_pretty(int* value)
{
    char num_buf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int res = json_serialize_to_buffer_r(value, ((void)0), 0, 1, num_buf);
    __auto_type _tern_17 = (((int)res) + 1);
    if ((res < 0)) {
        _tern_17 = 0;
    }
    return _tern_17;
}

int json_serialize_to_buffer_pretty(int* value, char* buf, int buf_size_in_bytes)
{
    int written = (-1);
    int needed_size_in_bytes = json_serialization_size_pretty(value);
    if (((needed_size_in_bytes == 0) || (buf_size_in_bytes < needed_size_in_bytes))) {
        return JSONFailure;
    }
    written = json_serialize_to_buffer_r(value, buf, 0, 1, ((void)0));
    if ((written < 0)) {
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_serialize_to_file_pretty(int* value, char* filename)
{
    int return_code = JSONSuccess;
    int* fp = ((void)0);
    char* serialized_string = json_serialize_to_string_pretty(value);
    if ((serialized_string == ((void)0))) {
        return JSONFailure;
    }
    fp = fopen(filename, "w");
    if ((fp == ((void)0))) {
        json_free_serialized_string(serialized_string);
        return JSONFailure;
    }
    if ((fputs(serialized_string, fp) == (-1))) {
        return_code = JSONFailure;
    }
    if ((fclose(fp) == (-1))) {
        return_code = JSONFailure;
    }
    json_free_serialized_string(serialized_string);
    return return_code;
}

char* json_serialize_to_string_pretty(int* value)
{
    int serialization_result = JSONFailure;
    int buf_size_bytes = json_serialization_size_pretty(value);
    char* buf = ((void)0);
    if ((buf_size_bytes == 0)) {
        return ((void)0);
    }
    buf = ((char)parson_malloc(buf_size_bytes));
    if ((buf == ((void)0))) {
        return ((void)0);
    }
    serialization_result = json_serialize_to_buffer_pretty(value, buf, buf_size_bytes);
    if ((serialization_result != JSONSuccess)) {
        json_free_serialized_string(buf);
        return ((void)0);
    }
    return buf;
}

void json_free_serialized_string(char* string_c)
{
    parson_free(string_c);
}

int json_array_remove(int* array_c, int ix)
{
    int to_move_bytes = 0;
    if (((array_c == ((void)0)) || (ix >= json_array_get_count(array_c)))) {
        return JSONFailure;
    }
    json_value_free(json_array_get_value(array_c, ix));
    to_move_bytes = (((json_array_get_count(array_c) - 1) - ix) * sizeof(int));
    memmove((array_c->items + ix), ((array_c->items + ix) + 1), to_move_bytes);
    array_c->count = (array_c->count - 1);
    return JSONSuccess;
}

int json_array_replace_value(int* array_c, int ix, int* value)
{
    if (((((array_c == ((void)0)) || (value == ((void)0))) || (value->parent != ((void)0))) || (ix >= json_array_get_count(array_c)))) {
        return JSONFailure;
    }
    json_value_free(json_array_get_value(array_c, ix));
    value->parent = json_array_get_wrapping_value(array_c);
    array_c->items[ix] = value;
    return JSONSuccess;
}

int json_array_replace_string(int* array_c, int i, char* string_c)
{
    int* value = json_value_init_string(string_c);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_replace_value(array_c, i, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_replace_string_with_len(int* array_c, int i, char* string_c, int len)
{
    int* value = json_value_init_string_with_len(string_c, len);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_replace_value(array_c, i, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_replace_number(int* array_c, int i, double number)
{
    int* value = json_value_init_number(number);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_replace_value(array_c, i, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_replace_boolean(int* array_c, int i, int boolean)
{
    int* value = json_value_init_boolean(boolean);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_replace_value(array_c, i, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_replace_null(int* array_c, int i)
{
    int* value = json_value_init_null();
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_replace_value(array_c, i, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_clear(int* array_c)
{
    int i = 0;
    if ((array_c == ((void)0))) {
        return JSONFailure;
    }
    i = 0;
    while ((i < json_array_get_count(array_c))) {
        json_value_free(json_array_get_value(array_c, i));
        i = (i + 1);
    }
    array_c->count = 0;
    return JSONSuccess;
}

int json_array_append_value(int* array_c, int* value)
{
    if ((((array_c == ((void)0)) || (value == ((void)0))) || (value->parent != ((void)0)))) {
        return JSONFailure;
    }
    return json_array_add(array_c, value);
}

int json_array_append_string(int* array_c, char* string_c)
{
    int* value = json_value_init_string(string_c);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_append_value(array_c, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_append_string_with_len(int* array_c, char* string_c, int len)
{
    int* value = json_value_init_string_with_len(string_c, len);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_append_value(array_c, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_append_number(int* array_c, double number)
{
    int* value = json_value_init_number(number);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_append_value(array_c, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_append_boolean(int* array_c, int boolean)
{
    int* value = json_value_init_boolean(boolean);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_append_value(array_c, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_array_append_null(int* array_c)
{
    int* value = json_value_init_null();
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_array_append_value(array_c, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_set_value(int* object_c, char* name, int* value)
{
    uint64_t hash = 0;
    int found = 0;
    int cell_ix = 0;
    int item_ix = 0;
    int* old_value = ((void)0);
    char* key_copy = ((void)0);
    if (((((!object_c) || (!name)) || (!value)) || value->parent)) {
        return JSONFailure;
    }
    hash = hash_string(name, sizeof(name));
    found = 0;
    cell_ix = json_object_get_cell_ix(object_c, name, sizeof(name), hash, (&found));
    if (found) {
        item_ix = object_c->cells[cell_ix];
        old_value = object_c->values[item_ix];
        json_value_free(old_value);
        object_c->values[item_ix] = value;
        value->parent = json_object_get_wrapping_value(object_c);
        return JSONSuccess;
    }
    if ((object_c->count >= object_c->item_capacity)) {
        int res = json_object_grow_and_rehash(object_c);
        if ((res != JSONSuccess)) {
            return JSONFailure;
        }
        cell_ix = json_object_get_cell_ix(object_c, name, sizeof(name), hash, (&found));
    }
    key_copy = parson_strdup(name);
    if ((!key_copy)) {
        return JSONFailure;
    }
    object_c->names[object_c->count] = key_copy;
    object_c->cells[cell_ix] = object_c->count;
    object_c->values[object_c->count] = value;
    object_c->cell_ixs[object_c->count] = cell_ix;
    object_c->hashes[object_c->count] = hash;
    object_c->count = (object_c->count + 1);
    value->parent = json_object_get_wrapping_value(object_c);
    return JSONSuccess;
}

int json_object_set_string(int* object_c, char* name, char* string_c)
{
    int* value = json_value_init_string(string_c);
    int status = json_object_set_value(object_c, name, value);
    if ((status != JSONSuccess)) {
        json_value_free(value);
    }
    return status;
}

int json_object_set_string_with_len(int* object_c, char* name, char* string_c, int len)
{
    int* value = json_value_init_string_with_len(string_c, len);
    int status = json_object_set_value(object_c, name, value);
    if ((status != JSONSuccess)) {
        json_value_free(value);
    }
    return status;
}

int json_object_set_number(int* object_c, char* name, double number)
{
    int* value = json_value_init_number(number);
    int status = json_object_set_value(object_c, name, value);
    if ((status != JSONSuccess)) {
        json_value_free(value);
    }
    return status;
}

int json_object_set_boolean(int* object_c, char* name, int boolean)
{
    int* value = json_value_init_boolean(boolean);
    int status = json_object_set_value(object_c, name, value);
    if ((status != JSONSuccess)) {
        json_value_free(value);
    }
    return status;
}

int json_object_set_null(int* object_c, char* name)
{
    int* value = json_value_init_null();
    int status = json_object_set_value(object_c, name, value);
    if ((status != JSONSuccess)) {
        json_value_free(value);
    }
    return status;
}

int json_object_dotset_value(int* object_c, char* name, int* value)
{
    char* dot_pos = ((void)0);
    int* temp_value = ((void)0);
    int* new_value = ((void)0);
    int* temp_object = ((void)0);
    int* new_object = ((void)0);
    int status = JSONFailure;
    int name_len = 0;
    char* name_copy = ((void)0);
    if ((((object_c == ((void)0)) || (name == ((void)0))) || (value == ((void)0)))) {
        return JSONFailure;
    }
    dot_pos = strchr(name, 46);
    if ((dot_pos == ((void)0))) {
        return json_object_set_value(object_c, name, value);
    }
    name_len = (dot_pos - name);
    temp_value = json_object_getn_value(object_c, name, name_len);
    if (temp_value) {
        if ((json_value_get_type(temp_value) != JSONObject)) {
            return JSONFailure;
        }
        temp_object = json_value_get_object(temp_value);
        return json_object_dotset_value(temp_object, (dot_pos + 1), value);
    }
    new_value = json_value_init_object();
    if ((new_value == ((void)0))) {
        return JSONFailure;
    }
    new_object = json_value_get_object(new_value);
    status = json_object_dotset_value(new_object, (dot_pos + 1), value);
    if ((status != JSONSuccess)) {
        json_value_free(new_value);
        return JSONFailure;
    }
    name_copy = parson_strndup(name, name_len);
    if ((!name_copy)) {
        json_object_dotremove_internal(new_object, (dot_pos + 1), 0);
        json_value_free(new_value);
        return JSONFailure;
    }
    status = json_object_add(object_c, name_copy, new_value);
    if ((status != JSONSuccess)) {
        parson_free(name_copy);
        json_object_dotremove_internal(new_object, (dot_pos + 1), 0);
        json_value_free(new_value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_dotset_string(int* object_c, char* name, char* string_c)
{
    int* value = json_value_init_string(string_c);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_object_dotset_value(object_c, name, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_dotset_string_with_len(int* object_c, char* name, char* string_c, int len)
{
    int* value = json_value_init_string_with_len(string_c, len);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_object_dotset_value(object_c, name, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_dotset_number(int* object_c, char* name, double number)
{
    int* value = json_value_init_number(number);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_object_dotset_value(object_c, name, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_dotset_boolean(int* object_c, char* name, int boolean)
{
    int* value = json_value_init_boolean(boolean);
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_object_dotset_value(object_c, name, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_dotset_null(int* object_c, char* name)
{
    int* value = json_value_init_null();
    if ((value == ((void)0))) {
        return JSONFailure;
    }
    if ((json_object_dotset_value(object_c, name, value) != JSONSuccess)) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

int json_object_remove(int* object_c, char* name)
{
    return json_object_remove_internal(object_c, name, 1);
}

int json_object_dotremove(int* object_c, char* name)
{
    return json_object_dotremove_internal(object_c, name, 1);
}

int json_object_clear(int* object_c)
{
    int i = 0;
    if ((object_c == ((void)0))) {
        return JSONFailure;
    }
    i = 0;
    while ((i < json_object_get_count(object_c))) {
        parson_free(object_c->names[i]);
        object_c->names[i] = ((void)0);
        json_value_free(object_c->values[i]);
        object_c->values[i] = ((void)0);
        i = (i + 1);
    }
    object_c->count = 0;
    i = 0;
    while ((i < object_c->cell_capacity)) {
        object_c->cells[i] = ((int)(-1));
        i = (i + 1);
    }
    return JSONSuccess;
}

int json_validate(int* schema, int* value)
{
    int* temp_schema_value = ((void)0);
    int* temp_value = ((void)0);
    int* schema_array = ((void)0);
    int* value_array = ((void)0);
    int* schema_object = ((void)0);
    int* value_object = ((void)0);
    int schema_type = JSONError;
    int value_type = JSONError;
    char* key = ((void)0);
    int i = 0;
    int count = 0;
    if (((schema == ((void)0)) || (value == ((void)0)))) {
        return JSONFailure;
    }
    schema_type = json_value_get_type(schema);
    value_type = json_value_get_type(value);
    if (((schema_type != value_type) && (schema_type != JSONNull))) {
        return JSONFailure;
    }
    if ((schema_type == JSONArray)) {
        schema_array = json_value_get_array(schema);
        value_array = json_value_get_array(value);
        count = json_array_get_count(schema_array);
        if ((count == 0)) {
            return JSONSuccess;
        }
        temp_schema_value = json_array_get_value(schema_array, 0);
        i = 0;
        while ((i < json_array_get_count(value_array))) {
            temp_value = json_array_get_value(value_array, i);
            if ((json_validate(temp_schema_value, temp_value) != JSONSuccess)) {
                return JSONFailure;
            }
            i = (i + 1);
        }
        return JSONSuccess;
    } else {
        if ((schema_type == JSONObject)) {
            schema_object = json_value_get_object(schema);
            value_object = json_value_get_object(value);
            count = json_object_get_count(schema_object);
            if ((count == 0)) {
                return JSONSuccess;
            } else {
                if ((json_object_get_count(value_object) < count)) {
                    return JSONFailure;
                }
            }
            i = 0;
            while ((i < count)) {
                key = json_object_get_name(schema_object, i);
                temp_schema_value = json_object_get_value(schema_object, key);
                temp_value = json_object_get_value(value_object, key);
                if ((temp_value == ((void)0))) {
                    return JSONFailure;
                }
                if ((json_validate(temp_schema_value, temp_value) != JSONSuccess)) {
                    return JSONFailure;
                }
                i = (i + 1);
            }
            return JSONSuccess;
        } else {
            if (((((schema_type == JSONString) || (schema_type == JSONNumber)) || (schema_type == JSONBoolean)) || (schema_type == JSONNull))) {
                return JSONSuccess;
            } else {
                return JSONFailure;
            }
        }
    }
}

int json_value_equals(int* a, int* b)
{
    int* a_object = ((void)0);
    int* b_object = ((void)0);
    int* a_array = ((void)0);
    int* b_array = ((void)0);
    JSON_String* a_string = ((void)0);
    JSON_String* b_string = ((void)0);
    char* key = ((void)0);
    int a_count = 0;
    int b_count = 0;
    int i = 0;
    int a_type = 0;
    int b_type = 0;
    a_type = json_value_get_type(a);
    b_type = json_value_get_type(b);
    if ((a_type != b_type)) {
        return 0;
    }
    if ((a_type == JSONArray)) {
        a_array = json_value_get_array(a);
        b_array = json_value_get_array(b);
        a_count = json_array_get_count(a_array);
        b_count = json_array_get_count(b_array);
        if ((a_count != b_count)) {
            return 0;
        }
        i = 0;
        while ((i < a_count)) {
            if ((!json_value_equals(json_array_get_value(a_array, i), json_array_get_value(b_array, i)))) {
                return 0;
            }
            i = (i + 1);
        }
        return 1;
    } else {
        if ((a_type == JSONObject)) {
            a_object = json_value_get_object(a);
            b_object = json_value_get_object(b);
            a_count = json_object_get_count(a_object);
            b_count = json_object_get_count(b_object);
            if ((a_count != b_count)) {
                return 0;
            }
            i = 0;
            while ((i < a_count)) {
                key = json_object_get_name(a_object, i);
                if ((!json_value_equals(json_object_get_value(a_object, key), json_object_get_value(b_object, key)))) {
                    return 0;
                }
                i = (i + 1);
            }
            return 1;
        } else {
            if ((a_type == JSONString)) {
                a_string = json_value_get_string_desc(a);
                b_string = json_value_get_string_desc(b);
                if (((a_string == ((void)0)) || (b_string == ((void)0)))) {
                    return 0;
                }
                return ((a_string->length == b_string->length) && (memcmp(a_string->chars, b_string->chars, a_string->length) == 0));
            } else {
                if ((a_type == JSONBoolean)) {
                    return (json_value_get_boolean(a) == json_value_get_boolean(b));
                } else {
                    if ((a_type == JSONNumber)) {
                        return (fabs((json_value_get_number(a) - json_value_get_number(b))) < 1e-06);
                    } else {
                        if ((a_type == JSONError)) {
                            return 1;
                        } else {
                            if ((a_type == JSONNull)) {
                                return 1;
                            } else {
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

int json_type(int* value)
{
    return json_value_get_type(value);
}

int* json_object(int* value)
{
    return json_value_get_object(value);
}

int* json_array(int* value)
{
    return json_value_get_array(value);
}

char* json_string(int* value)
{
    return json_value_get_string(value);
}

int json_string_len(int* value)
{
    return json_value_get_string_len(value);
}

double json_number(int* value)
{
    return json_value_get_number(value);
}

int json_boolean(int* value)
{
    return json_value_get_boolean(value);
}

void json_set_allocation_functions(int malloc_fun, int free_fun)
{
    parson_malloc = malloc_fun;
    parson_free = free_fun;
}

void json_set_escape_slashes(int escape_slashes)
{
    parson_escape_slashes = escape_slashes;
}

void json_set_float_serialization_format(char* format)
{
    if (parson_float_format) {
        parson_free(parson_float_format);
        parson_float_format = ((void)0);
    }
    if ((!format)) {
        parson_float_format = ((void)0);
        return;
    }
    parson_float_format = parson_strdup(format);
}

void json_set_number_serialization_function(int func)
{
    parson_number_serialization_function = func;
}


