
	/*
	** This program reads bridge.ppm, a 512 x 512 PPM image.
	** It smooths it using a standard 7x7 mean filter.
	** The program also demonstrates how to time a piece of code.
	** To compile, must link using -lrt  (man clock_gettime() function).
  ** 
  ** The program will store the time taken in the text file named
  ** timekeeper_sepslide.txt. The image will be stored as smoothed_sepsliding
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main()

{
FILE		*fpt;
unsigned char	*image;
unsigned char	*smoothed;
float *temp; /*temp has to be a number, since we still need to process it.*/
float sum; /*Float will yield better results.*/
char		header[320];
int		ROWS,COLS,BYTES;
int		r,c,r1,c1;
struct timespec	tp1,tp2;

char *textfile = "timekeeper_sepslide.txt";

	/* read image */
if ((fpt=fopen("bridge.ppm","rb")) == NULL)
  {
  printf("Unable to open bridge.ppm for reading\n");
  exit(0);
  }
fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
if (strcmp(header,"P5") != 0  ||  BYTES != 255)
  {
  printf("Not a greyscale 8-bit PPM image\n");
  exit(0);
  }
image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
header[0]=fgetc(fpt);
/* read white-space character that separates header */
fread(image,sizeof(char),COLS*ROWS,fpt); 
fclose(fpt);

	/* allocate memory for smoothed version of image */
smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
temp = (float *)calloc(ROWS*COLS,sizeof(float));

	/* query timer */
FILE *tp = fopen(textfile,"a");
if (tp == NULL)
  {
    printf("Error");
    exit(0);
  }
clock_gettime(CLOCK_REALTIME,&tp1);
printf("Start time : %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);
fprintf(tp,"Start time : %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, skipping the border points */
  /*Seprable Sliding Filter Columns part.*/
for (r=0; r<ROWS; r++)
  for (c=3; c<COLS-3; c++)
    {
      if (c == 3)
      {
        sum=0;
        for (c1 = -3;c1<=3;c1++)
          sum += image[r*COLS+(c+c1)];
      }
      else
      {
        sum -= image[r*COLS+(c-4)];
        sum += image[r*COLS+(c+3)];
      }
      temp[r*COLS+c]=sum;
    }
/* Horizontal filter. Row part*/

for (c=0; c<COLS; c++)
  for (r=3; r<ROWS-3; r++)
    {
      if (r == 3)
      {
        sum=0;
        for (r1 = -3;r1<=3;r1++)
          sum += temp[(r+r1)*COLS+c];
      }
      else
      {
        sum -= temp[(r-4)*COLS+c];
        sum += temp[(r+3)*COLS+c];
      }
      smoothed[r*COLS+c]=sum/49;
    }  

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp2);
printf("End Time : %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);
fprintf(tp,"End Time : %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);
	/* report how long it took to smooth */
printf("Difference in nsec : %ld\n",tp2.tv_nsec-tp1.tv_nsec);
fprintf(tp,"Difference in nsec : %ld\n\n",tp2.tv_nsec-tp1.tv_nsec);
fclose(tp);
	/* write out smoothed image to see result */
fpt=fopen("smoothed_sepsliding.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed,COLS*ROWS,1,fpt);
fclose(fpt);
return(0);
}
