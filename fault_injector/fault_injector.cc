// fault injector aborts program non deterministically

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <netdb.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>

// 
const double crash_trashhold = 0.025;;

// do not use the library constructor function -
// if drand48 is not called before the fork, childs starts
// with the same seed
//void init () __attribute__ ((constructor));
static void init() {
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    seed48((unsigned short *) &now);
}

static bool should_abort() {
    static bool first = true;
    if (first) {
        first = false;
        init();
    }
    return drand48() < crash_trashhold;
}

#define jump_to(func)                                                                 \
    asm("movl %ebp,%esp");                                                            \
    asm("popl %ebp");                                                                 \
    asm("jmp  *libc_" #func);

#define resolver(func)                                                                \
static void unresolved_##func();                                                      \
__typeof(func)* libc_##func = (__typeof(func)*) unresolved_##func;                    \
static void unresolved_##func()                                                       \
{                                                                                     \
    if (!libc_##func || (__typeof(func)*)unresolved_##func == libc_##func)            \
    {                                                                                 \
        libc_##func = (__typeof(func)*)dlsym (RTLD_NEXT, #func);                      \
        if (NULL == libc_##func)                                                      \
        {                                                                             \
            perror ("Could find orignal " #func " function");                         \
            abort ();                                                                 \
        }                                                                             \
    }                                                                                 \
                                                                                      \
    jump_to(func)                                                                     \
}

resolver(socket)
int socket(int domain, int type, int protocol) {
    if (should_abort())
        abort();

    jump_to(socket)
}

resolver(recv)
ssize_t recv(int s, void *buf, size_t len, int flags) {
    if (should_abort())
        abort();

    jump_to(recv)
}

resolver(gethostbyname)
struct hostent *gethostbyname(const char *name) {
    if (should_abort())
        abort();

    jump_to(gethostbyname)
}
