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
        int64_t integer;
        bool boolean;
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


qjson_array_t *qjson_create_array();
qjson_array_t *qjson_array_append(qjson_array_t *arr, const qjson_value_t *e);
qjson_object_t *qjson_create_object();
qjson_object_t *qjson_object_append(qjson_object_t *obj, const char *key, const qjson_value_t *e);

uint32_t qjson_load(const char *str, qjson_value_t **value, const char **parse_end);

qjson_value_t *qjson_create_int(int64_t i) {
    qjson_value_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->json_type = QJSON_INT;
    self->v.integer = i;
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

qjson_value_t *qjson_create_bool(bool value) {
    qjson_value_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->json_type = QJSON_BOOL;
    self->v.boolean = value;
    return self;
}

qjson_value_t *qjson_create_null() {
    qjson_value_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->json_type = QJSON_NULL;
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

int32_t qjson_strlen(const char *str) {
    const char *end = str;
    if(*end == '\"'){
        end++;
    }else{
        return -1;
    }

    while(*end != '\0' && *end != '\"') {
        if(*end == '\\') {
            end++;
        }
        end++;
    }

    return end - str;
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
    uint32_t last = len-2;

    if(from == NULL || to == NULL || len == 0){
        return 0;
    }

    if(from[from_pos] != '\"') {
        *parse_end = from;
        return 0;
    }
    from_pos++;

    while(from[from_pos] != '\0' && from[from_pos] != '\"' && to_pos < last) {
        if(from[from_pos] == '\\') {
            from_pos++;
            switch(from[from_pos]) {
            case '\"':
                to[to_pos++] = '\"';
                break;
            case '\\':
                to[to_pos++] = '\\';
                break;
            case 'b':
                to[to_pos++] = '\b';
                break;
            case 'f':
                to[to_pos++] = '\f';
                break;
            case 'n':
                to[to_pos++] = '\n';
                break;
            case 'r':
                to[to_pos++] = '\r';
                break;
            case 't':
                to[to_pos++] = '\t';
                break;
            default:
                to[to_pos++] = from[from_pos];
            }
        } else {
            to[to_pos++] = from[from_pos];
        }
        from_pos++;
    }
    from_pos++;

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

uint32_t qjson_dump_bool(qjson_value_t *value, char *buf, int len) {
    if(value == NULL || len < sizeof("false")){
        return 0;
    }

    char *result = value->v.boolean?"true": "false";
    strncpy(buf, result, len);
    return strlen(result);
}

uint32_t qjson_dump_null(qjson_value_t *value, char *buf, int len) {
    if(value == NULL || len < sizeof("null")){
        return 0;
    }

    strncpy(buf, "null", len);
    return strlen("null");
}


uint32_t qjson_dump_array(const qjson_array_t *arr, char *buf, uint32_t len);
uint32_t qjson_dump_object(const qjson_object_t *obj, char *buf, uint32_t len);

uint32_t qjson_dump(qjson_value_t *value, char *buf, uint32_t len) {
    qjson_type_t json_type = value->json_type;
    switch (json_type) {
    case QJSON_INT:
        return snprintf(buf, len, "%lld", value->v.integer);
    case QJSON_FLOAT:
        return snprintf(buf, len, "%lf", value->v.fraction);
    case QJSON_STRING:
        return qjson_dump_string(value->v.str, buf, len);
    case QJSON_ARRAY:
        return qjson_dump_array(value->v.array, buf, len);
    case QJSON_OBJECT:
        return qjson_dump_object(value->v.object, buf, len);
    case QJSON_NULL:
        return qjson_dump_null(value, buf, len);
    case QJSON_BOOL:
        return qjson_dump_bool(value, buf, len);
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

uint32_t qjson_dump_object(const qjson_object_t *obj, char *buf, uint32_t len) {
    if(obj == NULL || len < 2){
        return 0;
    }

    int i = 0;
    buf[i++] = '{';
    qjson_pair_t *pair = obj->head.next;
    while(pair != NULL) {
        i += qjson_dump_string(pair->key, buf+i, len-i);
        if(i + 2 < len) {
            buf[i++] = ':';
            buf[i++] = ' ';
        } else {
            return 0;
        }

        i += qjson_dump(&pair->value, buf+i, len-i);

        if(pair->next != NULL && i + 2 < len) {
            buf[i++] = ',';
            buf[i++] = ' ';
        }
        pair = pair->next;
    }

    if(i + 2 < len) {
        buf[i++] = '}';
        buf[i] = '\0';
        return i;
    } else {
        return 0;
    }
}


uint32_t qjson_load_string(const char *str, qjson_value_t **value, const char **parse_end) {
    const char *pos = str;
    while(isspace(*pos)) {
        pos++;
    }

    if(*pos != '\"') {
        *parse_end = pos;
        return FAILURE;
    }

    int32_t len = strlen(pos); //TODO
    char *buf = malloc(len);
    str_unescape(pos, buf, len, parse_end);
    pos = *parse_end;

    if(pos[-1] == '\"') {
        *value = qjson_create_str(buf);
        free(buf);
        return SUCCESS;
    }

    return FAILURE;
}

uint32_t qjson_load_bool(const char *str, qjson_value_t **value, const char **parse_end) {
    bool result = false;
    int parse_len = 0;
    int false_len = strlen("false");
    int true_len = strlen("true");

    if(strncmp(str, "false", false_len)  == 0){
        result = false;
        parse_len = false_len;
    } else if (strncmp(str, "true", true_len) ==0 ) {
        result = true;
        parse_len = true_len;
    } else {
        *parse_end = str;
        *value = NULL;
        return FAILURE;
    }

    qjson_value_t *boolean = malloc(sizeof(qjson_value_t));
    memset(boolean, 0, sizeof(qjson_value_t));
    boolean->json_type = QJSON_BOOL;
    boolean->v.boolean = result;
    *value = boolean;
    *parse_end = str + parse_len;
    return SUCCESS;
}

uint32_t qjson_load_null(const char *str, qjson_value_t **value, const char **parse_end) {
    int null_len = strlen("null");
    if(strncmp(str, "null", null_len) == 0){

        qjson_value_t *null = malloc(sizeof(qjson_value_t));
        memset(null, 0, sizeof(qjson_value_t));
        null->json_type = QJSON_NULL;
        *value = null;
        *parse_end = str + null_len;
        return SUCCESS;
    } else {
        printf("nullstr:%s\n", str);
    }
    *value = NULL;
    *parse_end = str;
    return FAILURE;
}


uint32_t qjson_load_integer(const char *str, int64_t *integer, const char **parse_end) {
#define INTEGER_STR_MAX_LEN 20
    int rpos = 0;
    int wpos = 0;
    char integer_str[INTEGER_STR_MAX_LEN + 1];

    while((isdigit(str[rpos]) || str[rpos] == '-' ) && wpos < INTEGER_STR_MAX_LEN) {
        integer_str[wpos++] = str[rpos++];
    }
    integer_str[wpos] = '\0';
    *integer = strtoll(integer_str, NULL, 10);
    //sscanf(integer_str, "%lld", integer);
    *parse_end = str+wpos;

    if(wpos != 0) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

uint32_t qjson_load_fraction(const char *str, uint64_t *fraction, const char **parse_end) {
    if(*str != '.') {
        *fraction = 0;
        *parse_end = str;
        return FAILURE;
    }

    uint32_t rpos = 1;
    uint32_t wpos = 0;
    char buf[INTEGER_STR_MAX_LEN+1];
    while(isdigit(str[rpos]) && wpos < INTEGER_STR_MAX_LEN) {
        buf[wpos++] = str[rpos++];
    }
    buf[wpos] = '\0';
    *fraction = strtoull(buf, NULL, 10);
    return SUCCESS;
}

uint32_t qjson_load_exponent(const char *str, int32_t *exponent, const char **parse_end) {
    if(*str != 'e' && *str != 'E') {
        *exponent = 0;
        *parse_end = str;
        return FAILURE;
    }

    uint32_t rpos = 1;
    uint32_t wpos = 0;
    char buf[INTEGER_STR_MAX_LEN+1];

    if(str[rpos] == '-') {
		buf[wpos++] = str[rpos++];
	} else if (str[rpos] == '+') {
		rpos++;
	} else {
	}

    while(isdigit(str[rpos]) && wpos < INTEGER_STR_MAX_LEN) {
        buf[wpos++] = str[rpos++];
    }
    buf[wpos] = '\0';
    *exponent = atoi(buf);
    return SUCCESS;
}

uint32_t qjson_load_number(const char *str, qjson_value_t **value, const char **parse_end) {
	const char *pos = str;
	if(*pos == '-') {
		pos++;
	}
	if(! isdigit(*pos)) {
		*value = NULL;
		*parse_end = str;
		return FAILURE;
	}

	qjson_value_t *number = malloc(sizeof(*number));
    memset(number, 0, sizeof(*number));
    *value = number;

	while(isdigit(*pos)) {
		pos++;
	}
    if(*pos == '.' || *pos == 'e' || *pos == 'E') {
        number->json_type = QJSON_FLOAT;
        number->v.fraction = strtod(str, (char**)parse_end);

	}else{
        number->json_type = QJSON_INT;
		int64_t integer;
		qjson_load_integer(str, &integer, parse_end);
        number->v.integer = integer;
	}
	return SUCCESS;
}


uint32_t qjson_load_object(const char *str, qjson_value_t **value, const char **parse_end) {
    if(*str != '{') {
        *value = NULL;
        *parse_end = str;
        return FAILURE;
    }
    str++;

    const char *pos = str;
    qjson_object_t *obj = qjson_create_object();
    while(*pos != '\0') {

        while(isspace(*pos)) {
            pos++;
        }

        if(*pos == '}'){
            pos++;
            break;
        }

        //int32_t keylen = qjson_strlen(pos);
        char k[BUFLEN];
        qjson_value_t *v = NULL;
        str_unescape(pos, k, BUFLEN, parse_end);
        pos = *parse_end;

        while(isspace(*pos)) {
            pos++;
        }

        if(*pos++ != ':') {
            *value = NULL;
            *parse_end = pos;
            return FAILURE;
        }

        while(isspace(*pos)) {
            pos++;
        }

        qjson_load(pos, &v, parse_end);
        pos = *parse_end;

        qjson_object_append(obj, k, v);

        while(isspace(*pos)) {
            pos++;
        }

        if(*pos == ',') {
            pos++;
        } else if(*pos == '}'){
            pos++;
            break;
        } else {
            //TODO: free
            *value = NULL;
            *parse_end = pos;
            return FAILURE;
        }
    }


    *parse_end = pos;
    *value = malloc(sizeof(*value));
    (*value)->json_type = QJSON_OBJECT;
    (*value)->v.object = obj;

    return SUCCESS;
}


uint32_t qjson_load_array(const char *str, qjson_value_t **value, const char **parse_end) {
    if(*str != '[') {
        *value = NULL;
        *parse_end = str;
        return FAILURE;
    }
    str++;

    const char *pos = str;
    qjson_array_t *array = qjson_create_array();
    while(*pos != ']' && *pos != '\0') {

        while(isspace(*pos)) {
            pos++;
        }

        uint32_t ret;
        qjson_value_t *item = NULL;
        ret = qjson_load(pos, &item, parse_end);

        printf("======= ret = %lu\n", ret);
        qjson_array_append(array, item);

        pos = *parse_end;
        while(isspace(*pos)) {
            pos++;
        }

        if(*pos == ',') {
            pos++;
        } else if(*pos == ']'){
            pos++;
            break;
        } else {
            //TODO: free array
            *value = NULL;
            *parse_end = pos;
            return FAILURE;
        }
    }

    *parse_end = pos;
    *value = malloc(sizeof(*value));
    (*value)->json_type = QJSON_ARRAY;
    (*value)->v.array = array;

    return SUCCESS;
}

uint32_t qjson_load(const char *str, qjson_value_t **value, const char **parse_end) {
    const char *pos = str;
    while(isspace(*pos)) {
        pos++;
    }

    uint32_t ret = SUCCESS;
    if(*pos == '-' || isdigit(*pos)) {
        ret = qjson_load_number(pos, value, parse_end);
    } else if(*pos == '\"') {
        ret = qjson_load_string(pos, value, parse_end);
    } else if(*pos == '[') {
        ret = qjson_load_array(pos, value, parse_end);
    } else if(*pos == '{') {
        ret = qjson_load_object(pos, value, parse_end);
    } else if(*pos == 't' || *pos == 'f'){
        ret = qjson_load_bool(pos, value, parse_end);
    } else if(*pos == 'n') {
        ret = qjson_load_null(pos, value, parse_end);
    } else {
        *value = NULL;
        *parse_end = pos;
        return FAILURE;
    }

    return ret;
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


qjson_object_t *qjson_create_object() {
    qjson_object_t *self = malloc(sizeof(*self));
    memset(self, 0, sizeof(sizeof(*self)));
    return self;
}


qjson_object_t *qjson_object_append(qjson_object_t *obj, const char *key, const qjson_value_t *e) {
    qjson_pair_t *pair = malloc(sizeof(qjson_pair_t));
    pair->key = strdup(key);
    pair->value = *e; //TODO: deep copy
    pair->next = NULL;

    qjson_pair_t *last = &obj->head;
    while(last->next != NULL){
        last = last->next;
    }
    last->next = pair;

    return obj;
}

void test_dump_str_array() {
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

    char buf[BUFLEN];
    int bytes = qjson_dump_array(arr, buf, BUFLEN);
    printf("buf: %s, bytes: %d, strlen: %lu\n", buf, bytes, strlen(buf));
}

#define elemsof(arr) (sizeof(arr)/sizeof(*arr))

void test_dump_object() {
    printf("\n\nin [%s]\n", __FUNCTION__);

    struct {
        const char *key;
        const char *value;
    } datas1[] = {
        {"name", "zhangsan"},
        {"city", "beijing"},
        {"birth", "2002"},
        {"dog", "xiaoming"}
    };

    struct {
        const char *key;
        int value;
    } datas2[] = {
        {"id", 123123},
        {"age", 18},
        {"number", -123},
    };

    qjson_object_t *object = qjson_create_object();
    for(int i=0; i<elemsof(datas1); i++) {
        qjson_object_append(object, datas1[i].key, qjson_create_str(datas1[i].value));
    }

    for(int i=0; i<elemsof(datas2); i++) {
        qjson_object_append(object, datas2[i].key, qjson_create_int(datas2[i].value));
    }


    char buf[BUFLEN];
    int bytes = qjson_dump_object(object, buf, BUFLEN);
    printf("buf: %s, bytes: %d, strlen: %lu\n", buf, bytes, strlen(buf));
}


void test_load_object() {
    printf("\n\nin [%s]\n", __FUNCTION__);

    const char *str = "{\"name\": \"zhangsan\", \"age\": 18, \"args1\": {\"tt\": {}}, \"args2\": {}}END";
    const char *end;
    qjson_value_t *object;

    qjson_load_object(str, &object, &end);

    printf("qjson_load_object end: [%s]\n", end);


    char buf[BUFSIZ];
    qjson_dump_object(object->v.object, buf, BUFLEN);
    printf("dump object: %s\n", buf);

}

void test_dump_number_array() {
    printf("\n\nin [%s]\n", __FUNCTION__);

    const char *numberlist[] = {
        "0",
        "-1",
        "1",
        "12345678",
        "1.1",
        "0.0",
        "-1.1",
        "-0.0",
        "1234567890.12345678901234567890",
        "1.23456789E+2",
        NULL,
    };
    const char **cur = &numberlist[0];

    qjson_array_t *arr = qjson_create_array();

    while(*cur != NULL) {
        qjson_value_t *number;
        char *end;
        qjson_load_number(*cur++, &number, (const char**)&end);
        qjson_array_append(arr, number);
    }

    qjson_value_t *temp;
    temp = qjson_create_bool(true);
    qjson_array_append(arr, temp);

    temp = qjson_create_bool(false);
    qjson_array_append(arr, temp);

    temp = qjson_create_null();
    qjson_array_append(arr, temp);

    char buf[BUFLEN];
    int bytes = qjson_dump_array(arr, buf, BUFLEN);
    printf("buf: %s, bytes: %d, strlen: %lu\n", buf, bytes, strlen(buf));
}

void test_load_array() {
    printf("\n\nin [%s]\n", __FUNCTION__);

    const char *str = "[-0.0,1,2.0,3, 4,   5.678  , \"9A\", \"10B.\" , null, false, true, [], [9,  8, 7, [0]]]] ]";
    const char *end;
    qjson_value_t *array;

    qjson_load_array(str, &array, &end);

    char buf[BUFSIZ];
    qjson_dump_array(array->v.array, buf, BUFLEN);
    printf("dump array: %s\n", buf);
}

void test_load_string() {
    printf("in [%s]\n", __FUNCTION__);

    qjson_value_t *json_value;
    const char *parse_end = NULL;
    const char *str = "  \t\"nihao\t\\hello\\\\world\"string end";
    uint32_t ret = qjson_load_string(str, &json_value, &parse_end);

    printf("ret: [%d]\n", ret);

    char buf[BUFLEN];
    qjson_dump(json_value, buf, BUFLEN);
    printf("json_value: [%s]\n", buf);
    printf("parse_end: [%s]\n", parse_end);
}

void test_unescape() {
    printf("in [%s]\n", __FUNCTION__);

    const char *str = "\"nihao\\t\\\\hello\\\"E\"ND";
    char buf[BUFLEN];
    const char *parse_end;
    uint32_t ret = str_unescape(str, buf, BUFLEN, &parse_end);
    printf("ret = %d \n", ret);
    printf("buf = %s \n", buf);
    printf("parse_end = %s\n", parse_end);
}

void test_load_integer() {
    printf("in [%s]\n", __FUNCTION__);

    const char *str = "-12345678901234567890123456789";
    const char *end;
    int64_t integer;
    qjson_load_integer(str, &integer, &end);
    printf("json_load_integer = %lld\n", integer);

    int nbytes = sscanf(str, "%lld", &integer);
    printf("sscanf integer = %lld, nbytes = %d\n", integer, nbytes);

    integer = strtoll(str, NULL, 10);
    printf("strtoll integer = %lld\n", integer);

    printf("parse_end = %s\n", end);
}


void test_load_fraction() {
    printf("in [%s]\n", __FUNCTION__);

    const char *str = ".12345678901234567890";
    const char *end;
    uint64_t integer = 0;
    qjson_load_fraction(str, &integer, &end);
    printf("json_load_fraction integer = %llu\n", integer);
    int nbytes = sscanf(str, "%llu", &integer);
    printf("sscanf integer = %llu, nbytes = %d\n", integer, nbytes);
    printf("parse_end = %s\n", end);
}

void test_lld() {
	printf("sizeof(uint64_t) = %d, sizeof(long long int) = %d, sizeof(long int) = %d\n", sizeof(uint64_t), sizeof(long long int), sizeof(long int));
}

int main() {
    //test_dump_str_array();
    test_dump_number_array();
    //test_load_array();
    //test_load_integer();
    //test_load_fraction();
    //test_load_string();
    //test_unescape();
    //test_lld();

    test_dump_object();

    test_load_object();
    return 0;
}
