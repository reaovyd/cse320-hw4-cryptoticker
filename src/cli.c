#include <stdlib.h>
#include <signal.h>

#include "ticker.h"
#include "store.h"
#include "cli.h"
#include "debug.h"
#include "watcher_table.h"
#include "e_common.h"
#include "parser.h"

static int cli_started = 0;

// returns NULL on error, non-null as a watcher object and no error
WATCHER *cli_watcher_start(WATCHER_TYPE *type, char *args[]) {
    if(cli_started) {
        return NULL;
    }
    cli_started = 1;
    WATCHER *ret = malloc(sizeof(WATCHER));
    SYSCALL_ERR_EXIT("malloc", ret, NULL);

    ret->proc_conn_info.name     = type->name;
    ret->proc_conn_info.read_fd  = STDIN_FILENO; 
    ret->proc_conn_info.write_fd = STDOUT_FILENO;
    ret->proc_conn_info.proc_id  = -1; 
    ret->trace = false;

    ret->argv = type->argv;
    ret->args = args;
    return ret;
}

int cli_watcher_stop(WATCHER *wp) {
    return -1;
}

int cli_watcher_send(WATCHER *wp, void *arg) {
    if(arg == NULL)
        return -1;
    fprintf(stdout, "%s", (char *)arg);
    fflush(stdout);
    return 0;
}

int cli_watcher_recv(WATCHER *wp, char *txt) {
    // this method will be for parsing txt
    // take the txt line
    // and then split spaces
    // process first line
    //"start", "quit", "watchers", "stop", "trace", "untrace", "show", "ANY"
    return is_valid_input(txt);
}

int cli_watcher_trace(WATCHER *wp, int enable) {
    if(enable == 0) {
        wp->trace = 0;
    } else {
        wp->trace = 1;
    }
    return 0;
}
