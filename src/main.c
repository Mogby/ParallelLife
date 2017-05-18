#include <dlfcn.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>

void terminateHandler(int signalNumber) {
    printf("Signal %d occured\n", signalNumber);
}

int main() {
    void *mylibHandle = dlopen("/home/mortie/CLionProjects/ParallelLife/src/libmylib.so", RTLD_NOW);

    void (*myPrintf)(const char*, ...);

    if (!mylibHandle) {
        perror(dlerror());
        exit(1);
    }

    *(void**) (&myPrintf) = dlsym(mylibHandle, "myPrintf");

    puts("Testing myPrintf:");
    myPrintf("blah %d %s", 15, "WHO'S YOUR DADDY?");

    int pipeToChild[2];
    int pipeToParent[2];

    pipe(pipeToChild);
    pipe(pipeToParent);

    pid_t child;

    signal(SIGINT, terminateHandler);

    fflush(stdout);

    puts("Using pipes to find sum of numbers:");
    if (child = fork()) {
        close(pipeToChild[0]);
        close(pipeToParent[1]);

        puts("Input number of integers:");
        u_int32_t count;
        scanf("%u", &count);

        int *integers = malloc(sizeof(int) * count);
        for (u_int32_t index = 0; index < count; ++index) {
            scanf("%d", integers + index);
        }

        write(pipeToChild[1], &count, sizeof(u_int32_t));
        for (u_int32_t index = 0; index < count; ++index) {
            write(pipeToChild[1], integers + index, sizeof(int));
        }

        int sum;
        read(pipeToParent[0], &sum, sizeof(int));
        printf("Sum: %d\n", sum);
    } else {
        close(pipeToChild[1]);
        close(pipeToParent[0]);

        u_int32_t count;
        read(pipeToChild[0], &count, sizeof(u_int32_t));

        int integer;
        int sum = 0;
        for (u_int32_t index = 0; index < count; ++index) {
            read(pipeToChild[0], &integer, sizeof(int));
            sum += integer;
        }

        write(pipeToParent[1], &sum, sizeof(int));
    }

    return 0;
}