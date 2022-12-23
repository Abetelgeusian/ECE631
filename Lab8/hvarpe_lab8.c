#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define SQR(x) ((x)*(x))
#define MAX_QUEUE 10000


void RegionGrow (unsigned char *image,	/* image data */
		unsigned char *labels,	/* segmentation labels */
		int ROWS,int COLS,	/* size of image */
		int r,int c,		/* pixel to paint from */
		int paint_over_label,	/* image label to paint over */
		int new_label,		/* image label for painting */
		int *indices,		/* output:  indices of pixels painted */
		int *count,     
    double *normal_x,
    double *normal_y, 
    double *normal_z,
    double avg_norm[3][10])		
{
int	r2,c2;
int	queue[MAX_QUEUE],qh,qt;
double avg_x, avg_y, avg_z, angle, dot_p, L1, L2;

if (indices != NULL)
  indices[0]=r*COLS+c;
queue[0]=r*COLS+c;
qh=1;	/* queue head */
qt=0;	/* queue tail */
(*count) = 0;
avg_x = normal_x[(queue[qt]/COLS)*COLS+queue[qt]%COLS]; 
avg_y = normal_y[(queue[qt]/COLS)*COLS+queue[qt]%COLS]; 
avg_z = normal_z[(queue[qt]/COLS)*COLS+queue[qt]%COLS];

while (qt != qh)
  {
    for (r2=-2; r2<=2; r2++)
    {
        for (c2=-2; c2<=2; c2++)
        { 
 
        if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
            continue;
        if(normal_x[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=0 )
        {
        
            avg_x = (avg_x + normal_x[(r+r2)*COLS+(c+c2)])*0.5;
            avg_y = (avg_y + normal_y[(r+r2)*COLS+(c+c2)])*0.5;
            avg_z = (avg_z + normal_z[(r+r2)*COLS+(c+c2)])*0.5;

            dot_p = (avg_x*normal_x[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            (avg_y*normal_y[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            (avg_z*normal_z[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]);
            

            L1 = sqrt(SQR(avg_x) + SQR(avg_y) + SQR(avg_z));
            L2 = sqrt(SQR(normal_x[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            SQR(normal_y[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            SQR(normal_z[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]));

            angle = acos(dot_p / (L1*L2));
            // printf("The angle value is %lf \n",angle); /*the threshold will be the lower limit of these values.*/
            
            if (angle > 0.55){continue;}            

            labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;                
            if (indices != NULL){indices[*count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;}                

            (*count)++;
            queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
            qh=(qh+1)%MAX_QUEUE;
            if (qh == qt)
                {
                printf("Max queue size exceeded\n");
                exit(0);
                }
            }
        }
    }
    qt=(qt+1)%MAX_QUEUE;
    }
    avg_norm[0][new_label] = avg_x; avg_norm[1][new_label] = avg_y; avg_norm[2][new_label] = avg_z;  /* You will need this line to print out the surface normals.*/
}

int main()
{
int	r,c,r1,c1,x,y,t,ROWS,COLS,BYTES,T;
double	cp[7],*normal_x,*normal_y,*normal_z,avg,var;
double	xangle,yangle,dist;
double	ScanDirectionFlag,SlantCorrection;
unsigned char	*RangeImage,*grown_image,*thresh_image,*color_image;
char	header[320];
FILE	*fpt;
int RegionSize,*RegionPixels,TotalRegions,*indices;
double P[3][128*128];
/* variable for region growing*/
int count,r2,c2,queue[MAX_QUEUE],qh,qt,index;
int location[2];
double avg_norm[3][10];



/* Open Range Image */
if((fpt = fopen("chair-range.ppm","rb"))==NULL)
{printf("Cannot Open File to read. Please check it exists. \n");exit(0);}
fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
RangeImage = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
thresh_image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
normal_x = (double *)calloc(ROWS*COLS,sizeof(double));
normal_y = (double *)calloc(ROWS*COLS,sizeof(double));
normal_z = (double *)calloc(ROWS*COLS,sizeof(double));
header[0] = fgetc(fpt); /*deal with white space in header*/
if(RangeImage == NULL){printf("Cannot allocate memory!!!\n");exit(0);}
fread(RangeImage,1,COLS*ROWS,fpt);
fclose(fpt);

/* Image Thresholding and output */
T = 135;
for(r=0;r<ROWS;r++){
  for(c=0;c<COLS;c++){
    if(RangeImage[r*COLS+c]<=T){
      thresh_image[r*COLS+c] = 0;
    }
    else{
      thresh_image[r*COLS+c] = 255;
    }
  }
}

fpt=fopen("thresholded_Image.ppm","wb");
if (fpt == NULL)
{
printf("Unable to open threshold.ppm for writing\n");
exit(0);
}
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(thresh_image,1,ROWS*COLS,fpt);
fclose(fpt);
/*------------------- works til here ------------------------------- */

cp[0]=1220.7;		/* horizontal mirror angular velocity in rpm */
cp[1]=32.0;		/* scan time per single pixel in microseconds */
cp[2]=(COLS/2)-0.5;		/* middle value of columns */
cp[3]=1220.7/192.0;	/* vertical mirror angular velocity in rpm */
cp[4]=6.14;		/* scan time (with retrace) per line in milliseconds */
cp[5]=(ROWS/2)-0.5;		/* middle value of rows */
cp[6]=10.0;		/* standoff distance in range units (3.66cm per r.u.) */

cp[0]=cp[0]*3.1415927/30.0;	/* convert rpm to rad/sec */
cp[3]=cp[3]*3.1415927/30.0;	/* convert rpm to rad/sec */
cp[0]=2.0*cp[0];		/* beam ang. vel. is twice mirror ang. vel. */
cp[3]=2.0*cp[3];		/* beam ang. vel. is twice mirror ang. vel. */
cp[1]/=1000000.0;		/* units are microseconds : 10^-6 */
cp[4]/=1000.0;			/* units are milliseconds : 10^-3 */

for (r=0; r<ROWS; r++)
  {
  for (c=0; c<COLS; c++)
    {
    SlantCorrection=cp[3]*cp[1]*((double)c-cp[2]);
    xangle=cp[0]*cp[1]*((double)c-cp[2]);
    yangle=(cp[3]*cp[4]*(cp[5]-(double)r))+	/* Standard Transform Part */SlantCorrection*1;	/*  downward direction */
    dist=(double)RangeImage[r*COLS+c]+cp[6];
    P[2][r*COLS+c]=sqrt((dist*dist)/(1.0+(tan(xangle)*tan(xangle))
+(tan(yangle)*tan(yangle))));
    P[0][r*COLS+c]=tan(xangle)*P[2][r*COLS+c];
    P[1][r*COLS+c]=tan(yangle)*P[2][r*COLS+c];
    }
  }

/* Surface Normal calculations */
double x0,x1,x2,y0,y1,y2,z0,z1,z2,ax,ay,az,bx,by,bz;
int pix_dist;
pix_dist = 2;

for (r=0;r<ROWS-pix_dist;r++){
  for(c=0;c<COLS-pix_dist;c++){
    if(thresh_image[r*COLS+c] == 0){/*this conditionis necessary to make this work.*/
    x0 = P[0][r*COLS+c]; y0=P[1][r*COLS+c]; z0=P[2][r*COLS+c];
    x1 = P[0][(r+pix_dist)*COLS+c]; y1 = P[1][(r+pix_dist)*COLS+c]; z1 = P[2][(r+pix_dist)*COLS+c];
    x2 = P[0][r*COLS+(c+pix_dist)]; y2 = P[1][r*COLS+(c+pix_dist)]; z2 = P[2][r*COLS+(c+pix_dist)];
    ax = x1-x0; ay=y1-y0; az=z1-z0;
    bx = x2-x0; by=y2-y0; bz=z2-z0;
    normal_x[r*COLS+c] = (ay*bz) - (az*by);
    normal_y[r*COLS+c] = (az*bx) - (ax*bz);
    normal_z[r*COLS+c] = (ax*by) - (ay*bx);}
  }
}
/* write out surface normals */
fpt = fopen("surface_normal.csv","wb");
for(r=0;r<ROWS*COLS;r++){
  fprintf(fpt,"%lf,%lf,%lf,%lf,%lf,%lf\n",P[0][r],P[1][r],P[2][r],normal_x[r],normal_y[r],normal_z[r]);
}
fclose(fpt);
/*----------Region Growing----------*/
color_image = (unsigned char *)calloc(ROWS*COLS*3,sizeof(unsigned char));
grown_image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
indices = (int *)calloc(ROWS*COLS*3,sizeof(int));
TotalRegions = 0;
for(r=0;r<(ROWS-2);r++){
  for(c=0;c<(COLS-2);c++){
    if(thresh_image[r*COLS+c] == 0){
      count = 0;      
      for(r1=-2;r1<=2;r1++){
        for(c1=-2;c1<=2;c1++){
          if(grown_image[(r+r1)*COLS+(c+c1)] != 0){count++;}
        }
      }/*end nested for loop*/
      if(count == 0){
        TotalRegions++;
        if(TotalRegions == 255){printf("Ran out of labels. Segmentation is incomplete.");break;}
        // if(RegionSize < 40){
        //   for(x=0;x<RegionSize;x++){grown_image[indices[x]] = 0;}
        //   TotalRegions--;
        // }        
        RegionGrow(thresh_image,grown_image,ROWS,COLS,r,c,0,TotalRegions,indices,&RegionSize,normal_x,normal_y,normal_z,avg_norm);
        if(RegionSize < 40){
          for(x=0;x<RegionSize;x++){grown_image[indices[x]] = 0;}
          TotalRegions--;
        }
        else{
          printf("Labeled Region %d of size %d \n",TotalRegions,RegionSize);
          printf("with average surface normals of  %lf %lf %lf)\n",avg_norm[0][TotalRegions],avg_norm[1][TotalRegions],avg_norm[2][TotalRegions]);
        }
        // if(RegionSize >= 40){
        //   printf("Labeled Region %d of size %d \n",TotalRegions,RegionSize);
        //   printf("at %d %d with average surface normals of  %lf %lf %lf)\n",location[0],location[1],avg_norm[0][TotalRegions],avg_norm[1][TotalRegions],avg_norm[2][TotalRegions]);
        // }
      }
    }
  }
}
printf("Total Regions %d \n", TotalRegions);

for (x=0;x<ROWS*COLS;x++){
  color_image[x*3+0] = (grown_image[x]*53)%255;
  color_image[x*3+1] = (grown_image[x]*97)%255;
  color_image[x*3+2] = (grown_image[x]*223)%255;
}
fpt = fopen("seg_color.ppm","wb");
fprintf(fpt,"P6 %d %d 255\n",COLS,ROWS);
fwrite(color_image,1,ROWS*COLS*3,fpt);
fclose(fpt);
}
