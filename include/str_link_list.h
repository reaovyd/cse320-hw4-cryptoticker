#ifndef STR_LINK_LIST_H
#define STR_LINK_LIST_H

typedef struct token_t {
    char *token;
    size_t len;
} token_t;

typedef struct sll_node_t {
    struct sll_node_t *next;
    token_t *token;
} sll_node_t;

typedef struct sll_t {
    struct sll_node_t* head;
    struct sll_node_t* tail;
    size_t len;
} sll_t;

extern sll_t *init_sll();
extern void destroy_sll(sll_t **sll);
extern void insert_into_sll(sll_t *sll, char *token, size_t len);
extern void print_lst(sll_t *sll);

extern void destroy_sll_stra(char **stra);
extern char **sll_to_stra(sll_t *sll);
extern void print_stra(char **stra);

#endif 
