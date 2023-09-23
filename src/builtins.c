#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "ticker.h"
#include "debug.h"
#include "builtins.h"
#include "watcher_table.h"
#include "store.h"

static void safe_print_table(watcher_table *table_print) {
    sigset_t sig_mask, prev_mask;
    sigfillset(&sig_mask);
    sigprocmask(SIG_BLOCK, &sig_mask, &prev_mask);
    print_watcher_table(table_print);
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
}

static bool is_zero_watcher(WATCHER_TYPE type) {
     return 
         type.name  == NULL &&
         type.argv  == NULL &&
         type.start == NULL &&
         type.stop  == NULL &&
         type.send  == NULL &&
         type.recv  == NULL &&
         type.trace == NULL
         ;
}

//"start", "quit", "watchers", "stop", "trace", "untrace", "show", "ANY"
static const char *delim = " \t\n";
static int builtin_start(watcher_table *wt, char *txt) {
    char *reentrant;
    strtok_r(txt, delim, &reentrant); // just skip start
    txt = NULL;
    char *watcher_name = strtok_r(txt, delim, &reentrant);
    if(strcmp(watcher_name, "CLI") == 0) {
        return -1;
    }
    WATCHER_TYPE *new_wt = search_watcher_by_name(watcher_name);
    if(new_wt == NULL) {
        return -1;
    }

    sll_t *args = init_sll();
    while(1) {
        char *token = strtok_r(txt, delim, &reentrant);
        if(token == NULL)
            break;
        insert_into_sll(args, token, strlen(token));
    }
    char **stra = sll_to_stra(args);
    destroy_sll(&args);

    sigset_t chld_mask, chld_prev_mask;

    sigemptyset(&chld_mask);
    sigaddset(&chld_mask, SIGCHLD);

    // block sigchld incase it may terminate for whwatever resason
    sigprocmask(SIG_BLOCK, &chld_mask, &chld_prev_mask);
    WATCHER *new_watcher = new_wt->start(new_wt, stra);

    if(new_watcher == NULL) {
        destroy_sll_stra(stra);
        return -1;
    }

    sigset_t all_mask, all_prev_mask;
    sigfillset(&all_mask);
    sigprocmask(SIG_BLOCK, &all_mask, &all_prev_mask);
    add_new_entry(wt, 
            new_watcher->proc_conn_info.name,
            new_watcher->proc_conn_info.proc_id,
            new_watcher->proc_conn_info.read_fd,
            new_watcher->proc_conn_info.write_fd,
            new_watcher->w_type,
            new_watcher->trace,
            new_watcher->argv,
            new_watcher->args);
    sigprocmask(SIG_BLOCK, &all_prev_mask, NULL);
    // unblock sigchld
    sigprocmask(SIG_SETMASK, &chld_prev_mask, NULL);

    free(new_watcher);

    //if(stra != NULL) {
    //    int i = 0;
    //    while(stra[i] != NULL) {
    //        new_wt->send(added_watcher, stra[i]);
    //        ++i;
    //    }
    //}

    return 0;
}


volatile sig_atomic_t EXISTS_CHILD_QUIT_FLAG = 0;
volatile sig_atomic_t CHILD_QUIT_LENGTH = 0;

static int builtin_quit(watcher_table *wt, char *txt) {
    // i = 1 because only care for children
    sigset_t sigio_block_forever;
    sigemptyset(&sigio_block_forever);
    sigaddset(&sigio_block_forever, SIGIO);
    sigprocmask(SIG_BLOCK, &sigio_block_forever, NULL);

    sigset_t block, prev_block;
    sigemptyset(&block);
    sigemptyset(&prev_block);
    sigaddset(&block, SIGIO);
    sigaddset(&block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block, &prev_block);
    volatile sig_atomic_t cap_length = wt->len - 1;
    for(size_t i = 1; i < wt->capacity; ++i) {
        watcher *entry = &wt->table_arr[i];
        if(!is_empty_watcher_entry(entry)) {
            EXISTS_CHILD_QUIT_FLAG = 1;
        }
    }
    sigprocmask(SIG_SETMASK, &prev_block, NULL);
    if(EXISTS_CHILD_QUIT_FLAG == 1) {
        // TODO uhh probably need a better solution for this but this
        // is all i got rn bruh
        while(CHILD_QUIT_LENGTH < cap_length) {
            for(size_t i = 1; i < wt->capacity; ++i) {
                sigprocmask(SIG_BLOCK, &block, &prev_block);
                watcher *entry = &wt->table_arr[i];
                if(!is_empty_watcher_entry(entry)) {
                    entry->w_type->stop(entry);
                }
                sigprocmask(SIG_SETMASK, &prev_block, NULL);
            }
        }
    }
    sigemptyset(&block);
    sigemptyset(&prev_block);
    sigaddset(&block, SIGIO);
    sigaddset(&block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block, NULL);
    destroy_table(&wt);
    sigprocmask(SIG_SETMASK, &prev_block, NULL);
    _exit(EXIT_SUCCESS);
    return 0;
}

static bool contains_all_digits(char *txt) {
    if(txt == NULL) {
        return false;
    }
    int i = 0;

    while(txt[i] != '\n' && txt[i] != '\0') {
        if(!isdigit(txt[i])) {
            return false;
        }
        i++;
    }
    return true;
}

static watcher *single_arg_case(watcher_table *wt, char *txt, bool istrace) {
    char *reentrant;
    strtok_r(txt, delim, &reentrant); // just skip start
    txt = NULL;

    char *arg = strtok_r(txt, delim, &reentrant);
    if(arg == NULL) { // athough this case may never happen
        return NULL;
    }
    if(!contains_all_digits(arg)) {
        return NULL;
    }

    // TODO potential integer overflows
    int idx = atoi(arg);
    if(!istrace && idx == 0) {
        return NULL;
    }
    watcher *w = get_watcher_entry_by_idx(wt, idx);
    if(w == NULL) {
        return NULL;
    }
    return w;
}

static int builtin_watchers(watcher_table *wt, char *txt) {
    safe_print_table(wt);
    return 0;
}

static int builtin_stop(watcher_table *wt, char *txt) {
    watcher *w = single_arg_case(wt, txt, false);
    if(w == NULL) {
        return -1;
    }
    return w->w_type->stop(w);
}

static int builtin_trace(watcher_table *wt, char *txt) {
    watcher *w = single_arg_case(wt, txt, true);
    if(w == NULL) {
        return -1;
    }
    sigset_t mask, prev_mask;
    sigemptyset(&mask);

    sigaddset(&mask, SIGIO);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask); 
    int res = w->w_type->trace(w, 1);
    sigprocmask(SIG_SETMASK, &prev_mask, NULL); 
    return res;
}

static int builtin_untrace(watcher_table *wt, char *txt) {
    watcher *w = single_arg_case(wt, txt, true);
    if(w == NULL) {
        return -1;
    }
    sigset_t mask, prev_mask;
    sigemptyset(&mask);

    sigaddset(&mask, SIGIO);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask); 
    int res = w->w_type->trace(w, 0);
    sigprocmask(SIG_SETMASK, &prev_mask, NULL); 
    return res;
}

static int builtin_show(watcher_table *wt, char *txt) {
    char *reentrant;
    strtok_r(txt, delim, &reentrant); // just skip start
    txt = NULL;

    char *arg = strtok_r(txt, delim, &reentrant);
    if(arg == NULL) { // athough this case may never happen
        return -1;
    }
    struct store_value *sv = store_get(arg);
    if(sv == NULL) {
        return -1;
    }
    fprintf(stdout, "%s\t%f\n", arg, sv->content.double_value);
    store_free_value(sv);
    fflush(stdout);
    return 0;
}

static int (*builtin_cmd[]) (watcher_table *wt, char *txt) = {
    builtin_start,
    builtin_quit,
    builtin_watchers,
    builtin_stop,
    builtin_trace,
    builtin_untrace,
    builtin_show
};

WATCHER_TYPE *search_watcher_by_name(char *name) {
    WATCHER_TYPE *wtp = watcher_types;
    while(!is_zero_watcher(*wtp)) {
        if(strcmp(wtp->name, name) == 0) {
            return wtp;
        }
        wtp++;
    }
    return NULL;
}

int run_builtin(int com, watcher_table *wt, char *txt) {
    if(com == -1) {
        return -1;
    }
    return builtin_cmd[com](wt, txt);
}

