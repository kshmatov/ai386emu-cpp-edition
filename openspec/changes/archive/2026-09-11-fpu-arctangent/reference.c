#include <stdio.h>
#include <math.h>
static void bytes(long double *x){for(int b=0;b<10;b++)printf("%02x",((unsigned char*)x)[b]);}
int main(void){
 long double v[]={-INFINITY,-2.5L,-1.L,-0.25L,-0.L,0.L,0.25L,1.L,2.5L,INFINITY,0x1p-16445L};
 for(int pc=0;pc<4;pc++)if(pc!=1)for(int rc=0;rc<4;rc++)for(int i=0;i<11;i++)for(int j=0;j<11;j++){
  long double x=v[i],y=v[j],o;unsigned short cw=0x7f|(pc<<8)|(rc<<10),sw;
  __asm__ volatile("fninit; fldcw %4; fldt %3; fldt %2; fpatan; fnstsw %%ax; fstpt %1":"=a"(sw),"=m"(o):"m"(x),"m"(y),"m"(cw):"st");
  printf("%04x ",cw);bytes(&x);printf(" ");bytes(&y);printf(" ");bytes(&o);printf(" %04x\n",sw);
 }
}
