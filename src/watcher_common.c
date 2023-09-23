#include <time.h>
#include <stdio.h>

#include "watcher_table.h"
#include "watcher_common.h"
#include "debug.h"

static void print_trace_time(WATCHER *w, char *msg) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long ms = ts.tv_nsec / 1000;
    fprintf(stderr, "[%lu.%06lu][%s][%d][%lu]: %s", 
            ts.tv_sec, ms, w->w_type->name, w->proc_conn_info.read_fd, w->serial_num, msg);
}

void trace_print(WATCHER *w, char *msg) {
    if(w == NULL) {
        return;
    }
    w->serial_num++;
    if(w->trace) {
       print_trace_time(w, msg); 
    }
}
