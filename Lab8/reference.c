#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define SQR(x) ((x)*(x))
#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */

int main()
{
    FILE	*fpt;
    unsigned char	*range_image,*labels, *threshold_image, *color_image;
    char header[320];
    int COLS, ROWS, BYTES, r, c, T, vec_dist;
    double	x_angle,y_angle,dist;
    double	SlantCorrection, ScanDirectionFlag,avg,var;
    int		RegionSize,*RegionPixels,TotalRegions,*indices;
    int r2,c2,i;
    double	cp[7], *Normal_X, *Normal_Y, *Normal_Z;
    void RegionGrow();

    /* read range image */
    if ((fpt=fopen("chair-range.ppm","rb")) == NULL)
    {
    printf("Unable to open chair-range.ppm for reading\n");
    exit(0);
    }
    fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
    range_image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    header[0]=fgetc(fpt);	/* read white-space character that separates header */
    if (range_image == NULL)
    {
    printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
    exit(0);
    }
    fread(range_image,1,COLS*ROWS,fpt);
    fclose(fpt);
    

    /* Creating Threshold image */
    T = 135;
    threshold_image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

    for (r=0; r<ROWS; r++) {
        for (c=0; c<COLS; c++) {
            if (range_image[r*COLS+c]<=T) {
                threshold_image[r*COLS+c] = 0;
            }
            else {
                threshold_image[r*COLS+c] = 255;
            }
        }
    }

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


    double P[3][ROWS*COLS];
    for (r=0; r<ROWS; r++)
    {
        for (c=0; c<COLS; c++)
        {
            SlantCorrection=cp[3]*cp[1]*((double)c-cp[2]);
            x_angle=cp[0]*cp[1]*((double)c-cp[2]);
            y_angle=(cp[3]*cp[4]*(cp[5]-(double)r))+	/* Standard Transform Part */
        SlantCorrection*1;	/*  + slant correction */
            dist=(double)range_image[r*COLS+c]+cp[6];
            P[2][r*COLS+c]=sqrt((dist*dist)/(1.0+(tan(x_angle)*tan(x_angle))
        +(tan(y_angle)*tan(y_angle))));
            P[0][r*COLS+c]=tan(x_angle)*P[2][r*COLS+c];
            P[1][r*COLS+c]=tan(y_angle)*P[2][r*COLS+c];
        }
    }
    
    double x0,y0,z0,x1,y1,z1,x2,y2,z2;
    double ax,ay,az,bx,by,bz;
    double *dot_product_col = (double *)calloc(ROWS*COLS,sizeof(double ));
    double *dot_product_row = (double *)calloc(ROWS*COLS,sizeof(double ));
    double *theta_col = (double *)calloc(ROWS*COLS,sizeof(double ));
    double *theta_row = (double *)calloc(ROWS*COLS,sizeof(double ));
    double amod,bmod;
    Normal_X = (double *)calloc(ROWS*COLS, sizeof(double));
    Normal_Y = (double *)calloc(ROWS*COLS, sizeof(double));
    Normal_Z = (double *)calloc(ROWS*COLS, sizeof(double));
    vec_dist = 1;

    for (r=0; r<ROWS; r++) {
        for (c=0; c<COLS; c++) {
            if (threshold_image[r*COLS+c]==0 && (r+vec_dist < ROWS || c+vec_dist < COLS)) 
            {
                x0 = P[0][r*COLS+c]; y0 = P[1][r*COLS+c]; z0 = P[2][r*COLS+c];
                x1 = P[0][(r+vec_dist)*COLS+(c)]; y1 = P[1][(r+vec_dist)*COLS+(c)]; z1 = P[2][(r+vec_dist)*COLS+(c)];
                x2 = P[0][(r)*COLS+(c+vec_dist)]; y2 = P[1][(r)*COLS+(c+vec_dist)]; z2 = P[2][(r)*COLS+(c+vec_dist)];
                ax = x1-x0; ay = y1-y0; az = z1-z0;
                bx = x2-x0; by = y2-y0; bz = z2-z0;
                Normal_X[r*COLS+c] = (ay*bz) - (az*by);
                Normal_Y[r*COLS+c] = (az*bx) - (ax*bz);
                Normal_Z[r*COLS+c] = (ax*by) - (ay*bx);
            }
        }
    }

    printf("Creating Outputs.csv file\n");
    fpt=fopen("Outputs.csv","wb");
   
    for (r=0;r<ROWS*COLS;r++) {
        fprintf(fpt,"%lf,%lf,%lf,%lf,%lf,%lf\n",P[0][r],P[1][r],P[2][r],Normal_X[r],Normal_Y[r],Normal_Z[r]);
    }
    fclose(fpt);
    printf("Outputs.csv created\n");

    fpt=fopen("threshold.ppm","wb");
    if (fpt == NULL)
    {
    printf("Unable to open threshold.ppm for writing\n");
    exit(0);
    }
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(threshold_image,1,ROWS*COLS,fpt);
    fclose(fpt);


    color_image=(unsigned char *)calloc(ROWS*COLS*3,sizeof(unsigned char)); //color_image color image
    labels = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    indices = (int *)calloc(ROWS*COLS*3, sizeof(int));
    TotalRegions = 0;
    int region_loc[2];
    double Average_normal[3][10];
    for (r = 2; r < (ROWS-2); r++)
    {
        for (c = 2; c < (COLS-2); c++)
        {
            if (labels[r*COLS+c] != 0)
            {
		        continue;
		    }
            if (threshold_image[r*COLS+c] == 0)
            {
                // Seed condition is no pixel in 5x5 region should be labeled
                int counter = 0;
                for (int r1 = -2; r1 <= 2; r1++)
                {
                for (int c1 = -2; c1 <= 2; c1++)
                {
                    if (labels[(r+r1)*COLS+(c+c1)] != 0) {counter++;}
                }
                }
                // No pixels are labeled
                if (counter == 0)
                {
                TotalRegions++;
                if (TotalRegions == 255) {printf("Segmentation incomplete.  Ran out of labels.\n"); break;}
                RegionGrow(threshold_image,labels,ROWS,COLS,r,c,0,TotalRegions,indices,&RegionSize,Normal_X,Normal_Y,Normal_Z,Average_normal,region_loc);
                if (RegionSize < 50)
                {
                for (i = 0; i < RegionSize; i++) {labels[indices[i]] = 0;}
                TotalRegions--;
                }
                else
                {
                printf("\n Region labeled %d is %d in size.\n",TotalRegions,RegionSize);
                printf("\n At %d %d, %lf %lf %lf \n",region_loc[1],region_loc[0],Average_normal[0][TotalRegions],Average_normal[1][TotalRegions],Average_normal[2][TotalRegions]);
                }
                }
            } // ALL REGION GROWING ENDS HERE
            else {continue;}
        }
    }
    printf("%d total regions were found\n", TotalRegions);


    for(i=0; i<ROWS*COLS;i++){
        color_image[i*3+0] = (labels[i]*53)%255;
        color_image[i*3+1] = (labels[i]*97)%255;
        color_image[i*3+2] = (labels[i]*223)%255;
    }
    if ((fpt=fopen("seg-color.ppm","wb")) == NULL)
    {
    printf("Unable to open file for writing\n");
    exit(0);
    }
    fprintf(fpt,"P6 %d %d 255\n",COLS,ROWS);
    fwrite(color_image,1,ROWS*COLS*3,fpt);

    fclose(fpt);
}

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
    double avg_norm[3][10],
    int location[2])		
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

           /* test criteria to join region */
        if(normal_x[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=0 )
        {
        
            avg_x = (avg_x + normal_x[(r+r2)*COLS+(c+c2)]) / 2.0;
            avg_y = (avg_y + normal_y[(r+r2)*COLS+(c+c2)]) / 2.0;
            avg_z = (avg_z + normal_z[(r+r2)*COLS+(c+c2)]) / 2.0;

            dot_p = (avg_x*normal_x[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            (avg_y*normal_y[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            (avg_z*normal_z[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]);

            L1 = sqrt(SQR(avg_x) + SQR(avg_y) + SQR(avg_z));

            L2 = sqrt(SQR(normal_x[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            SQR(normal_y[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
            SQR(normal_z[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]));

            angle = acos(dot_p / (L1*L2));
            
            if (angle >0.8){continue;}            

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
    location[0] = r;
    location[1] = c;
    avg_norm[0][new_label] = avg_x;
    avg_norm[1][new_label] = avg_y;
    avg_norm[2][new_label] = avg_z; 
}

#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */

// void RegionGrow (int *image,	/* image data */
// 		unsigned char *labels,	/* segmentation labels */
// 		int ROWS,int COLS,	/* size of image */
// 		int r,int c,		/* pixel to paint from */
// 		int paint_over_label,	/* image label to paint over */
// 		int new_label,		/* image label for painting */
// 		int *indices,		/* output:  indices of pixels painted */
// 		int *count,     
//     double *Normal_X,
//     double *Normal_Y, 
//     double *Normal_Z,
//     double Average_normal[3][10],
//     int region_loc[2])		
// {
// int	r2,c2;
// int	queue[MAX_QUEUE],qh,qt;

// double avgx, avgy, avgz, angle, dot, mag_a, mag_b;
// double ANGLE_THRESH =  0.174*4;
// if (indices != NULL)
//   indices[0]=r*COLS+c;
// queue[0]=r*COLS+c;
// qh=1;	/* queue head */
// qt=0;	/* queue tail */
// (*count) = 0;
// avgx = Normal_X[(queue[qt]/COLS)*COLS+queue[qt]%COLS]; 
// avgy = Normal_Y[(queue[qt]/COLS)*COLS+queue[qt]%COLS]; 
// avgz = Normal_Z[(queue[qt]/COLS)*COLS+queue[qt]%COLS];

// while (qt != qh)
//   {
//     for (r2=-1; r2<=1; r2++)
//     {
//         for (c2=-2; c2<=2; c2++)
//         { 
 
//         if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
//             continue;

//            /* test criteria to join region */
//         if(Normal_X[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=0 )
//         {
        
//             avgx = (avgx + Normal_X[(r+r2)*COLS+(c+c2)]) / 2.0;
//             avgy = (avgy + Normal_Y[(r+r2)*COLS+(c+c2)]) / 2.0;
//             avgz = (avgz + Normal_Z[(r+r2)*COLS+(c+c2)]) / 2.0;

//             dot = (avgx*Normal_X[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
//             (avgy*Normal_Y[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]) + 
//             (avgz*Normal_Z[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]);

//             mag_a = sqrt(pow(avgx, 2) + pow(avgy, 2) + pow(avgz, 2));

//             mag_b = sqrt(pow(Normal_X[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2], 2) + 
//             pow(Normal_Y[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2], 2) + 
//             pow(Normal_Z[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2], 2));

//             angle = acos(dot / (mag_a*mag_b));
            
//             if (angle >ANGLE_THRESH)
//             continue;

//             labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;                
//             if (indices != NULL)
//                 indices[*count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;

//             (*count)++;
//             queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
//             qh=(qh+1)%MAX_QUEUE;
//             if (qh == qt)
//                 {
//                 printf("Max queue size exceeded\n");
//                 exit(0);
//                 }
//             }
//         }
//     }
//     qt=(qt+1)%MAX_QUEUE;
//     }
//     region_loc[0] = r;
//     region_loc[1] = c;
//     Average_normal[0][new_label] = avgx;
//     Average_normal[1][new_label] = avgy;
//     Average_normal[2][new_label] = avgz; 
// }