#include <stdlib.h>

#include "deque_reader.h"
#include "debug.h"
#include "e_common.h"  

#define DEFAULT_DEQUE_SIZE 4096
#define DEQUE_FACTOR 4 

static void dr_extend_arr(deque_reader *dr) {
    size_t new_cap = dr->cap * DEQUE_FACTOR; 
    dr->buf = realloc(dr->buf, sizeof(char) * new_cap);

    SYSCALL_ERR_EXIT("realloc dr->buf sizeof char new_cap", dr->buf, NULL);
    dr->cap = new_cap;

    for(int i = dr->len; i < dr->cap; ++i) {
        dr->buf[i] = 0;
    }
}


deque_reader *dr_init() {
    deque_reader *ret = malloc(sizeof(deque_reader)); 
    SYSCALL_ERR_EXIT("malloc sizeof deque_reader", ret, NULL);

    ret->buf = calloc(DEFAULT_DEQUE_SIZE, sizeof(char));  
    SYSCALL_ERR_EXIT("malloc sizeof char DEFAULT_DEQUE_SIZE", ret->buf, NULL);

    ret->len = 0; 
    ret->cap = DEFAULT_DEQUE_SIZE;
    ret->input_ready = 0;

    return ret;
}

void dr_write_char(deque_reader *dr, char c) {
    if(dr == NULL) {
        return;
    }
    if(dr->len >= dr->cap) {
        dr_extend_arr(dr);
    }
    if(c == '\n')
        dr->input_ready = 1;
    dr->buf[dr->len] = c;
    dr->len++;
}

char *dr_get_avail_input(deque_reader *dr) {
    if(dr == NULL) {
        return NULL;
    }
    if(!dr->input_ready) {
        return NULL;
    }
    char *ret = calloc(dr->len + 1, sizeof(char)); 
    SYSCALL_ERR_EXIT("calloc sizeof char DEFAULT_DEQUE_SIZE", ret, NULL);
    for(int i = 0; i < dr->len; ++i) {
        ret[i] = dr->buf[i];
        dr->buf[i] = 0;
    }

    dr->input_ready = 0;
    dr->len = 0;

    return ret;
}

void dr_destroy(deque_reader **dr) {
    if(dr == NULL || *dr == NULL) {
        return;
    }
    free((*dr)->buf);
    free(*dr);
    *dr = NULL;
}
