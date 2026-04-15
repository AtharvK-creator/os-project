#include <stdio.h>
#include <unistd.h>

int main() {
    for (int i = 0; i < 20; i++) {
        printf("IO pulse %d\n", i);
        sleep(1);
    }
    return 0;
}
