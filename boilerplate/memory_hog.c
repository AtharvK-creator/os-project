#include <stdlib.h>
#include <unistd.h>

int main() {
    while (1) {
        void *ptr = malloc(1024 * 1024);
        sleep(1);
    }
}
