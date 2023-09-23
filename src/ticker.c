#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "builtins.h"
#include "ticker.h"
#include "watcher_table.h"
#include "debug.h"
#include "e_common.h"
#include "watcher_common.h"

watcher_table *gp_WATCHER_TABLE;

extern volatile sig_atomic_t EXISTS_CHILD_QUIT_FLAG;
extern volatile sig_atomic_t CHILD_QUIT_LENGTH;

static volatile sig_atomic_t fd_flag = 0;
static char *prompt = "ticker> ";
static char *parse_error = "???\n";

static void sigio_handler(int signum, siginfo_t *s_info, void *ctx) {
    fd_flag = 1;
}

static void sigchld_handler(int signum, siginfo_t *s_info, void *ctx) {
    int errno_tmp = errno;
    pid_t p;

    sigset_t cur_mask, prev_mask;
    sigfillset(&cur_mask);

    while((p = waitpid(-1, NULL, WNOHANG)) > 0) {
        if(EXISTS_CHILD_QUIT_FLAG == 1) {
            size_t tmp = CHILD_QUIT_LENGTH + 1;
            CHILD_QUIT_LENGTH = tmp;
        }
        sigprocmask(SIG_BLOCK, &cur_mask, &prev_mask);
        remove_entry_by_pid(gp_WATCHER_TABLE, p);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    }

    errno = errno_tmp;
}

static void sigint_handler(int signum, siginfo_t *s_info, void *ctx) {
    run_builtin(BUILTIN_QUIT_TYPE, gp_WATCHER_TABLE, NULL); 
}

static void signal_init(int signum, 
        void (*sigaction_handler)(int, siginfo_t *, void *)) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigaction_handler;

    SYSCALL_ERR_EXIT("sigaction", sigaction(signum, &act, NULL), -1);
}

static WATCHER *start_cli(WATCHER_TYPE *cli_type) {
    WATCHER *new_watcher = cli_type->start(cli_type, NULL);

    WATCHER *ret = add_new_entry(gp_WATCHER_TABLE, 
            new_watcher->proc_conn_info.name,
            new_watcher->proc_conn_info.proc_id,
            new_watcher->proc_conn_info.read_fd,
            new_watcher->proc_conn_info.write_fd,
            cli_type,
            false,
            new_watcher->argv,
            NULL);
    free(new_watcher);
    return ret;
}

int ticker(void) {
    bool has_stdin_input = isatty(STDIN_FILENO) == 0;
    set_async_nonblock(STDIN_FILENO, getpid());
    signal_init(SIGIO, sigio_handler);
    signal_init(SIGCHLD, sigchld_handler);
    signal_init(SIGINT, sigint_handler);
    gp_WATCHER_TABLE = table_init();
    WATCHER_TYPE wt_cli = watcher_types[CLI_WATCHER_TYPE];
    start_cli(&wt_cli);

    sigset_t mask_sigio, prev_mask_sigio;
    sigemptyset(&mask_sigio);
    sigaddset(&mask_sigio, SIGIO);
    sigprocmask(SIG_BLOCK, &mask_sigio, &prev_mask_sigio);
    if(has_stdin_input)
        raise(SIGIO);

    deque_reader *cli_dr = (get_watcher_entry_by_idx(gp_WATCHER_TABLE, 0))->dr;  
    wt_cli.send(NULL, prompt);
    do {
        fd_flag = 0;
        while(!fd_flag)
            sigsuspend(&prev_mask_sigio);
        while(errno != EWOULDBLOCK) {
            char static_buf[512] = {0};
            ssize_t n_read = 0;
            while((n_read = read(STDIN_FILENO, &static_buf, 512)) > 0) {
                for(int i = 0; i < n_read; ++i) {
                    dr_write_char(cli_dr, static_buf[i]);
                    char *cc = dr_get_avail_input(cli_dr);
                    if(cc == NULL) {
                        continue;
                    } else {
                        char *recv_parse_check = strdup(cc);
                        trace_print(get_watcher_entry_by_idx(gp_WATCHER_TABLE, 0), cc);
                        int bo = wt_cli.recv(
                                get_watcher_entry_by_idx(gp_WATCHER_TABLE, 0), recv_parse_check);
                        free(recv_parse_check);
                        if(bo < 0) {
                            free(cc);
                            wt_cli.send(NULL, parse_error);
                            wt_cli.send(NULL, prompt);
                        } else {
                            if(bo == BUILTIN_QUIT_TYPE) {
                                free(cc);
                                wt_cli.send(NULL, prompt);
                                run_builtin(bo, gp_WATCHER_TABLE, NULL);
                            } else {
                                int res = run_builtin(bo, gp_WATCHER_TABLE, cc);
                                free(cc);
                                if(res < 0) {
                                    wt_cli.send(NULL, parse_error);
                                }
                                wt_cli.send(NULL, prompt);
                            }
                        }
                    }
                }
            }
            if(n_read == 0) {
                run_builtin(BUILTIN_QUIT_TYPE, gp_WATCHER_TABLE, NULL);  
            } else if(n_read < 0) {
                break;
            }
        }

        sigset_t child_mask, prev_child_mask;
        sigemptyset(&child_mask);
        sigaddset(&child_mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &child_mask, &prev_child_mask);
        for(size_t i = 1; i < gp_WATCHER_TABLE->capacity; ++i) {
            watcher *wt = &gp_WATCHER_TABLE->table_arr[i]; 
            if(!is_empty_watcher_entry(wt)) {
                char c_static_buf[2048] = {0};
                ssize_t n_read;
                while((n_read = read(wt->proc_conn_info.read_fd, &c_static_buf, 2048)) > 0) {
                    for(int i = 0; i < n_read; ++i) {
                        dr_write_char(wt->dr, c_static_buf[i]); 
                        char *cc = NULL;
                        cc = dr_get_avail_input(wt->dr);
                        if(cc == NULL) {
                            continue;
                        } else {
                            char *sub_p = cc;
                            trace_print(wt, cc);
                            wt->w_type->recv(wt, sub_p);
                            free(cc);
                        }
                    }
                }
                errno = 0;
            }
        }
        sigprocmask(SIG_SETMASK, &prev_child_mask, NULL);
    } while(1);
    return 0;
}
