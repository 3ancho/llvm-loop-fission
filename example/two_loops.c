#include <stdio.h>

int main() {
    int i;
    int j;
    int a = 0;
    int b = 0;
    for (i=0; i<5; i++) {
        a += i;
        b = i;
    }

    for (j=0; j<5; j++) {
        b = j;
    }

    printf("a=%d, b=%d\n", a, b);
    return 0;
}
