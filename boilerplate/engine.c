#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 10
#define MAX_MSG 256
#define STACK_SIZE 1024 * 1024
#define MAX_CONTAINERS 10

// ---------------- BUFFER ----------------
char buffer[BUFFER_SIZE][MAX_MSG];
int in = 0, out = 0, count_buf = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

// ---------------- CONTAINER STRUCT ----------------
struct container {
    char name[50];
    int pid;
    int running;
};

struct container containers[MAX_CONTAINERS];
int count = 0;

// ---------------- ARG STRUCT ----------------
struct container_args {
    char name[50];
    char rootfs[100];
    int pipe_write;
};

// ---------------- CONSUMER ----------------
void *consumer(void *arg) {
    FILE *log = fopen("logs.txt", "a");

    while (1) {
        pthread_mutex_lock(&mutex);

        while (count_buf == 0)
            pthread_cond_wait(&not_empty, &mutex);

        char msg[MAX_MSG];
        strcpy(msg, buffer[out]);

        out = (out + 1) % BUFFER_SIZE;
        count_buf--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        fprintf(log, "%s", msg);
        fflush(log);
    }
    return NULL;
}

// ---------------- PRODUCER ----------------
void *producer(void *arg) {
    int fd = *(int *)arg;
    char msg[MAX_MSG];

    while (read(fd, msg, sizeof(msg)) > 0) {
        pthread_mutex_lock(&mutex);

        while (count_buf == BUFFER_SIZE)
            pthread_cond_wait(&not_full, &mutex);

        strcpy(buffer[in], msg);
        in = (in + 1) % BUFFER_SIZE;
        count_buf++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int child_func(void *arg) {
    struct container_args *args = (struct container_args *)arg;

    char *name = args->name;
    char *rootfs = args->rootfs;

    // Set process name
    prctl(PR_SET_NAME, name, 0, 0, 0);

    // Redirect logs
    dup2(args->pipe_write, STDOUT_FILENO);
    dup2(args->pipe_write, STDERR_FILENO);

    printf("🔥 Container %s started!\n", name);

    sethostname("jackfruit", strlen("jackfruit"));

    if (chroot(rootfs) != 0) {
        perror("chroot failed");
        exit(1);
    }

    chdir("/");
    mount("proc", "/proc", "proc", 0, NULL);

    // 🔥 MEMORY + CPU BEHAVIOR

    if (strcmp(name, "alpha") == 0) {
        // 🔴 HEAVY: CPU + MEMORY
        while (1) {
            malloc(1024 * 1024);  // allocate 1MB repeatedly
        }

    } else {
        // 🟢 LIGHT: LOW CPU + LOW MEMORY
        while (1) {
            for (volatile int i = 0; i < 100000000; i++);
            sleep(1);
        }
    }

    return 0;
}

// ---------------- START ----------------
void start_container(char *name) {
    if (count >= MAX_CONTAINERS) {
        printf("Max containers reached!\n");
        return;
    }

    int pipefd[2];
    pipe(pipefd);

    pthread_t prod, cons;

    pthread_create(&cons, NULL, consumer, NULL);
    pthread_create(&prod, NULL, producer, &pipefd[0]);

    void *stack = malloc(STACK_SIZE);

    struct container_args *args = malloc(sizeof(struct container_args));
    strcpy(args->name, name);
    sprintf(args->rootfs, "rootfs-%s", name);
    args->pipe_write = pipefd[1];

    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD;

    int pid = clone(child_func, stack + STACK_SIZE, flags, args);

    if (pid == -1) {
        perror("clone failed");
        return;
    }

    strcpy(containers[count].name, name);
    containers[count].pid = pid;
    containers[count].running = 1;
    count++;

    printf("✔ Container '%s' started with PID %d\n", name, pid);
}

// ---------------- LIST ----------------
void list_containers() {
    printf("\nRunning Containers:\n");
    for (int i = 0; i < count; i++) {
        if (containers[i].running) {
            printf("%s → PID %d\n", containers[i].name, containers[i].pid);
        }
    }
}

// ---------------- STOP ----------------
void stop_container(char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(containers[i].name, name) == 0 && containers[i].running) {
            kill(containers[i].pid, SIGKILL);
            waitpid(containers[i].pid, NULL, 0);
            containers[i].running = 0;

            printf("🛑 Stopped container '%s'\n", name);
            return;
        }
    }
    printf("Container not found!\n");
}

// ---------------- COUNT ----------------
void count_processes() {
    FILE *fp = fopen("/proc/loadavg", "r");
    int proc;
    fscanf(fp, "%*f %*f %*f %d/", &proc);
    fclose(fp);

    printf("📊 Process count from kernel: %d\n", proc);
}

// ---------------- MAIN ----------------
int main() {
    char cmd[100];

    printf("🚀 Jackfruit CLI Started\n");

    while (1) {
        printf("jackfruit> ");
        scanf("%s", cmd);

        if (strcmp(cmd, "start") == 0) {
            char name[50];
            scanf("%s", name);
            start_container(name);

        } else if (strcmp(cmd, "list") == 0) {
            list_containers();

        } else if (strcmp(cmd, "stop") == 0) {
            char name[50];
            scanf("%s", name);
            stop_container(name);

        } else if (strcmp(cmd, "count") == 0) {
            count_processes();

        } else if (strcmp(cmd, "exit") == 0) {
            printf("Exiting...\n");
            break;

        } else {
            printf("Commands: start <name>, list, stop <name>, count, exit\n");
        }
    }

    return 0;
}
