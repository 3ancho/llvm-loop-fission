#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#define N 100
int main(int argc, char const* argv[])
{
  unsigned int i;
  int a[N]={0}, b[N]={0}, c[N]={0}, d[N]={0};
  int k;
  
  assert(argc > 1);
  k = atoi(argv[1]);
  a[0] = k; a[3] = k*2;
  c[1] = k+1;
  for (i = 2; i < N-1; i++)
  {
    a[i] = k * i; //S1
	b[i] = a[i-2] + k; //S2
	c[i] = b[i] + a[i+1]; //S3
	d[i] = c[i-1] + k + i; //S4
  }
  printf("%d %d %d %d\n", a[N-2], b[N-1], c[N-2], d[N-2]);
  return 0;
 }
  