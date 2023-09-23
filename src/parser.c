#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "parser.h"
#include "e_common.h"

static const char *delim = " \n\t\b";

typedef struct parse_pair{
    unsigned short n;
    char *string_conn;
} parse_pair;

static int is_final_state(unsigned short k) {
    return k == 2 || k == 3 || k == 5;
}

static int holds_any(unsigned short k) {
    return k != 0 && k != 6;
}

static char *cmd_lst[] = {
    "start", "quit", "watchers", "stop", "trace", "untrace", "show", "ANY"
};

static int get_cmd_num(char *str) {
    for(int i = 0; i < 7; ++i) {
        if(strcmp(str, cmd_lst[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static parse_pair adj_lst[8][10] = {
    {
        {.n = 1,
        .string_conn = "start"},
        {.n = 3,
        .string_conn = "quit"},
        {.n = 3,
        .string_conn = "watchers"},
        {.n = 4,
        .string_conn = "stop"},
        {.n = 4,
        .string_conn = "trace"},
        {.n = 4,
        .string_conn = "untrace"},
        {.n = 4,
        .string_conn = "show"},
        {0}
    },
    {
        {.n = 2,
        .string_conn = "ANY"},
        {0}
    },
    {
        {.n = 2,
        .string_conn = "ANY"},
        {0}
    },
    {
        {.n = 6,
        .string_conn = "ANY"},
        {0}
    },
    {
        {.n = 5,
        .string_conn = "ANY"},
        {0}
    },
    {
        {.n = 6,
        .string_conn = "ANY"
        },
        {0}
    },
    {
        {.n = 6,
        .string_conn = "ANY"
        },
        {0}
    }
};

int is_valid_input(char *txt) {
    if(txt == NULL)
        return -1;

    unsigned short cur_state = 0;

    char *str_ptr = txt;
    char *save_ptr;
    char *token = NULL;
    char *first_token = NULL; 
    int get_first_token = 0;

    while(1) {
        token = strtok_r(str_ptr, delim, &save_ptr);
        if(token == NULL)
            break;
        if(!get_first_token) {
            first_token = token;
            get_first_token = 1;
        }
        parse_pair *to_swap = NULL;
        parse_pair *cur_state_lst = adj_lst[cur_state];

        int gcn_res_token = -1;
        if(!holds_any(cur_state)) {
            gcn_res_token = get_cmd_num(token);
        }

        while(cur_state_lst->n != 0 && cur_state_lst->string_conn != NULL) {
            int gcn_res_edge = get_cmd_num(cur_state_lst->string_conn);
            if(gcn_res_edge == gcn_res_token) {
                to_swap = cur_state_lst;
                break;
            }
            cur_state_lst++;
        }
        if(to_swap == NULL) {
            return -1;
        } else {
            cur_state = to_swap->n;
        }
        str_ptr = NULL;
    }
    if(is_final_state(cur_state)) {
        return get_cmd_num(first_token);
    }
    return -1;
}

FILE* is_valid_server_msg(char *txt) {
    if(txt == NULL)
        return NULL;
    char *reentrant;
    char *token = strtok_r(txt, delim, &reentrant);
    if(token == NULL || strcmp(token, ">") != 0) {
        return NULL;
    }
    token = strtok_r(NULL, delim, &reentrant);
    if(token == NULL || strcmp(token, "Server") != 0) {
        return NULL;
    }
    token = strtok_r(NULL, delim, &reentrant);
    if(token == NULL || strcmp(token, "message:") != 0) {
        return NULL;
    }
    token = strtok_r(NULL, delim, &reentrant);
    if(token == NULL) {
        return NULL;
    }
    size_t slen = strlen(token);
    if(slen < 3) {
        return NULL;
    }

    if(token[0] != '\'' || token[slen - 1] != '\'') {
        return NULL;
    }
    FILE *stream = fmemopen(NULL, slen, "r+");
    SYSCALL_ERR_EXIT("fmemopen NULL slen r+", stream, NULL);
    fwrite(token + 1, sizeof(char), slen - 2, stream);
    fflush(stream);
    rewind(stream);
    return stream;
}
