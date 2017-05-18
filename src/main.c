#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int main() {
    void *mylibHandle = dlopen("/home/mortie/CLionProjects/ParallelLife/src/libmylib.so", RTLD_NOW);

    void (*myPrintf)(const char*, ...);

    if (!mylibHandle) {
        perror(dlerror());
        exit(1);
    }

    *(void**) (&myPrintf) = dlsym(mylibHandle, "myPrintf");

    myPrintf("blah %d %s", 15, "WHO'S YOUR DADDY?");

    return 0;
}