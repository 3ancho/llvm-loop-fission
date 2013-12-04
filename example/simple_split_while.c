#include <stdio.h>

int main() {
    int i = 0;
    int a = 0;
    int b = 0;

    while (i<5) {
        a += i;
        b = i;
        i++;
    }

    printf("a=%d, b=%d\n", a, b);
    return 0;
}
