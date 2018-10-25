#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


//#define M_PI 3.14


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

int main(int argc, char *argv[]){

	if(argc != 4){
	printf("Invalid args\n");
	exit(1);
	}
  int x=360,y=240,n=1;
  x = atoi(argv[2]);
  y = atoi(argv[3]);

	unsigned char *data = stbi_load(argv[1], &x, &y, &n, 1);

	int w=x,h=y;

  unsigned char *im_compr = (unsigned char *)malloc(w*h*sizeof(char));

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
	int cnc1 = 0;

	for(int i1 = 0;(i1+8) <= h;i1 += 8)
  for(int j1 = 0;(j1+8) <= w;j1 += 8)
  {
		if(i1 == 0 && j1 == 0) printf("\n-----------Normalized matrix--------------\n");
    for(int u1 = 0;u1 < 8;u1++){
    for(int v1 = 0;v1 < 8;v1++)
    {
			bl[u1][v1] = (int)(data[w*i1 + j1 + w*u1 + v1]) - 128;
      if(i1 == 0 && j1 == 0) printf("%d ",bl[u1][v1]);
		}
		if(i1 == 0 && j1 == 0) printf("\n");
	}

	if(i1 == 0 && j1 == 0) printf("\n-------------DCT----------------\n");
    dct(bl,im_dct);
	if(i1 == 0 && j1 == 0)
	for(int itr1 = 0; itr1 < 8;itr1++){
		for(int i2 = 0;i2 < 8;i2++) printf("%.2f ",im_dct[itr1][i2]);
		printf("\n");
	}

    qtize(im_dct,im_qtized);
	if(i1 == 0 && j1 == 0) printf("\n-----------Quantized matrix-----------\n");
	if(i1 == 0 && j1 == 0)
		for(int i2 = 0;i2 < 8;i2++){
			for(int i3 = 0;i3 < 8;i3++) printf("%d ",im_qtized[i2][i3]);
			printf("\n");
		}
  for(int i =0;i < 128;i++) rlc[i] = -255;

  rl_code(im_qtized,rlc);

	if(i1 == 0 && j1 == 0) printf("\n-----------Run length code-----------\n");

		for(int i2 = 0;rlc[i2] != -255;i2++){
			if(i1 == 0 && j1 == 0) printf("%d ",rlc[i2]);
			cnc1++;
		}
  inflate_rlc(rlc,im_rec);

  idct(im_rec,im_qtized); //stored image idct
	if(i1 == 0 && j1 == 0) printf("\n-----------Original image---------\n");
	if(i1 == 0 && j1 == 0)
		for(int i2 = 0;i2 < 8;i2++){
			for(int i3 = 0;i3 < 8;i3++) printf("%d ",im_qtized[i2][i3]);
			printf("\n");
		}

    for(int u1 = 0;u1 < 8;u1++)
    for(int v1 = 0;v1 < 8;v1++)
    {
      im_compr[w*i1 + j1 + w*u1 + v1] = (char)(im_qtized[u1][v1] + 128);
      //if(i1 == 0 && j1 == 0) printf("%d %d ;",im_compr[360*i1 + j1 + 360*u1 + v1]-128,(int)(data[360*i1 + j1 + 360*u1 + v1]));
    }

  }

	float mse = 0;
	printf("\n----------Mean square error is-----------\n");
	int mx = -300;;
  for(int j1 = 0;j1 < h*w;j1++){
	int jk = (int)data[j1];
	if(jk > mx) mx = jk;
	int kl = (int)im_compr[j1];
	mse += pow(jk - kl,2);
	}
	printf("MSE = %f max value = %d\n",sqrt(mse)/(h*w),mx);

	printf("Compressed length in bytes is %d\n",cnc1);
  stbi_write_bmp("jpeg_comp.bmp", w, h, 1, im_compr);

  stbi_image_free(data);
  for(int i = 0;i < 8;i++){
  free(bl[i]);
  free(im_dct[i]);
  free(im_qtized[i]);
  free(im_rec[i]);
  }
  free(rlc);
  free(im_compr);

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


void move_dn(int *l, int *m, int k, int *t, int *inp[]){
  //printf("down began %d %d\n",i,j);
  int i=*l,j=*m;
  t[0] = inp[i++][j--];
	int cnt = 1;
	while(cnt <= k) t[cnt++] = inp[i++][j--];
  i--;
	j++;
  *l = i;
  *m = j;
}

void move_up(int *l, int *m, int k, int *t, int *inp[]){
  //printf("up began %d %d\n",i,j);
  int i=*l,j=*m;
  t[0] = inp[i--][j++];
	int cnt = 1;
	while(cnt <= k)	t[cnt++] = inp[i--][j++];
  i++;
	j--;
  *l = i;
  *m = j;
}

void rl_code(int *inp[],int *outp){
  int *st = (int *)malloc(64*sizeof(int));
  st[0] = inp[0][0];
	int k = 1, lp = 1,i = 0,j = 1;
	int *tmp = (int *)malloc(8*sizeof(int));

	for(;k <= 7;)
	{
		if(k%2){
			move_dn(&i,&j,k,tmp,inp);
			for(int l = 0;l <= k;l++) st[lp++] = tmp[l];
			if(k == 7){
				i = 7;
				j = 1;
				k = 6;
				break;
			}
			i++;
			j = 0;
			k++;
		}
		else{
			move_up(&i,&j,k,tmp,inp);
			for(int l = 0;l <= k;l++) st[lp++] = tmp[l];
			i = 0;
			j++;
			k++;
		}
	}
	k = 6;
	i = 7;
	j = 1;
	for(;k >= 1;)
	{
		if(k%2 == 0){
				move_up(&i,&j,k,tmp,inp);
				for(int l = 0;l <= k;l++) st[lp++] = tmp[l];
				//st[lp++] = inp[i-1][7];
				k--;
				j = 7;
				i++;
		}
		else{
			move_dn(&i,&j,k,tmp,inp);
			for(int l = 0;l <= k;l++) st[lp++] = tmp[l];
			if(k == 1){
				st[lp++] = inp[7][7];
				break;
			}
			i = 7;
			j++;
			k--;
    }
  }
  free(tmp);

  j = 0;
  lp  = 1;
  outp[0] = st[0];
  for(int i = 1; i < 64;i++)
  if(st[i]){
    //for(int k = 0;k < j;k++) outp[lp++] = 0;
    outp[lp++] = j;
    j = 0;
    outp[lp++] = st[i];
  }
  else j++;

  free(st);

}


void inflate_rlc(int *inp, int *outp[])
{
  int i,j,lp;
  int *st = (int *)malloc(64*sizeof(int));
  i = 1;
  lp = 1;
  int cl = 0;
  st[0] = inp[0];

  while(inp[i] != -255 && i < 129){
    if(!cl)
      for(j = 0,cl = 1,i++; j < inp[i-1];j++) st[lp++] = 0;
    else{
      st[lp++] = inp[i++];
      cl = 0;
    }
  }
  //printf("%d %d ",i,lp);
  for(;lp < 64;lp++) st[lp] = 0;

  for(int u1 = 0;u1 < 8;u1++)
  for(int v1 = 0;v1 < 8;v1++)
    outp[u1][v1] = 0;
  outp[0][0] = inp[0];
  int k = 1;
  i = 0;
  j = 1;
  lp = 1;
  for(;k <= 7;)
	{
		if(k%2){
			//move_dn(&i,&j,k,tmp,inp);
      outp[i++][j--] = st[lp++];
    	int cnt = 1;
      //printf("%d %d\n",i,j);
    	for(;cnt <= k;cnt++) outp[i++][j--] = st[lp++];
  		if(k == 7){
				i = 7;
				j = 1;
				k = 6;
				break;
			}
			//i++;
			j = 0;
			k++;
		}
		else{
			//move_up(&i,&j,k,tmp,inp);
      outp[i--][j++] = st[lp++];
    	int cnt = 1;
    	for(;cnt <= k;cnt++)	outp[i--][j++] = st[lp++];
			//for(int l = 0;l <= k;l++) st[lp++] = tmp[l];
			i = 0;
			//j++;
			k++;
		}
	}

	k = 6;
	i = 7;
	j = 1;
	for(;k >= 1;)
	{
		if(k%2 == 0){
				//move_up(&i,&j,k,tmp,inp);
        outp[i--][j++] = st[lp++];
      	int cnt = 1;
      	for(;cnt <= k;cnt++)	outp[i--][j++] = st[lp++];
  			k--;
				j = 7;
				i += 2;
		}
		else{
			//move_dn(&i,&j,k,tmp,inp);
      outp[i++][j--] = st[lp++];
    	int cnt = 1;
    	for(;cnt <= k;cnt++) outp[i++][j--] = st[lp++];
			//for(int l = 0;l <= k;l++) st[lp++] = tmp[l];
			if(k == 1){
				outp[7][7] = st[lp++];
				break;
			}
			i = 7;
			j += 2;
			k--;
    }
  }
  free(st);
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
    }
    //printf("\n");
  }
}
