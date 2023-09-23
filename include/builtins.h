#ifndef BUILTIN_H
#define BUILTIN_H
#include "watcher_table.h"

//"start", "quit", "watchers", "stop", "trace", "untrace", "show", "ANY"
typedef enum BUILTIN_ORDINAL {
    BUILTIN_START_TYPE,
    BUILTIN_QUIT_TYPE,
    BUILTIN_WATCHERS_TYPE,
    BUILTIN_STOP_TYPE,
    BUILTIN_TRACE_TYPE,
    BUILTIN_UNTRACE_TYPE,
    BUILTIN_SHOW_TYPE
} builtin_ordinal;

extern WATCHER_TYPE *search_watcher_by_name(char *name);
extern int run_builtin(int com, watcher_table *wt, char *txt);

#endif
