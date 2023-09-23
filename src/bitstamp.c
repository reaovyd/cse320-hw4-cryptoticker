#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "watcher_table.h"
#include "ticker.h"
#include "bitstamp.h"
#include "debug.h"
#include "e_common.h"
#include "argo.h"
#include "store.h"
#include "parser.h"

WATCHER *bitstamp_watcher_start(WATCHER_TYPE *type, char *args[]) {
    if(args == NULL || args[0] == NULL || type == NULL) {
        return NULL;
    }
    int fd1[2], fd2[2];
    SYSCALL_ERR_EXIT("pipe fd1", pipe(fd1), -1);
    SYSCALL_ERR_EXIT("pipe fd2", pipe(fd2), -1);

    set_async_nonblock(fd1[0], getpid());

    pid_t c_pid = fork();
    SYSCALL_ERR_EXIT("fork", c_pid, -1);
    if(c_pid == 0) {
        sigset_t unblock_all;
        sigemptyset(&unblock_all);
        sigprocmask(SIG_SETMASK, &unblock_all, NULL);
        SYSCALL_ERR_EXIT("close fd1[0]", close(fd1[0]), -1); 
        SYSCALL_ERR_EXIT("close fd2[1]", close(fd2[1]), -1); 

        SYSCALL_ERR_EXIT("dup2 fd1[1] STDOUT_FILENO", dup2(fd1[1], STDOUT_FILENO), -1); 
        SYSCALL_ERR_EXIT("dup2 fd2[0] STDOUT_FILENO", dup2(fd2[0], STDIN_FILENO), -1); 

        SYSCALL_ERR_EXIT("close fd1[1]", close(fd1[1]), -1); 
        SYSCALL_ERR_EXIT("close fd2[0]", close(fd2[0]), -1); 
        set_async_nonblock(STDIN_FILENO, c_pid);

        int exec_status = execvp(UWSC_PATHNAME, type->argv);
        SYSCALL_ERR_EXIT("execvp UWSC_PATHNAME type->argv", exec_status, -1);

        return NULL; /* can guarantee this doesn't run cuz of execvp*/
    }

    SYSCALL_ERR_EXIT("close fd1[1]", close(fd1[1]), -1); 
    SYSCALL_ERR_EXIT("close fd2[0]", close(fd2[0]), -1); 

    WATCHER *ret = malloc(sizeof(WATCHER));
    SYSCALL_ERR_EXIT("malloc", ret, NULL);

    ret->proc_conn_info.name     = type->name;
    ret->proc_conn_info.read_fd  = fd1[0]; 
    ret->proc_conn_info.write_fd = fd2[1];
    ret->proc_conn_info.proc_id  = c_pid; 
    ret->trace = false;

    ret->argv = type->argv;
    ret->args = args;

    ret->w_type = type; 
    ret->w_type->send(ret, args[0]);

    return ret;
}

int bitstamp_watcher_stop(WATCHER *wp) {
    // send kill -9 to wp->proc_conn_info.proc_id
    int res = kill(wp->proc_conn_info.proc_id, SIGTERM);
    return res;
}

int bitstamp_watcher_send(WATCHER *wp, void *arg) {
    // { "event": "bts:subscribe", "data": { "channel": "live_trades_btcusd" } }
    return dprintf(wp->proc_conn_info.write_fd, "{\"event\":\"bts:subscribe\",\"data\":{\"channel\":\"%s\"}}}", (char *)arg);
}

int bitstamp_watcher_recv(WATCHER *wp, char *txt) {
    char *parse_buf = strdup(txt); 
    SYSCALL_ERR_EXIT("strdup txt", parse_buf, NULL);
    FILE *stream = is_valid_server_msg(parse_buf); 
    free(parse_buf);
    if(stream == NULL) {
        return -1;
    }

    ARGO_VALUE *init_av = argo_read_value(stream);
    fclose(stream);
    if(init_av == NULL || argo_value_is_null(init_av)) {
        return -1;
    }
    //argo_value_get_member(ARGO_VALUE *vp, char *name);
    ARGO_VALUE *event_av = argo_value_get_member(init_av, "event");
    if(event_av == NULL || argo_value_is_null(event_av)) {
        argo_free_value(init_av);
        return -1;
    }

    char *trade_s = argo_value_get_chars(event_av);
    if(trade_s == NULL || strcmp(trade_s, "trade") != 0) {
        if(trade_s != NULL)
            free(trade_s);
        argo_free_value(init_av);
        return -1; 
    }
    free(trade_s);

    ARGO_VALUE *channel_av = argo_value_get_member(init_av, "channel");
    if(channel_av == NULL || argo_value_is_null(channel_av)) {
        argo_free_value(init_av);
        return -1;
    }
    char *channel_s = argo_value_get_chars(channel_av);
    if(channel_s == NULL || strncmp(channel_s, "live_trades_", 12) != 0) {
        if(channel_s != NULL)
            free(channel_s);
        argo_free_value(init_av);
        return -1;
    }

    // NOTE:XXX typically always has event, data bc that's how wss data is formatted
    ARGO_VALUE *data_av = argo_value_get_member(init_av, "data");
    if(data_av == NULL || argo_value_is_null(data_av)) {
        free(channel_s);
        argo_free_value(init_av);
        return -1; 
    }
    ARGO_VALUE *price_av = argo_value_get_member(data_av, "price");

    if(price_av == NULL || argo_value_is_null(price_av)) {
        free(channel_s);
        argo_free_value(init_av);
        return -1; 
    }

    ARGO_VALUE *amount_av = argo_value_get_member(data_av, "amount");

    if(amount_av == NULL || argo_value_is_null(amount_av)) {
        free(channel_s);
        argo_free_value(init_av);
        return -1; 
    }

    double price;
    double amount;

    int status = argo_value_get_double(price_av, &price);
    if(status == -1) {
        argo_free_value(init_av);
        free(channel_s);
        return -1;
    }
    status = argo_value_get_double(amount_av, &amount);
    if(status == -1) {
        argo_free_value(init_av);
        free(channel_s);
        return -1;
    }
    char price_a[38] = "bitstamp.net:";
    strcat(price_a, channel_s);
    strcat(price_a, ":price");

    char amount_a[40] = "bitstamp.net:";
    strcat(amount_a, channel_s);
    strcat(amount_a, ":volume");
    free(channel_s);

    struct store_value p_store;
    p_store.type = STORE_DOUBLE_TYPE; 
    p_store.content.double_value = price; 

    int sp_status = store_put(price_a, &p_store);
    if(sp_status == -1) {
        argo_free_value(init_av);
        return -1;
    }

    struct store_value *cur_amount = store_get(amount_a);
    if(cur_amount == NULL) {
        p_store.type = STORE_DOUBLE_TYPE;
        p_store.content.double_value = 0.0;

        sp_status = store_put(amount_a, &p_store);
        if(sp_status == -1) {
            argo_free_value(init_av);
            return -1;
        }
    } else {
        p_store.type = STORE_DOUBLE_TYPE;
        p_store.content.double_value = amount + cur_amount->content.double_value;
        store_free_value(cur_amount);

        sp_status = store_put(amount_a, &p_store);
        if(sp_status == -1) {
            argo_free_value(init_av);
            return -1;
        }
    }

    argo_free_value(init_av);
    return 0;
}

int bitstamp_watcher_trace(WATCHER *wp, int enable) {
    if(enable == 0) {
        wp->trace = 0;
    } else {
        wp->trace = 1;
    }
    return 0;
}
