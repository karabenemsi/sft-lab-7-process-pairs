/// File:
/// A fault injector that randomly aborts the program

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>

const double crash_threshold = 0.325;

// Do not use the library constructor function because if drand48
// is not called before the fork, all children will starts
// with the same seed value
static void init() {
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    srand48(time(NULL) ^ (getpid()<<16));
}

static bool should_abort() {
    static bool first = true;
    if (first) {
        first = false;
        init();
    }
    return drand48() < crash_threshold;
}

typedef int (*socket_type)(int, int, int);

int socket(int domain, int type, int protocol) {
  static socket_type orig = 0;
  if (should_abort()) {
    abort();
  }
  if (0 == orig) {
    orig = (socket_type) dlsym(RTLD_NEXT, "socket");
    assert (orig && "original socket function not found");
  }
  return orig(domain, type, protocol);
}

ssize_t recv(int s, void *buf, size_t len, int flags) {
  // handle recv fault injection
  return 0;
}
struct hostent *gethostbyname(const char *name) {
  // handle gethostbyname fault injection
  return 0;
}
