// ECE 6310 Introduction to CV 
// Lab 3 Fall 22
// Harshal Varpe
// Clemson University
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

int main (int argc, char *argv[]){
    FILE *fpt, *temp_file, *g_truth, *out;
    int r,c,gt_row,gt_col,detected,not_detected,T,TP,FP,FN,TN,i,j,k;
    int min,max,r1,c1,ROWS,COLS,BYTES,temp_R, temp_C,temp_B;
    char header[320],gt_letter[320],letter;
    unsigned char *input,*template_img,*img_msf_norm,*final, *binary, *copy, *thin_img;
    float TPR,FPR,mean; int *temp_img;
    double *img_msf,sum;
    // New variable decalaration
    int del_counter;
    unsigned char neighbor[9] = {0};
    int e2ne,flag1,flag2,flag3,num_neighbors,endpt,branchpt;

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

img_msf_norm = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));


fpt = fopen("normalized_msf.ppm","rb");
if(fpt == NULL){printf("File %s could not be opened!\n","normalized_msf.ppm");exit(0);}
fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
fread(img_msf_norm,1,ROWS*COLS,fpt);
fclose(fpt);

// Open the ground truth file
g_truth = fopen("parenthood_gt.txt","rb");
if (g_truth == NULL){
    printf("The ground truth file could not be opened. Check if the name is parenthood_gt.txt and location is correct.");
    exit(0);
}
// /* creating a text file to write the threshold and ROC curve data*/
out  = fopen("confusion_mat.txt","a"); // make sure to delete the previous data from text file.
fprintf(out,"T TP FP FN TN TPR FPR \n");
// printf("%c \n",letter);


for(T=0;T<=255;T++){
    printf("%d\n",T);
    // for point 4.a
    binary = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    TP=TN=FP=FN=0;
    while(fscanf(g_truth,"%s %d %d",gt_letter,&gt_col,&gt_row) != EOF){
        // k = fscanf(g_truth,"%s %d %d",gt_letter,&gt_col,&gt_row);
        // printf("%d",k);
        detected = 0;
        not_detected = 0;
        // if(k!=3){break;}
        // printf(" detected %d \n",detected);
        for(r=gt_row-7;r<=gt_row+7;r++){
            for(c=gt_col-4;c<=gt_col+4;c++){
                if(img_msf_norm[r*COLS+c]>=T){detected = 1;}
                else{not_detected = 1;}
                }
            }
        // printf(" detected %d \n",detected); // delete this ; eveything works up until this point if you comment everything below.
        
        if (detected == 1){
            // printf(" inside detected loop \n");
            int l = 0;
            // printf("%d %d \n",gt_col,gt_row);
            // getchar();
            copy = (unsigned char *)calloc(temp_R*temp_C,sizeof(unsigned char));
            for(r=gt_row-7; r<=gt_row+7; r++ ){
                for(c=gt_col-4 ;c<=gt_col+4; c++){
                    // printf("%d\n",l);
                    copy[l] = input[r*COLS+c]; // check indexing here !! copy index may be wrong
                    l++;
                }
            }
            // 2.iv thresholding at 128 
            for (i=0;i<(temp_R*temp_C);i++){
                if(copy[i] > 128){
                    copy[i] = 0;}
                else{ 
                    copy[i] = 255;
                }
            }
            // if(*gt_letter == letter){
            //     fpt = fopen("thresholded.ppm","wb");
            //     fprintf(fpt,"P5 %d %d 255\n",temp_C,temp_R);
            //     fwrite(copy,1,temp_R*temp_C,fpt);
            //     fclose(fpt);
            //     getchar();
            // } //  writing letter
            
            
            del_counter = 1;

            while(del_counter > 0){
                del_counter = 0;
                thin_img = (unsigned char *)calloc(temp_R*temp_C,sizeof(unsigned char));
                for(i=0; i<(temp_R*temp_C) ;i++){
                    thin_img[i] = 0;
                }
                for(r=0;r<15;r++){
                    for(c=0;c<9;c++){
                        num_neighbors = 0;
                        e2ne = 0;
                        flag1 = 0;
                        flag2 = 0;
                        flag3 = 0;
                        if(copy[r*temp_C+c] == 255)
                        { // neighbor check
                            if(r==0 || c == 0){neighbor[0]=0;}
                            else{neighbor[0] = copy[(r-1)*temp_C+(c-1)];}

                            if(r==0){neighbor[1]=0;}
                            else{neighbor[1] = copy[(r-1)*temp_C+(c)];}

                            if(r==0 || c == 8){neighbor[2]=0;}
                            else{neighbor[2] = copy[(r-1)*temp_C+(c+1)];}
                            
                            if(c == 8){neighbor[3]=0;}
                            else{neighbor[3] = copy[(r)*temp_C+(c+1)];}

                            if(r== 14 || c == 8){neighbor[4]=0;}
                            else{neighbor[4] = copy[(r+1)*temp_C+(c+1)];}

                            if(r==14){neighbor[5]=0;}
                            else{neighbor[5] = copy[(r+1)*temp_C+(c)];}

                            if(r==14 || c == 0){neighbor[6]=0;}
                            else{neighbor[6] = copy[(r+1)*temp_C+(c-1)];}

                            if(c == 0){neighbor[7]=0;}
                            else{neighbor[7] = copy[(r)*temp_C+(c-1)];}
                        
                            // edge to non-edge transition condition
                            for(i=0;i<7;i++){if(neighbor[i]==255 && neighbor[i+1]==0){e2ne++;}}
                            if(neighbor[7]==255 && neighbor[0]==0){e2ne++;}
                            // printf("%d\n",e2ne);
                            if(e2ne==1){flag1 = 1;}
            
                            // number of neighbor condition
                            for(i=0;i<8;i++){if(neighbor[i] == 255){num_neighbors++;}}
                            if(num_neighbors >= 2 && num_neighbors <=6){flag2 = 1;}
                            // printf("%d\n",num_neighbors);
                            // north or east or (west AND south) is not edge
                            if(neighbor[1]==0 || neighbor[3]==0 || (neighbor[7] == 0 && neighbor[5] == 0)){
                                flag3 = 1;
                            }
                            // printf("%d\n",flag3);
                            if(flag1 == 1 && flag2 == 1 && flag3 == 1){
                                del_counter = del_counter - 1;
                                thin_img[r*temp_C+c] = 255;
                                // printf("counter reset");
                                }
                        } // if 
                    } // for
                }//for loop
                //  delete marked cells
                for(r=0;r<15;r++){
                    for(c=0;c<9;c++){
                        if(thin_img[r*temp_C+c] == 255){copy[r*temp_C+c]=0;}
                        }
                    }
                //  if(*gt_letter == letter){
                //     fpt = fopen("thresholded.ppm","wb");
                //     fprintf(fpt,"P5 %d %d 255\n",temp_C,temp_R);
                //     fwrite(copy,1,temp_R*temp_C,fpt);
                //     fclose(fpt);
                //     getchar();
                //     } //  writing letter
            } // second while loop
            endpt =0;
            branchpt = 0;
            for(r=0;r<15;r++){
                    for(c=0;c<9;c++){
                        if(copy[r*temp_C+c] == 255)
                        { // neighbor check
                            e2ne = 0;
                            // printf("Checking B and E\n");
                            if(r==0 || c == 0){neighbor[0]=0;}
                            else{neighbor[0] = copy[(r-1)*temp_C+(c-1)];}

                            if(r==0){neighbor[1]=0;}
                            else{neighbor[1] = copy[(r-1)*temp_C+(c)];}

                            if(r==0 || c == 8){neighbor[2]=0;}
                            else{neighbor[2] = copy[(r-1)*temp_C+(c+1)];}
                            
                            if(c == 8){neighbor[3]=0;}
                            else{neighbor[3] = copy[(r)*temp_C+(c+1)];}

                            if(r== 14 || c == 8){neighbor[4]=0;}
                            else{neighbor[4] = copy[(r+1)*temp_C+(c+1)];}

                            if(r==14){neighbor[5]=0;}
                            else{neighbor[5] = copy[(r+1)*temp_C+(c)];}

                            if(r==14 || c == 0){neighbor[6]=0;}
                            else{neighbor[6] = copy[(r+1)*temp_C+(c-1)];}

                            if(c == 0){neighbor[7]=0;}
                            else{neighbor[7] = copy[(r)*temp_C+(c-1)];}
                        
                            // edge to non-edge transition condition
                            
                            for(i=0;i<7;i++){if(neighbor[i]==255 && neighbor[i+1]==0){e2ne++;}}
                            if(neighbor[7]==255 && neighbor[0]==0){e2ne++;}
                            if(e2ne==1){endpt++;}
                            else if(e2ne>2){branchpt++;}              
                        }                                               
                    }
                 }
            if(branchpt == 1 && endpt == 1){detected = 1;}
            else{not_detected = 0;}
            // detected if loop
        }

        if ((detected == 1) && (*gt_letter == letter)){TP += 1;}
        else if ((detected == 1) && (*gt_letter != letter)){FP += 1;}
        else if ((not_detected == 1) && (*gt_letter == letter)){FN += 1;}
        else if ((not_detected == 1) && (*gt_letter != letter)){ TN +=1 ;}
           
    }
    TPR = (float)TP / (float)(TP+FN);
    FPR = (float)FP / (float)(FP+TN);
    fprintf(out,"%d %d %d %d %d %f %f\n",T,TP,FP,FN,TN,TPR,FPR);
    rewind(g_truth);
    // fclose(g_truth);
 }
fclose(out);
fclose(g_truth);
printf("This is the end my friend!");
}



