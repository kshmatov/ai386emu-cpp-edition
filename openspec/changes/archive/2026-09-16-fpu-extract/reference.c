#include <stdio.h>
#include <math.h>
int main(void){
 for(int kind=0;kind<4;kind++){
  long double x=kind==3?-0.L:1.L,a,b;unsigned short sw;
  __asm__ volatile("fninit":::"st");
  if(kind!=0) __asm__ volatile("fldt %0"::"m"(x):"st");
  if(kind==2) for(int i=0;i<7;i++) __asm__ volatile("fld1":::"st");
  __asm__ volatile("fxtract; fnstsw %%ax; fstpt %0; fstpt %1":"=m"(a),"=m"(b),"=a"(sw)::"st");
  printf("%d %.1Lf %.1Lf %04x\n",kind,a,b,sw);
 }
}
