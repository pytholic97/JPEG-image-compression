#include <stdio.h>
#include <stdlib.h>

void dctt(int *b[]){
  printf("%d %d \n",b[0][0],b[2][3]);
}

int main(int argc, char const *argv[]) {
  int *bbb[8];

  int q50[8][8] = {{16,11,10,16,24,40,51,61},
{12,12,14,19,26,58,60,55},
{14,13,16,24,40,57,69,56},
{14,17,22,29,51,87,80,62},
{18,22,37,56,68,109,103,77},
{24,35,55,64,81,104,113,92},
{49,64,78,87,103,121,120,101},
{72,92,95,98,112,100,130,99}};


  for(int  i =0;i < 8;i++) bbb[i] = (int *)malloc(8*sizeof(int));
  bbb[0][0] = 5;
  bbb[2][3] = 8;
  dctt(bbb);
  for(int  i =0;i < 8;i++) free(bbb[i]);
  return 0;
}
