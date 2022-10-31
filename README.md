# ECE631
This repo contains the lab work done for the ECE 6310 Intro to CV course taught by Adam Hoover at Clemson University. Each lab folder contains a brief report of what has been done in each of the labs.

In the Lab 1, students were asked to implement three different versions of 7x7 filter : 1) Standard 7x7 mean filter 2) Separable 7x7 filter 3) A combination of Separable and Sliding Filter. The C scripts conv2d, sepfilter, sep_slidingfilter are for standard 7x7 filter, separable filter and hybrid of separable and sliding window filter. Since each of the scripts were to be timed, Each script will append the text file with name - "timekeeping_<filter name>.txt".

The lab 2 dealt with the optical character recognition problem and corresponding ROC curve to evaluate the performance of the algorithm.

The lab 3 build upon the lab2. In this lab, I implemented the skeletonization and edge finding to reduce number of False Positives.

In the lab 4, I have created a GUI using WIN32 and C. With this GUI one can open an image and color different segments of the image with different color. Moreover, user can also choose how to color. In step mode, a pixel is colored at each step. In play mode, a region is colored based on two conditions - difference in color intensity and distance from the centroid.
