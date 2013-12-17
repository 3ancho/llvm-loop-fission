#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#define N 10
int main()
{
        unsigned int i;
        int a[N]={0}, b[N]={0}, c[N]={0}, d[N]={0};
        int k;
        k = 10; a[0] = k; a[3] = k*2; c[1] = k+1;
        for (i = 2; i < N-1; i++) {
                a[i] = k*i;
                b[i] = a[i-2]+k;
    c[i] = b[i] + a[i+1];
    d[i] = c[i-1] + k + i;
        }
        printf("%d %d %d %d\n", a[N-2],b[N-1],c[N-2],d[N-2]);
        return 0;
}
