#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#define N 10
int main(int argc, char const* argv[])
{
	unsigned int i;
	int a[N]={0}, b[N]={0}, c[N]={0};
	int k=10;
	a[0] = k;
	for (i = 1; i < N; i++) {
		a[i] = c[i];
		b[i] = a[i-1]+1;
	}
	printf("%d %d\n", a[N-1],b[N-1]);
	return 0;
}
