#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "str_link_list.h"
#include "e_common.h"

// assume str is valid
// warning is that len is the length without the \0
static token_t *init_token(char *str, size_t len) {
    token_t *new_token = (token_t *)malloc(sizeof(token_t));
    SYSCALL_ERR_EXIT("malloc sizeof token_t", new_token, NULL);
    // + 1 to include the NULL
    new_token->token = (char *)malloc(sizeof(char) * (len + 1)); 
    strcpy(new_token->token, str);
                                                                 
    new_token->len = len;

    return new_token;
}

static sll_node_t *init_node(token_t *token, sll_node_t *next) {
    sll_node_t *new_node = (sll_node_t *)malloc(sizeof(sll_node_t));
    SYSCALL_ERR_EXIT("malloc sizeof sll_node_t", new_node, NULL);
    new_node->token = token;
    new_node->next = next;
    return new_node;
}

static void destroy_token(token_t **t) {
    if(t == NULL || *t == NULL) {
        return;
    }
    free((*t)->token);
    free(*t);
    *t = NULL;
}

static void destroy_node(sll_node_t **node) {
    if(node == NULL || *node == NULL) {
        return;
    }

    destroy_token(&(*node)->token);
    free(*node);
    *node = NULL;
}


sll_t *init_sll() {
    sll_t *new_sll = (sll_t *)malloc(sizeof(sll_t));
    SYSCALL_ERR_EXIT("malloc sizeof sll_t", new_sll, NULL);

    new_sll->head = NULL;
    new_sll->tail = NULL;
    new_sll->len = 0;
    return new_sll;
}


void insert_into_sll(sll_t *sll, char *token, size_t len) {
    if(sll == NULL) {
        return;
    }
    token_t *new_token = init_token(token, len);
    sll_node_t *new_node = init_node(new_token, NULL);
    if(sll->head == NULL) {
        sll->head = new_node;
        sll->tail = new_node;
    } else {
        sll->tail->next = new_node;
        sll->tail = new_node;
    }

    sll->len++;
}

char **sll_to_stra(sll_t *sll) {
    if(sll == NULL || sll->len == 0) {
        return NULL;
    }
    char **stra = (char **)(malloc(sizeof(char *) * (sll->len + 1)));
    SYSCALL_ERR_EXIT("malloc sizeof char * sll->len + 1", stra, NULL);

    sll_node_t * cur = sll->head;

    for(size_t i = 0; i < sll->len; ++i, cur = cur->next) {
        stra[i] = (char *)malloc(sizeof(char) * (cur->token->len + 1)); 
        SYSCALL_ERR_EXIT("malloc sizeof char * cur->token->len + 1", stra, NULL);
        strcpy(stra[i], cur->token->token);
    }
    stra[sll->len] = NULL;

    return stra;
}

void destroy_sll_stra(char **stra) {
    if(stra == NULL) {
        return;
    }
    size_t i = 0;
    while(stra[i] != NULL) { 
        free(stra[i]);
        ++i;
    }
    free(stra);
}

void destroy_sll(sll_t **sll) {
    //debug("hi, %p\n", *sll);
    if(sll == NULL || *sll == NULL) {
        return;
    }

    sll_node_t * cur = (*sll)->head;
    sll_node_t * next = cur;

    while(cur != NULL) {
        next = next->next;
        destroy_node(&cur);
        cur = next;
    }

    free(*sll);
    *sll = NULL;
}

void print_stra(char **stra) {
    if(stra == NULL || *stra == NULL) {
        return;
    }
    printf("[");
    int i = 0, j = 1;
    for(; stra[j] != NULL; ++i, ++j) {
        printf("%s ", stra[i]);
    }
    printf("%s", stra[i]);
    printf("]");
}

void print_lst(sll_t *sll) {
    if(sll == NULL || sll->len == 0) {
        return;
    }
    sll_node_t * cur = sll->head;
    printf("[");
    for(size_t i = 0; i < sll->len - 1; ++i, cur = cur->next) {
        printf("%s ", cur->token->token);
    }
    printf("%s", cur->token->token);
    printf("]");
}
