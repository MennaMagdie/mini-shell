#include <stdio.h>
#include <unistd.h> //fork()
#include <sys/types.h> //pid_t

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        printf("Hello from the child process! PID: %d\n", getpid());
    } else {
        printf("Hello from the parent process! Child PID: %d\n", pid);
    }

    return 0;
}
