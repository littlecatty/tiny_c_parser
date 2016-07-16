#include <stdio.h>
int sum(int a,int b)
{
    return a+b;
}
int main()
{
  int x = 10;
  int y = 40;
  int c = 0;
  c = sum(x,y);
  printf("sum %d\n",c);
  return 0;
}
