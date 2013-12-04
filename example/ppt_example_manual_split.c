#include <stdio.h>

int main() {

    int a [5]= {0,1,2,3,4};
    int b [5]= {0};
    int c [5]= {0};
    int d [5]= {0};
    int i;

    for (i=1; i<5; i++) {
        b[i] = c[i-1] + 5; 
        c[i] = b[i] * 2 ;      
    }

    for (i=1; i<5; i++) 
        a[i] = a[i] + b[i-1];    

    for (i=1; i<5; i++) 
        d[i] = c[i] + 1;   

    // print out 
    for (i=0; i<5; i++ ) {
        printf("a[%d] = %d, b[%d] = %d, c[%d] = %d, d[%d] = %d\n", i, a[i], i, b[i], i, c[i], i, d[i]);
    }
}
