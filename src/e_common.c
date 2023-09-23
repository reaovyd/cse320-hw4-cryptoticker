#include <sys/types.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#include "e_common.h"

void set_async_nonblock(int fd, pid_t pid) {
    SYSCALL_ERR_EXIT("fcntl fd F_SETOWN pid", fcntl(fd, F_SETOWN, pid), -1); 
    SYSCALL_ERR_EXIT("fcntl fd F_SETFL O_ASYNC | O_NONBLOCK", fcntl(fd, F_SETFL, O_ASYNC | O_NONBLOCK), -1);
    SYSCALL_ERR_EXIT("fcntl fd F_SETSIG SIGIO", fcntl(fd, F_SETSIG, SIGIO), -1);
}
