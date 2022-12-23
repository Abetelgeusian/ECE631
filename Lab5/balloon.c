
	/*
	** Coding challenge:  calculate balloon energy
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))

int main(int argc, char *argv[])

{
FILE	*fpt;
int	px[100],py[100];
int	i,total_points;
int	window,x,y,move_x,move_y,cx,cy;
int	energy[19*19];
double	distance;

if (argc != 3)
  {
  printf("Usage:  balloon [points.txt] [window]\n");
  exit(0);
  }
window=atof(argv[2]);
if (window < 3  ||  window > 19)
  {
  printf("3 <= window <= 19\n");
  exit (0);
  }

	/* read contour points file */
if ((fpt=fopen(argv[1],"r")) == NULL)
  {
  printf("Unable to open %s for reading\n",argv[1]);
  exit(0);
  }
total_points=0;
while (1)
  {
  i=fscanf(fpt,"%d %d",&px[total_points],&py[total_points]);
  if (i != 2)
    break;
  total_points++;
  if (total_points > 100)
    break;
  }
fclose(fpt);

	/* calculate balloon energy around point 1 */
cx=cy=0;
for (i=0; i<total_points; i++)
  {
  cx+=px[i];
  cy+=py[i];
  }
cx/=total_points;
cy/=total_points;
printf("centroid at %d,%d\n",cx,cy);
int control_points =25;
for (y=0; y<window; y++)
  {
  for (x=0; x<window; x++)
    {
    move_x=px[control_points]-window/2+x;
    move_y=py[control_points]-window/2+y;
    distance=sqrt((double)SQR(cx-move_x)+(double)SQR(cy-move_y));
    energy[y*window+x]=0-(int)distance;
    }
  }

for (y=0; y<window; y++)
  {
  for (x=0; x<window; x++)
    printf("%2d ",energy[y*window+x]);
  printf("\n");
  }


}

