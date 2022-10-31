// ECE 6310 Introduction to CV 
// Lab 2 Fall 22
// Harshal Varpe
// Clemson University
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

int main (int argc, char *argv[]){
    FILE *fpt, *temp_file, *g_truth, *out;
    int r,c,gt_row,gt_col,detected,not_detected,T,TP,FP,FN,TN,i,j;
    int min,max,sum,r1,c1,ROWS,COLS,BYTES,temp_R, temp_C,temp_B;
    char header[320],gt_letter[320],letter;
    unsigned char *input,*template_img,*img_msf_norm,*final, *binary;
    float TPR,FPR,mean; int *temp_img;
    double *img_msf;

if(argc != 2){printf("Wrong number or arguments! \n Usage : [executable_name] [letter]");
exit(0);} 
letter = argv[1][0];

/* Task 1 - Create a Zero mean centered template from the template image */
fpt = fopen("parenthood.ppm","rb");
if (fpt == NULL)
  {
  printf("Unable to open %s for reading\n","parenthood.ppm");
  exit(0);
  }
fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
input = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
fread(input,1,ROWS*COLS,fpt);
fclose(fpt);

fpt = fopen("parenthood_e_template.ppm","rb");
if (fpt == NULL)
  {
  printf("Unable to open %s for reading\n","parenthood_e_template.ppm");
  exit(0);
  }
fscanf(fpt,"%s %d %d %d",header,&temp_C,&temp_R,&temp_B);
template_img = (unsigned char *)calloc(temp_R*temp_C,sizeof(unsigned char));
fread(template_img,1,temp_R*temp_C,fpt);
fclose(fpt);
// printf("%d this is temp_r \n",temp_R);
// printf("%d \n",template_img[12]);
/* Calculating a ZMC template */
mean = 0;
for(j=0;j<(temp_R*temp_C);j++){
    mean += (float)template_img[j];
}
mean = mean / (float)(temp_R * temp_C);
// printf("%d",(int)mean);
/* We will store the zmc template.*/
temp_img = (int *)calloc(temp_R*temp_C,sizeof(int));
// temp_img = (double *)calloc(temp_R*temp_C,sizeof(double));
// temp_img = (float *)calloc(temp_R*temp_C,sizeof(float));
for(j=0;j<(temp_R*temp_C);j++){
    temp_img[j] = (int)((float)template_img[j] - mean);
} 
// for(int l=0;l<ROWS*COLS;l++){printf("%d \n",temp_img[l]);}
// printf("%f is mean \n",mean);
// /* Lets us create an MSF version of original image. */
img_msf = (double *)calloc(ROWS*COLS,sizeof(double));
for(r = (temp_R/2);r<=(ROWS-(temp_R/2));r++){
    for(c = (temp_C/2);c<=(COLS-(temp_C/2));c++){
        sum=0;
        for(r1 = -(temp_R/2);r1<=(temp_R/2);r1++){
            for(c1 = -(temp_C/2);c1<=(temp_C/2);c1++){
                sum += (temp_img[(r1+(temp_R/2))*temp_C+(c1+(temp_C/2))] * input[(r+r1)*COLS+(c+c1)]);
            }
        }
        img_msf[r*COLS+c] = sum;
    }
}

/* normalization of the msf image */
min =  5000; max = 0;
for(j=0;j<ROWS*COLS;j++){
    if(img_msf[j]>max){max = img_msf[j];}
    else if(img_msf[j]<min){min = img_msf[j];}
}
// printf("%d is min \n",min);
// printf("%d is max \n",max);

img_msf_norm = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
for(j=0;j<(ROWS*COLS);j++){
    img_msf_norm[j] = ((img_msf[j]-min)*255)/(max-min);
}

fpt = fopen("normalized_msf.ppm","w");
fprintf(fpt,"%s %d %d 255",header,COLS,ROWS);
fwrite(img_msf_norm,1,ROWS*COLS,fpt);
fclose(fpt);


// /* creating a text file to write the threshold and ROC curve data*/
out  = fopen("confusion_mat.txt","a");
fprintf(out,"T TP FP FN TN TPR FPR \n");
// printf("%c \n",letter);
for(T=0;T<=255;T++){
    // for point 4.a
    binary = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    for(j=0;j<(ROWS*COLS);j++){
        if (img_msf_norm[j]>=T){binary[j]=255;}
        else{binary[j]=0;}
    }

    // Although in the
    g_truth = fopen("parenthood_gt.txt","rb");
    TP=TN=FP=FN=0;    
    while(1){
        
        i = fscanf(g_truth,"%s %d %d",gt_letter,&gt_col,&gt_row);
        detected = not_detected = 0;
        if(i!=3){break;}    
        else { 
            for(r=gt_row-7;r<=gt_row+7;r++){
                for(c=gt_col-4;c<=gt_col+4;c++){
                    if(img_msf_norm[r*COLS+c]>=T){detected = 1;}
                    else{not_detected = 1;}
                }
            }
            if ((detected == 1) && (*gt_letter == letter)){TP += 1;}
            else if ((detected == 1) && (*gt_letter != letter)){FP += 1;}
            else if ((not_detected == 1) && (*gt_letter == letter)){FN += 1;}
            else if ((not_detected == 1) && (*gt_letter != letter)){ TN +=1 ;}
        }
    }
    // printf("%c is gt \n", gt_letter);
    // printf("%c is input \n",letter);
    TPR = (float)TP / (float)(TP+FN);
    FPR = (float)FP / (float)(FP+TN);
    fprintf(out,"%d %d %d %d %d %f %f\n",T,TP,FP,FN,TN,TPR,FPR);
    fclose(g_truth);
    // printf("%d",T);
}
fclose(out);
printf("This is the end my friend!");
/* binary image with chosen threshold */
// The variable final is used to store a binary image
T = 206; // Knee point of ROC
final = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
for(j=0;j<(ROWS*COLS);j++){
        if (img_msf_norm[j]>=T){final[j]=255;}
        else{final[j]=0;}
    }
fpt = fopen("final_output.ppm","w");
fprintf(fpt,"P5 %d %d 255",COLS,ROWS);
fwrite(final,1,ROWS*COLS,fpt);
fclose(fpt);
}