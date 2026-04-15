#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>   
#include <signal.h>   
void check_containers() {
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    int found = 0;

    printf("\n[supervisor] Checking running containers:\n");

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0) {

                char path[256], name[256];
                sprintf(path, "/proc/%d/comm", pid);

                FILE *f = fopen(path, "r");
                if (f) {
                    fgets(name, sizeof(name), f);
                    name[strcspn(name, "\n")] = 0;

                    // STRICT CHECK
                    if ((strcmp(name, "alpha") == 0 || strcmp(name, "beta") == 0)
                        && kill(pid, 0) == 0) {

                        printf("✔ Container %s running with PID %d\n", name, pid);
                        found = 1;
                    }

                    fclose(f);
                }
            }
        }
    }

    if (!found) {
        printf("No containers running\n");
    }

    closedir(dir);
}

int main() {
    printf("Supervisor started...\n");

    for (int i = 0; i < 15; i++) {
        sleep(2);
        check_containers();
    }

    printf("\nSupervisor exiting...\n");
    return 0;
}
