#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "watcher_table.h"
#include "e_common.h"
#include "debug.h"

#define DEFAULT_TABLE_SIZE 256 
#define EXTEND_FACTOR      2

static void extend_table(watcher_table *table) {

    size_t new_capacity = table->capacity * EXTEND_FACTOR;

    table->table_arr = realloc(table->table_arr, 
            sizeof(watcher) * new_capacity);

    SYSCALL_ERR_EXIT("realloc", table->table_arr, NULL);

    for(size_t i = table->len; i < new_capacity; ++i) {
        table->table_arr[i].idx = 0;
        table->table_arr[i].proc_conn_info.name = 0;
        table->table_arr[i].proc_conn_info.proc_id = 0;
        table->table_arr[i].proc_conn_info.read_fd = 0;
        table->table_arr[i].proc_conn_info.write_fd = 0;
        table->table_arr[i].w_type = NULL;
        table->table_arr[i].trace = false;
        table->table_arr[i].argv = 0;
        table->table_arr[i].args = NULL;
        table->table_arr[i].serial_num = 0;
        table->table_arr[i].dr = NULL;
    }

    table->capacity = new_capacity;

}

static bool is_empty_entry(watcher *entry) {
    return entry->idx == 0 &&
        entry->proc_conn_info.name == 0 &&
        entry->proc_conn_info.proc_id == 0 &&
        entry->proc_conn_info.read_fd == 0 && 
        entry->proc_conn_info.write_fd == 0 &&  
        entry->w_type == NULL &&
        entry->trace == false &&
        entry->argv == NULL &&
        entry->args == NULL &&
        entry->serial_num == 0 &&
        entry->dr == NULL;
}

bool is_empty_watcher_entry(watcher *entry) {
    if(entry == NULL)
        return true;
    return is_empty_entry(entry);
}

watcher_table* table_init() {
    watcher_table *table = (watcher_table *)malloc(sizeof(watcher_table));

    SYSCALL_ERR_EXIT("malloc", table, NULL);

    table->table_arr = (watcher *)calloc(DEFAULT_TABLE_SIZE, sizeof(watcher));

    SYSCALL_ERR_EXIT("calloc", table->table_arr, NULL);

    table->capacity = DEFAULT_TABLE_SIZE;
    table->len      = 0;

    return table;
}

watcher *add_new_entry(watcher_table *table, char *new_name, 
        pid_t new_pid, int read_fd, int write_fd, WATCHER_TYPE *w_type, bool trace, char **argv, char **args) {
    if(table->len == table->capacity) {
        extend_table(table);
    }

    watcher *ret = NULL;


    for(size_t i = 0; i < table->capacity; ++i) {
        if(is_empty_entry(&table->table_arr[i])) {
            table->table_arr[i].idx = i;
            table->table_arr[i].proc_conn_info.name = new_name;
            table->table_arr[i].proc_conn_info.proc_id = new_pid;
            table->table_arr[i].proc_conn_info.read_fd = read_fd;
            table->table_arr[i].proc_conn_info.write_fd = write_fd;
            table->table_arr[i].w_type = w_type;
            table->table_arr[i].trace = false;
            table->table_arr[i].argv = argv;
            table->table_arr[i].args = args;
            table->table_arr[i].serial_num = 0;
            table->table_arr[i].dr = dr_init();

            ret = &table->table_arr[i];
            break;
        }
    }

    ++table->len;
    return ret;
}

int remove_entry(watcher_table *table, size_t idx) {
    if(idx >= table->capacity || idx < 0 || is_empty_entry(&table->table_arr[idx])) {
        return -1;
    }

    close(table->table_arr[idx].proc_conn_info.read_fd);
    close(table->table_arr[idx].proc_conn_info.write_fd);

    table->table_arr[idx].idx = 0;
    table->table_arr[idx].proc_conn_info.name = 0;
    table->table_arr[idx].proc_conn_info.proc_id = 0;
    table->table_arr[idx].proc_conn_info.read_fd = 0;
    table->table_arr[idx].proc_conn_info.write_fd = 0;

    table->table_arr[idx].w_type = NULL;
    table->table_arr[idx].trace = false;

    table->table_arr[idx].argv = 0;

    destroy_sll_stra(table->table_arr[idx].args);
    table->table_arr[idx].args = NULL;

    dr_destroy(&table->table_arr[idx].dr);
    table->table_arr[idx].serial_num = 0;

    --table->len;

    return 0;
}

watcher *get_watcher_entry_by_idx(watcher_table *table, int idx) {
    if(idx >= table->capacity || idx < 0 || is_empty_entry(&table->table_arr[idx])) {
        return NULL;
    }
    return &table->table_arr[idx];
}

int remove_entry_by_pid(watcher_table *table, pid_t pid) {
    if(table == NULL) {
        return -1;
    }
    for(size_t i = 0; i < table->capacity; ++i) {
        if(!is_empty_entry(&table->table_arr[i]) && 
                table->table_arr[i].proc_conn_info.proc_id == pid) {
            return remove_entry(table, i);
        }
    }
    return -1;
}

void print_watcher_table(watcher_table *table) {
    for(size_t i = 0; i < table->capacity; ++i) {
        watcher cur_entry = table->table_arr[i];
        if(!is_empty_entry(&cur_entry)) {
            fprintf(stdout, "%d\t%s(%d,%d,%d)", 
                    cur_entry.idx, 
                    cur_entry.proc_conn_info.name,
                    cur_entry.proc_conn_info.proc_id,
                    cur_entry.proc_conn_info.read_fd,
                    cur_entry.proc_conn_info.write_fd
                    );
            if(cur_entry.argv != NULL) {
                char **ptr = cur_entry.argv;
                while(ptr != NULL && *ptr != NULL) {
                    fprintf(stdout, " %s", *ptr);
                    ptr++;
                }
            }
            if(cur_entry.args != NULL && *cur_entry.args != NULL) {
                printf(" ");
                print_stra(cur_entry.args);
            }
            printf("\n");
            fflush(stdout);
        }
    }
}

void destroy_table(watcher_table **table) {
    if(table == NULL || *table == NULL) {
        return;
    }

    for(size_t i = 0; i < (*table)->capacity; ++i) {
        if(!is_empty_entry(&((*table)->table_arr[i]))) 
            remove_entry(*table, i);
    }

    free((*table)->table_arr);
    free(*table);
    *table = NULL;
}
