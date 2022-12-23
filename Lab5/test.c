#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))

int main(){
FILE  *fpt;
int		ROWS,COLS,BYTES;
char  header[320];
unsigned char *Input, *initialContour,*norm_sob;
int yPoints[60], xPoints[60], totalData,i,x,y,x1,y1;
float *sobel,max,min,sob_x,sob_y;

fpt = fopen("hawk.ppm","rb");
Input = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
fread(Input,1,COLS*ROWS,fpt);
fclose(fpt);

sobel = (float *)calloc(ROWS*COLS,sizeof(float));
norm_sob = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
int sob_fx[9]= {1,0,-1,2,0,-2,1,0,-1};
int sob_fy[9] = {1,2,1,0,0,0,-1,-2,-1};



printf("Good till here 1 \n");


for (x=1;x<ROWS-1;x++){
  for(y=1;y<COLS-1;y++){
    sob_x = 0.0;
    sob_y = 0.0;
    for(x1=-1; x1<=1; x1++){
      for(y1=-1; y1<=1; y1++){
        sob_x += Input[(x+x1)*COLS+(y+y1)] * sob_fx[(x1+1)*3+(y1+1)];
        sob_y += Input[(x+x1)*COLS+(y+y1)] * sob_fy[(x1+1)*3+(y1+1)];
      }
    }
    sobel[x*COLS+y] = sqrt( SQR(sob_x) + SQR(sob_y) );
  }
}
printf("Good till here 2 \n");

// Let is find the min and max intensities of the sobel filtered output
// max_prev = 0;
// min_prev = 0;
// max = 0;
// min = 1000;

max = sobel[0];
min = sobel[0];

for(i=0;i<ROWS*COLS;i++){
  if(max < sobel[i]){max = sobel[i];}
  if(min > sobel[i]){min = sobel[i];}
}

printf("max: %f\tmin: %f \n",max,min);
//Let us now normalize the min and max

// for (i = 0; i < ROWS*COLS; i++)
//   {
//     norm_sob[i] = (sobel[i] - min)*255/(max-min);
//   }
 

// // for(x=0;x<ROWS;x++){
// //   for(y=0;y<COLS;y++){
// //     norm_sob[x*COLS+y] = ((sobel[x*COLS+y] - min_prev)* ((max-min)/(max_prev - min_prev)))+ min ; 
// //   }
// // }

// fpt = fopen("sobel_test.ppm","w");
// if(fpt==NULL){printf("Unable to open sobel output");exit(0);}
// fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
// fwrite(norm_sob,COLS*ROWS,1,fpt);
// fclose(fpt);
}
