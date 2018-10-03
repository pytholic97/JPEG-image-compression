#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/******* Reminders

To free:
stbi_image_free(data);
for(int i = 0;i < 8;i++){
free(bl);
free(im_dct);
}
********/

int q50[8][8] = {{16,11,10,16,24,40,51,61},
{12,12,14,19,26,58,60,55},
{14,13,16,24,40,57,69,56},
{14,17,22,29,51,87,80,62},
{18,22,37,56,68,109,103,77},
{24,35,55,64,81,104,113,92},
{49,64,78,87,103,121,120,101},
{72,92,95,98,112,100,130,99}};


void dct(int *bl[],float *im_dct[]);
void qtize(float *inp[],int *outp[]);
void inflate_rlc(int *inp,int *outp[]);
void rl_code(int *inp[],int *outp);
void idct(int *inp[],int *outp[]);

int main(int argc, char const *argv[]){
  int x=360,y=240,n=1;
  unsigned char *data = stbi_load("img_00031.bmp", &x, &y, &n, 1);

/*****
  for(int i = 0; i < 8; i++)
    printf("%d %d\n",i,(int)data[360*8+16+i]);
  ***/
  int w=360,h=240;

  unsigned char *im_compr = (unsigned char *)malloc(86400*sizeof(char));

  int *bl[8];
  for(int  i =0;i < 8;i++) bl[i] = (int *)malloc(8*sizeof(int));

  int *im_qtized[8];
  for(int  i =0;i < 8;i++) im_qtized[i] = (int *)malloc(8*sizeof(int));

  float *im_dct[8];
  for(int i = 0;i < 8;i++) im_dct[i] = (float *)malloc(8*sizeof(float));

  int *im_rec[8]; //recovered image from run length code
  for(int i = 0;i < 8;i++) im_rec[i] = (int *)malloc(8*sizeof(int));

  int *rlc = (int *)malloc(128*sizeof(int));
  for(int i =0;i < 128;i++) rlc[i] = -255;

int koi = 0;

  for(int i1 = 0;(i1+8) <= h;i1 += 8)
  for(int j1 = 0;(j1+8) <= w;j1 += 8)
  {
    for(int u1 = 0;u1 < 8;u1++)
    for(int v1 = 0;v1 < 8;v1++)
    {
      bl[u1][v1] = (int)(data[360*i1 + j1 + 360*u1 + v1]) - 128;
      //if(i1 == 0 && j1 == 0) printf("%d ",bl[u1][v1]+128);
    }


    if(i1 == 0 && j1 == 0){
      for(int u1 = 0;u1 < 8;u1++){
      for(int v1 = 0;v1 < 8;v1++)
      {
        printf("%d ",bl[u1][v1]);
      }
      printf("\n");
      }
    }

    dct(bl,im_dct);
    if(i1 == 0 && j1 == 0){
      for(int u1 = 0;u1 < 8;u1++){
      for(int v1 = 0;v1 < 8;v1++)
      {
        printf("%0.1f ",im_dct[u1][v1]);
      }
      printf("\n");
      }
    }
    //qtize(im_dct,im_qtized);
    if(i1 == 0 && j1 == 0){
        qtize(im_dct,im_qtized);
      for(int u1 = 0;u1 < 8;u1++){
      for(int v1 = 0;v1 < 8;v1++)
      {
        printf("%d ",im_qtized[u1][v1]);
      }
      printf("\n");
      }
    }

    for(int i =0;i < 128;i++) rlc[i] = -255;

    //rl_code(im_qtized,rlc);
    if(i1 == 0 && j1 == 0){
        rl_code(im_qtized,rlc);
      for(int u1 = 0;u1 < 85;u1++) printf("%d ",rlc[u1]);
    }
#ifdef STTS
    inflate_rlc(rlc,im_rec);

    idct(im_rec,im_qtized); //stored image idct and added 128
    if(i1 == 0 && j1 == 0) printf("\n");
    for(int u1 = 0;u1 < 8;u1++)
    for(int v1 = 0;v1 < 8;v1++)
    {
      im_compr[360*i1 + j1 + 360*u1 + v1] = (char)(im_dct[u1][v1] + 128);
      //if(i1 == 0 && j1 == 0) printf("%d ",im_compr[360*i1 + j1 + 360*u1 + v1]-128);
    }
  }
  stbi_write_bmp("jpeg_comp.bmp", 360, 240, 1, im_compr);
#endif
}
  stbi_image_free(data);
  for(int i = 0;i < 8;i++){
  free(bl[i]);
  free(im_dct[i]);
  free(im_qtized[i]);
  free(im_rec[i]);
  }
  free(rlc);
  free(im_compr);

  //printf("%d\n",koi);
  return 0;
}


void dct(int *inp[],float *outp[]){

  int i,j,u,v;
  float k,csum = 0.0;

  for(i = 0;i < 8;i++){
    for(j = 0;j < 8;j++){
      for(u = 0;u < 8;u++)
        for(v = 0;v < 8;v++)
          csum += inp[u][v]*cos((2*u+1)*i*M_PI/(16))*cos((2*v+1)*j*M_PI/(16));
      if(!i && !j) k = 0.5;
      else if((!i && j) || (i && !j)) k = 1/sqrt(2);
      else k = 1;
      outp[i][j] = csum*k/4;
      //printf("%.3f ",dct[i][j]);
      csum = 0;
    }
    //printf("\n");
  }
}


void qtize(float *inp[],int *outp[]){

  for(int i = 0;i < 8;i++)
  for(int j = 0;j < 8;j++)
  {
    outp[i][j] = (int)(inp[i][j]/q50[i][j]);
    //if(i == 0) printf("is %d ",outp[i][j]);
  }
}

void rl_code(int *inp[],int *outp){
  int k = 0,z_cnt=0;
  int i=0,j=0;
  outp[0] = inp[0][0];
  int cnt = 1;
  i = 0;
  j = 1;
  k = 1;

  while(k <= 7){
    for(int u = 0;u < k;u++)  //Move down
    {
      if(inp[i][j]){
        outp[cnt++] = z_cnt;
        z_cnt = 0;
        outp[cnt++] = inp[i][j];
      }
      else z_cnt++;

      i++;
      j--;
    }
    if(inp[i][j]){
      outp[cnt++] = z_cnt;
      z_cnt = 0;
      outp[cnt++] = inp[i][j];
    }
    else z_cnt++;

    k++;
    j = 0;
    i++;

    if(k > 8) break;

    for(int u = 0;u < k;u++)   //Move up
    {
      if(inp[i][j]){
        outp[cnt++] = z_cnt;
        z_cnt = 0;
        outp[cnt++] = inp[i][j];
      }
      else z_cnt++;

      i--;
      j++;
    }
    if(inp[i][j]){
      outp[cnt++] = z_cnt;
      z_cnt = 0;
      outp[cnt++] = inp[i][j];
    }
    else z_cnt++;

    k++;
    i = 0;
    j++;

    if(inp[i][j]){
      outp[cnt++] = z_cnt;
      z_cnt = 0;
      outp[cnt++] = inp[i][j];
    }
    else z_cnt++;
  }

  k = 6;
  i = 7;
  j = 1;

  while(k >= 1){
    for(int u = 0;u < k;u++)     //Move up
    {
      if(inp[i][j]){
        outp[cnt++] = z_cnt;
        z_cnt = 0;
        outp[cnt++] = inp[i][j];
      }
      else z_cnt++;

      j++;
      i--;
    }
    if(inp[i][j]){
      outp[cnt++] = z_cnt;
      z_cnt = 0;
      outp[cnt++] = inp[i][j];
    }
    else z_cnt++;
    i++;
    j = 7;
    k--;

    if(!k) break;

    for(int u = 0;u < k;u++)   //Move down
    {
      if(inp[i][j]){
        outp[cnt++] = z_cnt;
        z_cnt = 0;
        outp[cnt++] = inp[i][j];
      }
      else z_cnt++;

      i++;
      j--;
    }
    if(inp[i][j]){
      outp[cnt++] = z_cnt;
      z_cnt = 0;
      outp[cnt++] = inp[i][j];
    }
    else z_cnt++;
    j++;
    k--;
    i = 7;
    if(inp[i][j]){
      outp[cnt++] = z_cnt;
      z_cnt = 0;
      outp[cnt++] = inp[i][j];
    }
    else z_cnt++;
  }

}


void inflate_rlc(int *inp,int *outp[]){
  int lp = 1, cl = 0;

  for(int i = 0;i < 8;i++)
  for(int j = 0;j < 8;j++)
  {
    outp[i][j] = 0;
  }

  outp[0][0] = inp[0];
  int zr = 0;
  int i = 0,j = 1;
  int k = 1;
  while(k <= 7){
    for(int u = 0;u < k;u++)  //Move down
    {
      if(!cl){
        cl = 1;
        zr = inp[lp++];
      }

      if(cl && zr){
        outp[i][j] = 0;
        zr--;
      }
      else if(cl && !zr){
        cl = 0;
        outp[i][j] = inp[lp++];
        zr = 0;
      }

      i++;
      j--;
    }
    if(!cl){
      cl = 1;
      zr = inp[lp++];
    }

    if(cl && zr){
      outp[i][j] = 0;
      zr--;
    }
    else if(cl && !zr){
      cl = 0;
      outp[i][j] = inp[lp++];
      zr = 0;
    }

    k++;
    j = 0;
    i++;

    if(k > 8) break;

    for(int u = 0;u < k;u++)   //Move up
    {
      if(!cl){
        cl = 1;
        zr = inp[lp++];
      }

      if(cl && zr){
        outp[i][j] = 0;
        zr--;
      }
      else if(cl && !zr){
        cl = 0;
        outp[i][j] = inp[lp++];
        zr = 0;
      }

      i--;
      j++;
    }
    if(!cl){
      cl = 1;
      zr = inp[lp++];
    }

    if(cl && zr){
      outp[i][j] = 0;
      zr--;
    }
    else if(cl && !zr){
      cl = 0;
      outp[i][j] = inp[lp++];
      zr = 0;
    }

    k++;
    i = 0;
    j++;

    if(!cl){
      cl = 1;
      zr = inp[lp++];
    }

    if(cl && zr){
      outp[i][j] = 0;
      zr--;
    }
    else if(cl && !zr){
      cl = 0;
      outp[i][j] = inp[lp++];
      zr = 0;
    }
  }

  k = 6;
  i = 7;
  j = 1;

  while(k >= 1){
    for(int u = 0;u < k;u++)     //Move up
    {
      if(!cl){
        cl = 1;
        zr = inp[lp++];
      }

      if(cl && zr){
        outp[i][j] = 0;
        zr--;
      }
      else if(cl && !zr){
        cl = 0;
        outp[i][j] = inp[lp++];
        zr = 0;
      }

      j++;
      i--;
    }
    if(!cl){
      cl = 1;
      zr = inp[lp++];
    }

    if(cl && zr){
      outp[i][j] = 0;
      zr--;
    }
    else if(cl && !zr){
      cl = 0;
      outp[i][j] = inp[lp++];
      zr = 0;
    }
    i++;
    j = 7;
    k--;

    if(!k) break;

    for(int u = 0;u < k;u++)   //Move down
    {
      if(!cl){
        cl = 1;
        zr = inp[lp++];
      }

      if(cl && zr){
        outp[i][j] = 0;
        zr--;
      }
      else if(cl && !zr){
        cl = 0;
        outp[i][j] = inp[lp++];
        zr = 0;
      }

      i++;
      j--;
    }
    if(!cl){
      cl = 1;
      zr = inp[lp++];
    }

    if(cl && zr){
      outp[i][j] = 0;
      zr--;
    }
    else if(cl && !zr){
      cl = 0;
      outp[i][j] = inp[lp++];
      zr = 0;
    }
    j++;
    k--;
    i = 7;
    if(!cl){
      cl = 1;
      zr = inp[lp++];
    }

    if(cl && zr){
      outp[i][j] = 0;
      zr--;
    }
    else if(cl && !zr){
      cl = 0;
      outp[i][j] = inp[lp++];
      zr = 0;
    }
  }

}

void idct(int *inp[],int *outp[]){
  int i,j,u,v;
  float csum = 0,k;
  for(i = 0;i < 8;i++){
    for(j = 0;j < 8;j++){
      for(u = 0;u < 8;u++)
        for(v = 0;v < 8;v++){
          if(!u && !v) k = 0.5;
          else if((!u && v) || (u && !v)) k = 1/sqrt(2);
          else k = 1;
          csum += k*inp[u][v]*q50[u][v]*cos((2*i+1)*M_PI*u/(16))*cos((2*j+1)*M_PI*v/(16));
        }
      outp[i][j] = (int)csum/4;
      csum = 0;
      //printf("%0.1f ",idct[i][j]);
    }
    //printf("\n");
  }
}
