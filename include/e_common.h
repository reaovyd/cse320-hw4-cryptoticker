#ifndef E_COMMON_H
#define E_COMMON_H

#define SYSCALL_ERR_EXIT(syscall_name, lcomp, rcomp) \
    do {                                             \
        if((lcomp) == (rcomp)) {                     \
            perror(syscall_name);                    \
            exit(1);                                 \
        }                                            \
    } while(0);                                      \

#define UWSC_PATHNAME "uwsc"

extern void set_async_nonblock(int fd, pid_t pid);

#endif /* E_COMMON_H */
