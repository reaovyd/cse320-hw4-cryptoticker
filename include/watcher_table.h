#ifndef WATCHER_TABLE_H 
#define WATCHER_TABLE_H

#include <sys/types.h>
#include <stdbool.h>

#include "ticker.h"
#include "str_link_list.h"
#include "deque_reader.h"

typedef struct watcher {
    int idx;
    struct {
        char *name;
        pid_t proc_id;
        int read_fd; /* watcher process reads from this fd as input*/
        int write_fd; /* watcher process writes to this fd and read from TICKER SHELL*/
    } proc_conn_info;
    WATCHER_TYPE *w_type;
    bool trace;
    char **argv;
    char **args;
    size_t serial_num;
    deque_reader *dr;
} watcher;

typedef struct watcher_table {
    watcher *table_arr;
    volatile size_t len;
    size_t capacity;
} watcher_table;


extern watcher_table* table_init();

extern watcher *add_new_entry(watcher_table *table, char *new_name, 
        pid_t new_pid, int read_fd, int write_fd, WATCHER_TYPE *w_type, bool trace, char **argv, char** args);
extern int remove_entry(watcher_table *table, size_t idx);
extern int remove_entry_by_pid(watcher_table *table, pid_t pid);
extern void print_watcher_table(watcher_table *table);

extern void destroy_table(watcher_table **table);
extern bool is_empty_watcher_entry(watcher *entry);
extern watcher *get_watcher_entry_by_idx(watcher_table *table, int idx);



#endif /* WATCHER_TABLE_H */
