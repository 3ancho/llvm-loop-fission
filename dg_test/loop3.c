#include <stdio.h>
#define N 10
int main()
{
  unsigned int i;
  int a[N], b[N], c[N], d[N];
  for (i = 0; i < N; i++)
  {
    a[i] = i; //S1
  	b[i] = i + 1; //S2
	  c[i] = b[i] + 1; //S3
	  d[i] = 2*i; //S4
  }
  printf ("%d%d%d%d\n", a[N-2], b[N-1], c[N-2], d[N-2]);
  return 0;
 }
  
