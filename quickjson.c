#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


enum qjson_type {
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

uint32_t qjson_dump_string(char *str, char *buf, int len) {
    if(str == NULL || len < 2){
        return 0;
    }
    int i = 0;
    buf[i++] = '\"';

    for(char *pos = str; *pos != '\0' && i < len-2; pos++){
        switch(*pos) {
        case '\"':
            buf[i++] = '\\';
            buf[i++] = '\"';
        case '\\':
            buf[i++] = '\\';
            buf[i++] = '\\';
        case '\b':
            buf[i++] = '\\';
            buf[i++] = 'b';
        case '\f':
            buf[i++] = '\\';
            buf[i++] = 'f';
        case '\n':
            buf[i++] = '\\';
            buf[i++] = 'n';
        case '\r':
            buf[i++] = '\\';
            buf[i++] = 'r';
        case '\t':
            buf[i++] = '\\';
            buf[i++] = 't';
        default:
            buf[i++] = *pos;
        }
    }
    buf[i++] = '\"';
    buf[i] = '\0';
    return i;
}

uint32_t qjson_dump_array(qjson_array_t *arr, char *buf, uint32_t len);

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

uint32_t qjson_dump_array(qjson_array_t *arr, char *buf, uint32_t len) {
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



int main() {
    const char * strlist[] = {
        "linux",
        "firefox",
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

    char buf[1024];
    int bytes = qjson_dump_array(arr, buf, 1024);

    printf("buf: %s, bytes: %d, strlen: %d\n", buf, bytes, strlen(buf));
    return 0;
}

