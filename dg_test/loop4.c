#include <stdio.h>
#define N 10
int main()
{
  unsigned int i;
  int a[N], b[N], c[N], d[N];
  for (i = 1; i < N; i++)
  {
    b[i] = i;
    a[i] = a[i-1]+b[i]; //S1
	d[i] = c[i] + 2; //S4
  }
  printf ("%d%d%d%d\n", a[N-2], b[N-1], c[N-2], d[N-2]);
  return 0;
 }
  
