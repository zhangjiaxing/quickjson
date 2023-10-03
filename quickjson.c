#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define BUFLEN (4*1024)

#define SUCCESS 0
#define FAILURE 1

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

enum qjson_type {
    QJSON_INVALID,
    QJSON_OBJECT,
    QJSON_ARRAY,
    QJSON_STRING,
    QJSON_INT,
    QJSON_FLOAT,
    QJSON_BOOL,
    QJSON_NULL,
};
typedef enum qjson_type qjson_type_t;

struct qjson_value;
struct qjson_array;
struct qjson_object;

struct qjson_value {
    qjson_type_t json_type;
    union {
        int64_t interger;
        double fraction;
        char *str;
        struct qjson_array *array;
        struct qjson_object *object;
    }v;
};
typedef struct qjson_value qjson_value_t;


struct qjson_pair {
    char *key;
    struct qjson_value value;
    struct qjson_pair *next;
};
typedef struct qjson_pair qjson_pair_t;

struct qjson_object {
    struct qjson_pair head;
};
typedef struct qjson_object qjson_object_t;

struct qjson_array_item {
    struct qjson_value value;
    struct qjson_array_item *next;
};
typedef struct qjson_array_item qjson_array_item_t;

struct qjson_array {
    struct qjson_array_item head;
};
typedef struct qjson_array qjson_array_t;


qjson_value_t *qjson_create_int(int64_t i) {
    qjson_value_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->json_type = QJSON_INT;
    self->v.interger = i;
    return self;
}

qjson_value_t *qjson_create_float(double f) {
    qjson_value_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->json_type = QJSON_FLOAT;
    self->v.fraction = f;
    return self;
}

qjson_value_t *qjson_create_str(const char *str) {
    qjson_value_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->json_type = QJSON_STRING;
    self->v.str = strdup(str);
    return self;
}


uint32_t str_espace_len(const char *str) {
    uint32_t len = 0;
    for(const char *end = str; *end != '\0'; end++){
        switch(*end) {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            len += 2;
            break;
        default:
            len++;
        };
    }
    return len;
}


uint32_t str_escape(const char *from, char *to, uint32_t len) {
    uint32_t from_pos = 0;
    uint32_t to_pos = 0;
    uint32_t last = len-1;

    if(from == NULL || to == NULL || len == 0){
        return 0;
    }

    while(from[from_pos] != '\0' && to_pos < last) {
        switch(from[from_pos]) {
        case '\"':
            to[to_pos++] = '\\';
            to[to_pos++] = '\"';
            break;
        case '\\':
            to[to_pos++] = '\\';
            to[to_pos++] = '\\';
            break;
        case '\b':
            to[to_pos++] = '\\';
            to[to_pos++] = 'b';
            break;
        case '\f':
            to[to_pos++] = '\\';
            to[to_pos++] = 'f';
            break;
        case '\n':
            to[to_pos++] = '\\';
            to[to_pos++] = 'n';
            break;
        case '\r':
            to[to_pos++] = '\\';
            to[to_pos++] = 'r';
            break;
        case '\t':
            to[to_pos++] = '\\';
            to[to_pos++] = 't';
            break;
        default:
            to[to_pos++] = from[from_pos];
            break;
        }
        from_pos++;
    }
    to_pos = to_pos < last? to_pos: last;
    to[to_pos] = '\0';
    return to_pos;
}

uint32_t str_unescape(const char *from, char *to, uint32_t len, const char **parse_end) {
    uint32_t from_pos = 0;
    uint32_t to_pos = 0;
    uint32_t last = len-1;

    if(from == NULL || to == NULL || len == 0){
        return 0;
    }

    while(from[from_pos] != '\0' && from[from_pos] != '\"' && to_pos < last) {
        if(from[from_pos] == '\\') {
            switch(from[from_pos++]) {
            case '\"':
                to[to_pos++] = '\\';
                to[to_pos++] = '\"';
                break;
            case '\\':
                to[to_pos++] = '\\';
                to[to_pos++] = '\\';
                break;
            case 'b':
                to[to_pos++] = '\\';
                to[to_pos++] = '\b';
                break;
            case 'f':
                to[to_pos++] = '\\';
                to[to_pos++] = '\f';
                break;
            case 'n':
                to[to_pos++] = '\\';
                to[to_pos++] = '\n';
                break;
            case 'r':
                to[to_pos++] = '\\';
                to[to_pos++] = '\r';
                break;
            case 't':
                to[to_pos++] = '\\';
                to[to_pos++] = '\t';
                break;
            default:
                to[to_pos++] = from[from_pos];
            }
        } else {
            to[to_pos++] = from[from_pos++];
        }
    }

    if(parse_end != NULL) {
        *parse_end = &from[from_pos];
    }
    to_pos = to_pos < last? to_pos: last;
    to[to_pos] = '\0';
    return to_pos;
}


uint32_t qjson_dump_string(char *str, char *buf, int len) {
    if(str == NULL || len < 3){
        return 0;
    }
    int i = 0;
    buf[i++] = '\"';
    i += str_escape(str, &buf[i], len-3);
    buf[i++] = '\"';
    buf[i] = '\0';
    return i;
}

uint32_t qjson_dump_array(const qjson_array_t *arr, char *buf, uint32_t len);

uint32_t qjson_dump(qjson_value_t *value, char *buf, uint32_t len) {
    qjson_type_t json_type = value->json_type;
    switch (json_type) {
    case QJSON_INT:
        return snprintf(buf, len, "%lld", value->v.interger);
    case QJSON_FLOAT:
        return snprintf(buf, len, "%lf", value->v.fraction);
    case QJSON_NULL:
        return snprintf(buf, len, "null");
    case QJSON_STRING:
        return qjson_dump_string(value->v.str, buf, len);
    case QJSON_ARRAY:
        return qjson_dump_array(value->v.array, buf, len);
    default:
        return 0;
    }
}


uint32_t qjson_dump_array(const qjson_array_t *arr, char *buf, uint32_t len) {
    if(arr == NULL || len < 2){
        return 0;
    }

    int i = 0;
    buf[i++] = '[';
    qjson_array_item_t *item = arr->head.next;
    while(item != NULL) {
        i += qjson_dump(&item->value, buf+i, len-i);
        if(item->next != NULL) {
            buf[i++] = ',';
            buf[i++] = ' ';
        }
        item = item->next;
    }
    buf[i++] = ']';
    buf[i] = '\0';
    return i;
}


uint32_t qjson_load_string(const char *str, qjson_value_t **value, const char **parse_end) {
    const char *pos = str;
    while(isspace(*pos)) {
        pos++;
    }

    if(*pos++ != '\"') {
        *parse_end = pos;
        return FAILURE;
    }

    int32_t len = strlen(pos);
    char *buf = malloc(len);
    int32_t nwrite = str_unescape(pos, buf, len, parse_end);
    pos = *parse_end;

    if(*pos != '\"') {
        pos++;
        *parse_end = pos;
        return FAILURE;
    }

    *value = qjson_create_str(buf);
    free(buf);
    return SUCCESS;
}

uint32_t qjson_load(const char *buf, uint32_t len, qjson_value_t *value) {
    const char *pos = buf;

    return pos-buf;
}

qjson_array_t *qjson_create_array() {
    qjson_array_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(sizeof(*self)));
    return self;
}

qjson_array_t *qjson_array_append(qjson_array_t *arr, const qjson_value_t *e) {
    qjson_array_item_t *cur = &arr->head;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    qjson_array_item_t *item = malloc(sizeof(*item));
    item->value = *e;
    item->next = NULL;

    cur->next = item;
    return arr;
}

uint32_t qjson_array_length(qjson_array_t *arr) {
    qjson_array_item_t *cur = arr->head.next;
    uint32_t count = 0;

    while(cur != NULL) {
        cur = cur->next;
        count++;
    }
    return count;
}



void test_dump_array() {
    const char * strlist[] = {
        "linux",
        "fire\tfox",
        "emacs",
        "vim",
        "bash",
        "zsh",
        "fish",
        NULL,
    };
    const char **p = &strlist[0];

    qjson_array_t *arr = qjson_create_array();

    while(*p != NULL) {
        qjson_value_t *value = qjson_create_str(*p++);
        qjson_array_append(arr, value);
    }

    qjson_value_t *value = qjson_create_int(12345);
    qjson_array_append(arr, value);

    value = qjson_create_float(12345.6789);
    qjson_array_append(arr, value);

    char buf[BUFLEN];
    int bytes = qjson_dump_array(arr, buf, BUFLEN);

    printf("buf: %s, bytes: %d, strlen: %d\n", buf, bytes, strlen(buf));
}


void test_load_string() {
    printf("in [%s]\n", __FUNCTION__);

    qjson_value_t *json_value;
    const char *parse_end = NULL;
    const char *str = "  \t\"nihao\t\\hello world\"string end";
    uint32_t ret = qjson_load_string(str, &json_value, &parse_end);

    printf("ret: [%d]\n", ret);

    char buf[BUFLEN];
    qjson_dump(json_value, buf, BUFLEN);
    printf("json_value: [%s]\n", buf);
    printf("parse_end: [%s]\n", parse_end);
}

void test_unescape() {
    printf("in [%s]\n", __FUNCTION__);

    const char *str = "nihao\t\\\bhello\"END";
    char buf[BUFLEN];
    const char *parse_end;
    uint32_t ret = str_unescape(str, buf, BUFLEN, &parse_end);
    printf("ret = %d \n", ret);
    printf("buf = %s \n", buf);
    printf("parse_end = %s\n", parse_end);

}

int main() {
    test_dump_array();

    test_load_string();
    test_unescape();
    return 0;
}
