/*
ECE 6310
Motion tracking
Harshal Varpe
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define SQR(x) ((x)*(x))
#define thresh_acc 0.0001
#define thresh_gy 0.001
#define sample_t 0.05
#define g 9.81

int main(int argc, char *argv[]){
    FILE *fpt,*fpt1;
    int data_pts, filt_win ,x,y;
    char head1[320],head2[320],head3[320],head4[320],head5[320],head6[320],head7[320];
    double t[1300],acc_x[1300],acc_y[1300],acc_z[1300],roll[1300],pitch[1300],yaw[1300]; // for no header data
    // long double t[640],acc_x[640],acc_y[640],acc_z[640],roll[640],pitch[640],yaw[640]; // unusable
    double f_acc_x[1300],f_acc_y[1300],f_acc_z[1300],f_roll[1300],f_pitch[1300],f_yaw[1300];
    double sum_x, sum_y, sum_z, sum_p, sum_r, sum_yaw, avx, avy, avz, avp, avr, avyaw;
    double temp_x, temp_y, temp_z, temp_r, temp_p, temp_yaw;
    double var_accx[1300],var_accy[1300],var_accz[1300],var_pitch[1300],var_roll[1300],var_yaw[1300];

    double start_t, stop_t,start_rest,stop_rest;
    double int_acc[3],in_vel[3],vel[3];
    double int_pitch, int_roll, int_yaw ;
    int start_idx, stop_idx, start_rest_idx, stop_rest_idx, move;


    fpt = fopen("acc_gyro.txt","r");
    if(fpt == NULL){
        printf("Unable to open IMU readings!\n");
        exit(0);
    }

    fscanf(fpt,"%s %s %s %s %s %s %s",head1,head2,head3,head4,head5,head6,head7);
    
    data_pts = 0;    // calculating the number of data points
    while(fscanf(fpt,"%lf %lf %lf %lf %lf %lf %lf ",&t[data_pts],&acc_x[data_pts],&acc_y[data_pts],&acc_z[data_pts],&pitch[data_pts],&roll[data_pts],&yaw[data_pts]) != EOF){
        data_pts++;
    }
    fclose(fpt);
   
// variance calculation
int var_win = 15;

// Just like mean calculation above we will deal with edge cases first and then move on to
// variance calculation.
for(x=0;x<var_win/2;x++){
    var_accx[x] = 0;
    var_accy[x] = 0;
    var_accz[x] = 0;
    var_pitch[x] = 0;
    var_roll[x] = 0;
    var_yaw[x] = 0;
}
for(x=data_pts-(var_win/2);x<data_pts;x++){
    var_accx[x] = 0;
    var_accy[x] = 0;
    var_accz[x] = 0;
    var_pitch[x] = 0;
    var_roll[x] = 0;
    var_yaw[x] = 0;
}       
for(x=var_win/2;x<(data_pts - (var_win/2));x++){
        sum_x = 0; sum_y =0; sum_z=0;
        sum_p = 0; sum_r = 0; sum_yaw=0;
        for(y=(-var_win/2);y<=(var_win/2);y++){
            sum_x = sum_x + acc_x[x+y] ;
            sum_y = sum_y + acc_y[x+y] ;
            sum_z = sum_z + acc_z[x+y] ;
            sum_p = sum_p + pitch[x+y] ;
            sum_r = sum_r + roll[x+y] ;
            sum_yaw = sum_yaw + yaw[x+y] ;
        }
        avx = sum_x / var_win;
        avy = sum_y / var_win;
        avz = sum_z / var_win;
        avp = sum_p / var_win;
        avr = sum_r / var_win;
        avyaw = sum_yaw / var_win;
        temp_x = temp_y = temp_z = temp_r = temp_p = temp_yaw = 0;
        for(y=(-var_win/2);y<=(var_win/2);y++){
            temp_x = temp_x + SQR(acc_x[x+y] - avx) ;
            temp_y = temp_y + SQR(acc_y[x+y] - avy) ;
            temp_z = temp_z + SQR(acc_z[x+y] - avz) ;
            temp_p = temp_p + SQR(pitch[x+y] - avp) ;
            temp_r = temp_r + SQR(roll[x+y] - avr) ;
            temp_yaw = temp_yaw + SQR(yaw[x+y] - avyaw) ;
        }
        var_accx[x] = temp_x / (var_win - 1);
        var_accy[x] = temp_y / (var_win - 1);
        var_accz[x] = temp_z / (var_win - 1);
        var_pitch[x] = temp_p / (var_win - 1);
        var_roll[x] = temp_r / (var_win - 1);
        var_yaw[x] = temp_yaw / (var_win - 1);
    }

    fpt = fopen("variance.csv","w");
    fprintf(fpt," %s, %s, %s, %s, %s, %s, %s \n","time","var_acx","var_acy","var_acz","pitch","var_roll","vra_yaw");
    for(x = 0; x < data_pts; x++){
        fprintf(fpt," %lf, %lf, %lf, %lf, %lf, %lf, %lf \n",t[x],var_accx[x],var_accy[x],var_accz[x],var_pitch[x],var_roll[x],var_yaw[x]);}
    fclose(fpt);
// printf("Good till here");


start_rest_idx = 1; // index 1 means not resting
move = 0;
start_t = stop_t = stop_rest = start_idx = stop_idx = start_rest = 0;
stop_rest_idx = 0;

int_acc[0] = int_acc[1] = int_acc[2] = 0;
int_pitch = int_roll = int_yaw = 0;
in_vel[0] = in_vel[1] = in_vel[2] = 0;
vel[0] = vel[1] = vel[2] = 0;
//Write out the data in txt file
fpt = fopen("output.csv","w");
fprintf(fpt,"%s  , %s  , %s   ,%s   ,%s   ,%s   ,%s   ,%s   ,%s   ,%s   ,%s  \n","state","start_idx","stop_idx","start_time","stop_time","dist_x(m)","dist_y(m)","dist_z(m)","pitch(rad)","roll(rad)","yaw(rad)");
//(work till here)
for(x=0;x<data_pts;x++){
    // if the variance is above threshold motion is detected
    if(var_accx[x]>thresh_acc || var_accy[x]>thresh_acc || var_accz[x]>thresh_acc || var_pitch[x]>thresh_gy || var_roll[x]>thresh_gy || var_yaw[x]>thresh_gy){
        move = 1;
        // printf("%d",x);
    }
    else{move = 0;
    //printf("%d",x);
    }
    // if move is 1 and start_idx is zero, then start_idx 
    if(move == 1 && start_idx == 0){start_idx = x; start_t = t[x];}
    
    //if it is moving, stop_rest_idx is zero, and start_rest-idex is not what initilised
    //(meaning we are in rest period) ,  then stop_rest_idx
    // if(move == 1 || start_rest_idx != 2 && stop_rest_idx == 0 )
    // if(move == 1 && stop_rest_idx == 0 || start_rest_idx != 2 && x == data_pts - 1)
    if((move == 1 && stop_rest_idx == 0 && start_rest_idx != 1) || x == data_pts - 1)
    {stop_rest_idx=x; stop_rest = t[x];} 
    
    // if it is not moving, and stop index is zero but start index is not zero, then stop index 
    if(move == 0 && stop_idx == 0 && start_idx != 0){stop_idx = x ; stop_t = t[x];}
    
    // if it is not moving and start_rest_index is initilised value, then start rest
    if(move == 0 && start_rest_idx == 1){start_rest_idx = x; start_rest = t[x];}

    if(start_idx != 0 && stop_idx != 0){
        
        for(y = start_idx; y<stop_idx; y++){
            int_pitch = int_pitch + pitch[y] * sample_t ;
            int_roll = int_roll + roll[y] * sample_t ;
            int_yaw = int_yaw + yaw[y] * sample_t ;

            in_vel[0] = vel[0]; in_vel[1] = vel[1]; in_vel[2] = vel[2];

            vel[0] = vel[0] + (acc_x[y] * g * sample_t);
            vel[1] = vel[1] + (acc_y[y] * g * sample_t);
            vel[2] = vel[2] + (acc_z[y] * g * sample_t);

            int_acc[0] = int_acc[0] + (((in_vel[0] + vel[0])/2)*sample_t); // dist x
            int_acc[1] = int_acc[1] + (((in_vel[1] + vel[1])/2)*sample_t); // dist y
            int_acc[2] = int_acc[2] + (((in_vel[2] + vel[2])/2)*sample_t); // dist z        

        }
        fprintf(fpt,"%s   ,%d   ,%d   ,%lf   ,%lf   ,%lf   ,%lf   ,%lf   ,%lf   ,%lf   ,%lf \n","move",start_idx,stop_idx,start_t,stop_t,int_acc[0],int_acc[1],int_acc[2],int_pitch,int_roll,int_yaw);
        start_idx = stop_idx = start_t = stop_t = 0;
        int_acc[0] = int_acc[1] = int_acc[2] = 0;
        // int_gy[0] = int_gy[1] = int_gy[2] = 0; // may not need this
        int_pitch = int_roll = int_yaw = 0;
        in_vel[0] = in_vel[1] = in_vel[2] = 0;
        vel[0] = vel[1] = vel[2] = 0;
    }
    if(start_rest_idx != 1 && stop_rest_idx != 0){
        
        fprintf(fpt,"%s   ,%d   ,%d   ,%lf   ,%lf  \n","rest",start_rest_idx,stop_rest_idx,start_rest,stop_rest);
        stop_rest_idx = stop_rest = start_rest = 0;
        start_rest_idx = 1;
    }
    move = 0;
}
fclose(fpt);

}