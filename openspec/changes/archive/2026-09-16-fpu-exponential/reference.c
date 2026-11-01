#include <stdio.h>
int main(int argc, char **argv) {
 if(argc>1) {
  for(int rc=0;rc<4;rc++) for(int pc=0;pc<4;pc++) {
   if(pc==1) continue;
   for(int i=-16;i<=16;i++) {
    long double x=(long double)i/16.L,out; unsigned short cw=0x7f|(pc<<8)|(rc<<10),sw;
    __asm__ volatile("fninit; fldcw %2; fldt %3; f2xm1; fnstsw %%ax; fstpt %1":"=a"(sw),"=m"(out):"m"(cw),"m"(x):"st");
    printf("%04x %d ",cw,i);
    for(int b=0;b<10;b++) printf("%02x",((unsigned char*)&out)[b]);
    printf(" %04x\n",sw);
   }
  }
  return 0;
 }

 long double values[]={-1.L,-0.5L,-0.L,0.L,0.5L,1.L,0x1p-16445L};
 unsigned short controls[]={0x037f,0x035f,0x036f,0x037d};
 for(int mode=0;mode<4;mode++) for(int i=0;i<7;i++) {
  long double x=values[i], out; unsigned short cw=controls[mode],sw; unsigned char env[28];
  __asm__ volatile("fninit; fldcw %3; fldt %4; f2xm1; fnstsw %%ax; fnstenv %2; fstpt %1":"=a"(sw),"=m"(out),"=m"(env):"m"(cw),"m"(x):"st");
  printf("cw=%04x x=%La out=%La sw=%04x\n",cw,x,out,sw);
 }
}
