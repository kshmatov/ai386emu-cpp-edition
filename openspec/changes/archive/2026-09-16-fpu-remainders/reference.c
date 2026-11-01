#include <stdio.h>
int main(void) {
 for(int mode=0;mode<2;mode++) for(int x=-9;x<=9;x++) {
  long double a=x,b=2,r; unsigned short s;
  if(mode) __asm__ volatile("fldt %3; fldt %2; fprem1; fnstsw %%ax; fstpt %0; fstp %%st(0)":"=m"(r),"=a"(s):"m"(a),"m"(b):"st");
  else __asm__ volatile("fldt %3; fldt %2; fprem; fnstsw %%ax; fstpt %0; fstp %%st(0)":"=m"(r),"=a"(s):"m"(a),"m"(b):"st");
  printf("%d %d %.1Lf %04x\n",mode,x,r,s&0x4700);
 }
}
