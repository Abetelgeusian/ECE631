
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine, int nCmdShow)

{
MSG			msg;
HWND		hWnd;
WNDCLASS	wc;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName="ID_MAIN_MENU";
wc.lpszClassName="PLUS";

if (!RegisterClass(&wc))
  return(FALSE);

hWnd=CreateWindow("PLUS","Final Project",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,700,700,NULL,NULL,hInstance,NULL);
if (!hWnd)
  return(FALSE);

ShowScrollBar(hWnd,SB_BOTH,FALSE);
ShowWindow(hWnd,nCmdShow);
UpdateWindow(hWnd);
MainWnd=hWnd;

ShowPixelCoords=0;
play_mode = 1;
step_mode = 0;
restore = 0;
red = 1;
green = 0;
yellow = 0;
blue = 0;
mode_trig = 1;

MessageBox(
	NULL,
	TEXT(" Hello There! \n\n Use File tab to load an image of PNM format. \n\n Press Q to restore original." 
			"\n\n Use Display tab to view Sobel Image of the loaded image."
			"\n\n Press and Hold Left Mouse Button to draw a contour around\n any object in an image.\n Let go of the Left button to start rubberband active contouring."
			"\n\n Click Right mouse button to start Ballon active contouring."
			"\n\n After either rubberbanding or ballooning is done, \nhold shift and Left mouse button to drag any point \nto desired location. This will start neutral contouring."),
		TEXT("User Guide"),
		MB_OK
	);

strcpy(filename,"");
OriginalImage=NULL;
ROWS=COLS=0;

InvalidateRect(hWnd,NULL,TRUE);
UpdateWindow(hWnd);

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}

void Rubberband()
{/*reused from lab4*/
	HDC hDC;
	char message[320];
	int l, m, n,j,k; // counters
	int c1, c2, c3; // for energy loops
	int iter = 30;
	int total_points = rubberct; 
	float distance,avgdist,max,min;
	float ext_E[window * window], int_E1[window * window], int_E2[window * window], total_E[window * window];
	int w1, w2, w3;
	//int *px, *py;
	//px = (int*)calloc(rubberct, sizeof(int));
	//py = (int*)calloc(rubberct, sizeof(int));
	//int px[1000];
	//int py[1000];
	//for (l = 0; l < rubberct; l++) {
	//	px[l] = point_x[l];
	//	py[l] = point_y[l];
	//}
	// printf("\n %d", window);

	for (l = 0; l < iter; l++) {
		distance = 0;
		for (m = 0; m < total_points; m++) {			
			c1 = 0;
			for (x = (-window / 2); x <= window / 2; x++) {
				for (y = (-window / 2); y <= window / 2; y++) {
					ext_E[c1] = (float)SQR(norm_sobel[(point_y[m] + x) * COLS + (point_x[m] + y)]);
					// printf("%f ", ext_E[c1]);
					c1++;
				}
			} //  external energy
			// printf("External energy is %f \n",ext_E);
			
			c2 = 0;
			for (x = (-window / 2); x <= window / 2; x++) {
				for (y = (-window / 2); y <= window / 2; y++) {
					if (m == total_points - 1) { int_E1[c2] = (float)SQR((point_y[m] + x) - (point_y[0])) + SQR((point_x[m] + y) - (point_x[0])); }
					else { int_E1[c2] = (float)SQR((point_y[m] + x) - (point_y[0])) + SQR((point_x[m] + y) - (point_x[0])); }
					// printf("%f ", int_E1[c2]);
					c2++;
				}
			} // internal energy 1
			distance = 0.0;

			for (n = 0; n < total_points; n++) {
				if (m == total_points - 1) 
				{ distance += sqrt(SQR(point_y[m] - point_y[0]) + SQR(point_x[m] - point_x[0])); }
				else 
				{ distance += sqrt(SQR(point_y[m] - point_y[m + 1]) + SQR(point_x[m] - point_x[m + 1])); }
			}
			avgdist = (float)(distance / total_points);
			// printf("\n%f\n",avgdist);

			c3 = 0;
			for (x = (-window / 2); x <= window / 2; x++) {
				for (y = (-window / 2); y <= window / 2; y++) {
					int temp = 0;
					if (m == total_points - 1) {
						temp = sqrt(SQR(point_y[m] + x - point_y[0]) + SQR(point_x[m] + y - point_x[0]));
						int_E2[c3] = (float)SQR(avgdist - temp);
						// printf("Here! \n");
						// printf("%d \n",temp);
						// printf("%d \n",avgdist);
					}
					else {
						temp = sqrt(SQR(point_y[m] + x - point_y[m + 1]) + SQR(point_x[m] + y - point_x[m + 1]));
						int_E2[c3] = (float)SQR(avgdist - temp);
					}
					// printf("%f ", int_E2[c3]);
					c3++;
				}
			}// internal energy 2

		  // All the energies need to be in normalised before they could be added.

			int min1 = 10000;
			int max1 = 0;
			int min2 = 10000;
			int max2 = 0;
			int min3 = 10000;
			int max3 = 0;
			for (n = 0; n < (window * window); n++) {
				if (max1 < ext_E[n]) { max1 = ext_E[n]; }
				if (min1 > ext_E[n]) { min1 = ext_E[n]; }
			}

			for (n = 0; n < (window * window); n++) {
				ext_E[n] = 1 * ((ext_E[n] - min1) / (max1 - min1));
			}

			for (n = 0; n < (window * window); n++) {
				if (max2 < int_E1[n]) { max2 = int_E1[n]; }
				if (min2 > int_E1[n]) { min2 = int_E1[n]; }
			}

			for (n = 0; n < (window * window); n++) {
				int_E1[n] = 1 * ((int_E1[n] - min2) / (max2 - min2));
			}

			for (n = 0; n < (window * window); n++) {
				if (max3 < int_E2[n]) { max3 = int_E2[n]; }
				if (min3 > int_E2[n]) { min3 = int_E2[n]; }
			}
;
			for (n = 0; n < (window * window); n++) {
				int_E2[n] = 1 * ((int_E2[n] - min3) / (max3 - min3));
			}

			for (n = 0; n < window * window; n++) {
				total_E[n] = (float)(10 * (int_E1[n]) + 5 * (int_E2[n]) - 20 * (ext_E[n]));
				// printf("%f \n",total_E[n]);
			}
			// for(x=0;x<50;x++){
			//   printf("%f  ",total_E[x]);
			// }

			// We have to move the point to location with min energy. Hence - 
			int min_idx = 0;
			min = 10000;
			for (n = 0; n < window * window; n++) {
				if (min > total_E[n]) { min = total_E[n]; min_idx = n; }
			}

			point_x[m] = point_x[m] + (min_idx % window) - (window / 2);
			point_y[m] = point_y[m] + (min_idx / window) - (window / 2);
		} // contour points loop
		Sleep(30);
		PaintImage();
		sprintf(message, "Rubberband %d", l + 1);
		for (j = 0; j < total_points; j++) {
			hDC = GetDC(MainWnd);
			TextOut(hDC, 0, 0, message, strlen(message));
			int a, b;
			for (a = -2; a <= 2; a++)
			{
				for (b = -2; b <= 2; b++) {
					SetPixel(hDC, point_x[j] + a, point_y[j] + b, RGB(0, 0, 255));	/* color the cursor position red */
				}
			}
			ReleaseDC(MainWnd, hDC);
		}
		 //printf("Im here. %d \n", l);
	}//iter loop
	//for (k = 0; k < total_points; k++) {
	//	point_x[k] = px[k];
	//	point_y[k] = py[k];
	//	//hDC = GetDC(MainWnd);
	//	//sprintf(message, "Iteration %d", point_x[k]);
	//	//TextOut(hDC, 0, 0, message, strlen(message));
	//	//ReleaseDC(MainWnd, hDC);
	//}

}

void Balloon()
{/*reused from lab4*/
	HDC hDC;
	char message[320];
	int l, m, n, j, k; // counters
	int c1, c2, c3; // for energy loops
	int iter;
	int total_points = ballonct;
	float distance, avgdist, max, min;
	float ext_E[window_1 * window_1], int_E1[window_1 * window_1], int_E2[window_1 * window_1], total_E[window_1 * window_1];
	if (strcmp("D:\\Fall_22\\ECE 6310 Intro to CV\\Final Project\\hushpuppies-biscuits.pnm", filename) == 0) { iter = 50;}
	else { iter = 35; }
	//int *px, *py;
	//px = (int*)calloc(rubberct, sizeof(int));
	//py = (int*)calloc(rubberct, sizeof(int));
	//int px[1000];
	//int py[1000];
	//for (l = 0; l < rubberct; l++) {
	//	px[l] = point_x[l];
	//	py[l] = point_y[l];
	//}
	// printf("\n %d", window);

	for (l = 0; l < iter; l++) {
		distance = 0;
		for (m = 0; m < total_points; m++) {
			c1 = 0;
			for (x = (-window_1 / 2); x <= window_1 / 2; x++) {
				for (y = (-window_1 / 2); y <= window_1 / 2; y++) {
					ext_E[c1] = (float)SQR(norm_sobel[(bpoint_y[m] + x) * COLS + (bpoint_x[m] + y)]);
					// printf("%f ", ext_E[c1]);
					c1++;
				}
			} //  external energy
			// printf("External energy is %f \n",ext_E);

			c2 = 0;
			for (x = (-window_1 / 2); x <= window_1 / 2; x++) {
				for (y = (-window_1 / 2); y <= window_1 / 2; y++) {
					if (m == total_points - 1) { int_E1[c2] = (float)SQR((bpoint_y[m] + x) - (bpoint_y[0])) + SQR((bpoint_x[m] + y) - (bpoint_x[0])); }
					else { int_E1[c2] = (float)SQR((bpoint_y[m] + x) - (bpoint_y[0])) + SQR((bpoint_x[m] + y) - (bpoint_x[0])); }
					// printf("%f ", int_E1[c2]);
					c2++;
				}
			} // internal energy 1
			distance = 0.0;

			for (n = 0; n < total_points; n++) {
				if (m == total_points - 1)
				{
					distance += sqrt(SQR(bpoint_y[m] - bpoint_y[0]) + SQR(bpoint_x[m] - bpoint_x[0]));
				}
				else
				{
					distance += sqrt(SQR(bpoint_y[m] - bpoint_y[m + 1]) + SQR(bpoint_x[m] - bpoint_x[m + 1]));
				}
			}
			avgdist = (float)(distance / total_points);
			// printf("\n%f\n",avgdist);

			c3 = 0;
			for (x = (-window_1 / 2); x <= window_1 / 2; x++) {
				for (y = (-window_1 / 2); y <= window_1 / 2; y++) {
					int temp = 0;
					if (m == total_points - 1) {
						temp = sqrt(SQR(bpoint_y[m] + x - bpoint_y[0]) + SQR(bpoint_x[m] + y - bpoint_x[0]));
						int_E2[c3] = (float)SQR(avgdist - temp);
						// printf("Here! \n");
						// printf("%d \n",temp);
						// printf("%d \n",avgdist);
					}
					else {
						temp = sqrt(SQR(bpoint_y[m] + x - bpoint_y[m + 1]) + SQR(bpoint_x[m] + y - bpoint_x[m + 1]));
						int_E2[c3] = (float)SQR(avgdist - temp);
					}
					// printf("%f ", int_E2[c3]);
					c3++;
				}
			}// internal energy 2

		  // All the energies need to be in normalised before they could be added.

			int min1 = 10000;
			int max1 = 0;
			int min2 = 10000;
			int max2 = 0;
			int min3 = 10000;
			int max3 = 0;
			for (n = 0; n < (window_1 * window_1); n++) {
				if (max1 < ext_E[n]) { max1 = ext_E[n]; }
				if (min1 > ext_E[n]) { min1 = ext_E[n]; }
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				ext_E[n] = 1 * ((ext_E[n] - min1) / (max1 - min1));
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				if (max2 < int_E1[n]) { max2 = int_E1[n]; }
				if (min2 > int_E1[n]) { min2 = int_E1[n]; }
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				int_E1[n] = 1 * ((int_E1[n] - min2) / (max2 - min2));
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				if (max3 < int_E2[n]) { max3 = int_E2[n]; }
				if (min3 > int_E2[n]) { min3 = int_E2[n]; }
			}
			;
			for (n = 0; n < (window_1 * window_1); n++) {
				int_E2[n] = 1 * ((int_E2[n] - min3) / (max3 - min3));
			}

			for (n = 0; n < window_1 * window_1; n++) {
				total_E[n] = (float)( - 9 * (int_E1[n]) + 11 * (int_E2[n]) + 1 * (ext_E[n]));
				// printf("%f \n",total_E[n]);
			}
			// for(x=0;x<50;x++){
			//   printf("%f  ",total_E[x]);
			// }

			// We have to move the point to location with min energy. Hence - 
			int min_idx = 0;
			min = 10000;
			for (n = 0; n < window_1 * window_1; n++) {
				if (min > total_E[n]) { min = total_E[n]; min_idx = n; }
			}

			bpoint_x[m] = bpoint_x[m] + (min_idx % window_1) - (window_1 / 2);
			bpoint_y[m] = bpoint_y[m] + (min_idx / window_1) - (window_1 / 2);
		} // contour points loop
		Sleep(30);
		PaintImage();
		sprintf(message, "Balloon %d", l + 1);
		for (j = 0; j < total_points; j++) {
			hDC = GetDC(MainWnd);
			TextOut(hDC, 0, 0, message, strlen(message));
			int a, b;
			for (a = -2; a <= 2; a++)
			{
				for (b = -2; b <= 2; b++) {
					SetPixel(hDC, bpoint_x[j] + a, bpoint_y[j] + b, RGB(0, 0, 255));	/* color the cursor position red */
				}
			}
			ReleaseDC(MainWnd, hDC);
		}
		//printf("Im here. %d \n", l);
	}//iter loop
	//for (k = 0; k < total_points; k++) {
	//	point_x[k] = px[k];
	//	point_y[k] = py[k];
	//	//hDC = GetDC(MainWnd);
	//	//sprintf(message, "Iteration %d", point_x[k]);
	//	//TextOut(hDC, 0, 0, message, strlen(message));
	//	//ReleaseDC(MainWnd, hDC);
	//}

}

void Neutral()
{/*reused from lab4*/
	HDC hDC;
	char message[320];
	int l, m, n, j, k; // counters
	int c1, c2, c3; // for energy loops
	int iter = 20;
	int total_points = npointct;
	float distance, avgdist, max, min;
	float ext_E[window_1 * window_1], int_E1[window_1 * window_1], int_E2[window_1 * window_1], total_E[window_1 * window_1];

	//int *px, *py;
	//px = (int*)calloc(rubberct, sizeof(int));
	//py = (int*)calloc(rubberct, sizeof(int));
	//int px[1000];
	//int py[1000];
	//for (l = 0; l < rubberct; l++) {
	//	px[l] = point_x[l];
	//	py[l] = point_y[l];
	//}
	// printf("\n %d", window);

	for (l = 0; l < iter; l++) {
		distance = 0;
		for (m = 0; m < total_points; m++) {
			c1 = 0;
			if (m == anchor) {
				continue;
			}
			for (x = (-window_1 / 2); x <= window_1 / 2; x++) {
				for (y = (-window_1 / 2); y <= window_1 / 2; y++) {
					ext_E[c1] = (float)SQR(norm_sobel[(npoint_y[m] + x) * COLS + (npoint_x[m] + y)]);
					// printf("%f ", ext_E[c1]);
					c1++;
				}
			} //  external energy
			// printf("External energy is %f \n",ext_E);

			c2 = 0;
			for (x = (-window_1 / 2); x <= window_1 / 2; x++) {
				for (y = (-window_1 / 2); y <= window_1 / 2; y++) {
					if (m == total_points - 1) { int_E1[c2] = (float)SQR((npoint_y[m] + x) - (npoint_y[0])) + SQR((npoint_x[m] + y) - (npoint_x[0])); }
					else { int_E1[c2] = (float)SQR((npoint_y[m] + x) - (npoint_y[0])) + SQR((npoint_x[m] + y) - (npoint_x[0])); }
					// printf("%f ", int_E1[c2]);
					c2++;
				}
			} // internal energy 1
			distance = 0.0;

			for (n = 0; n < total_points; n++) {
				if (m == total_points - 1)
				{
					distance += sqrt(SQR(npoint_y[m] - npoint_y[0]) + SQR(npoint_x[m] - npoint_x[0]));
				}
				else
				{
					distance += sqrt(SQR(npoint_y[m] - npoint_y[m + 1]) + SQR(npoint_x[m] - npoint_x[m + 1]));
				}
			}
			avgdist = (float)(distance / total_points);
			// printf("\n%f\n",avgdist);

			c3 = 0;
			for (x = (-window_1 / 2); x <= window_1 / 2; x++) {
				for (y = (-window_1 / 2); y <= window_1 / 2; y++) {
					int temp = 0;
					if (m == total_points - 1) {
						temp = sqrt(SQR(npoint_y[m] + x - npoint_y[0]) + SQR(npoint_x[m] + y - npoint_x[0]));
						int_E2[c3] = (float)SQR(avgdist - temp);
						// printf("Here! \n");
						// printf("%d \n",temp);
						// printf("%d \n",avgdist);
					}
					else {
						temp = sqrt(SQR(npoint_y[m] + x - npoint_y[m + 1]) + SQR(npoint_x[m] + y - npoint_x[m + 1]));
						int_E2[c3] = (float)SQR(avgdist - temp);
					}
					// printf("%f ", int_E2[c3]);
					c3++;
				}
			}// internal energy 2

		  // All the energies need to be in normalised before they could be added.

			int min1 = 10000;
			int max1 = 0;
			int min2 = 10000;
			int max2 = 0;
			int min3 = 10000;
			int max3 = 0;
			for (n = 0; n < (window_1 * window_1); n++) {
				if (max1 < ext_E[n]) { max1 = ext_E[n]; }
				if (min1 > ext_E[n]) { min1 = ext_E[n]; }
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				ext_E[n] = 1 * ((ext_E[n] - min1) / (max1 - min1));
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				if (max2 < int_E1[n]) { max2 = int_E1[n]; }
				if (min2 > int_E1[n]) { min2 = int_E1[n]; }
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				int_E1[n] = 1 * ((int_E1[n] - min2) / (max2 - min2));
			}

			for (n = 0; n < (window_1 * window_1); n++) {
				if (max3 < int_E2[n]) { max3 = int_E2[n]; }
				if (min3 > int_E2[n]) { min3 = int_E2[n]; }
			}
			;
			for (n = 0; n < (window_1 * window_1); n++) {
				int_E2[n] = 1 * ((int_E2[n] - min3) / (max3 - min3));
			}

			//if(strcmp())

			for (n = 0; n < window_1 * window_1; n++) {
				total_E[n] = (float)(0 * (int_E1[n]) - 10 * (int_E2[n]) - 1 * (ext_E[n]));
				// printf("%f \n",total_E[n]);
			}
			// for(x=0;x<50;x++){
			//   printf("%f  ",total_E[x]);
			// }

			// We have to move the point to location with min energy. Hence - 
			int min_idx = 0;
			min = 10000;
			for (n = 0; n < window_1 * window_1; n++) {
				if (min > total_E[n]) { min = total_E[n]; min_idx = n; }
			}

			npoint_x[m] = npoint_x[m] + (min_idx % window_1) - (window_1 / 2);
			npoint_y[m] = npoint_y[m] + (min_idx / window_1) - (window_1 / 2);
		} // contour points loop
		Sleep(30);
		PaintImage();
		sprintf(message, "Neutral %d", l + 1);
		for (j = 0; j < total_points; j++) {
			hDC = GetDC(MainWnd);
			TextOut(hDC, 0, 0, message, strlen(message));
			int a, b;
			for (a = -2; a <= 2; a++)
			{
				for (b = -2; b <= 2; b++) {
					SetPixel(hDC, npoint_x[j] + a, npoint_y[j] + b, RGB(0, 150, 0));	/* color the cursor position red */
				}
			}
			ReleaseDC(MainWnd, hDC);
		}
		//printf("Im here. %d \n", l);
	}//iter loop
	//for (k = 0; k < total_points; k++) {
	//	point_x[k] = px[k];
	//	point_y[k] = py[k];
	//	//hDC = GetDC(MainWnd);
	//	//sprintf(message, "Iteration %d", point_x[k]);
	//	//TextOut(hDC, 0, 0, message, strlen(message));
	//	//ReleaseDC(MainWnd, hDC);
	//}

}


void Greyscaler()
{
	int gc, gr;
	GreyscaleImage = (unsigned char *)calloc(ROWS * COLS, sizeof(unsigned char));
	for (gr = 0; gr < ROWS; gr++) {
		for (gc = 0; gc < COLS; gc++) {
			GreyscaleImage[gr * COLS + gc] = (int)(( OriginalImage[(gr*COLS+gc)*3] + OriginalImage[(gr*COLS+gc)*3+1] + OriginalImage[(gr*COLS+gc)*3+2]) / 3);
		}
	}
}

void Sobeler()
{/* Code block from assignment 5 */
	int x, y, x1, y1,i;
	float sob_x, sob_y,min,max;
	int sob_fx[9] = { -1,0,1,-2,0,2,-1,0,1 };
	int sob_fy[9] = { 1,2,1,0,0,0,-1,-2,-1 };
	SobelImage = (float *)calloc(ROWS * COLS, sizeof(float));
	norm_sobel = (unsigned char*)calloc(ROWS * COLS, sizeof(unsigned char));
	norm_sob = (unsigned char*)calloc(ROWS * COLS, sizeof(unsigned char));

	for (x = 1; x < ROWS - 1; x++) {
		for (y = 1; y < COLS - 1; y++) {
			sob_x = 0.0;
			sob_y = 0.0;
			for (x1 = -1; x1 <= 1; x1++) {
				for (y1 = -1; y1 <= 1; y1++) {
					sob_x += GreyscaleImage[(x + x1) * COLS + (y + y1)] * sob_fx[(x1 + 1) * 3 + (y1 + 1)];
					sob_y += GreyscaleImage[(x + x1) * COLS + (y + y1)] * sob_fy[(x1 + 1) * 3 + (y1 + 1)];
				}
			}
			SobelImage[x * COLS + y] = sqrt(SQR(sob_x) + SQR(sob_y));
		}
	}

	max = SobelImage[0];
	min = SobelImage[0];

	for (i = 0; i < ROWS * COLS; i++) {
		if (max < SobelImage[i]) { max = SobelImage[i]; }
		if (min > SobelImage[i]) { min = SobelImage[i]; }
	}

	// printf("max: %f\tmin: %f",max,min);
	// //Let us now normalize the min and max

	for (x = 0; x < ROWS; x++) {
		for (y = 0; y < COLS; y++) {
			norm_sobel[x * COLS + y] = 255 * ((SobelImage[x * COLS + y] - min) / (max - min));
		}
	}

	for (x = 0; x < ROWS; x++) {
		for (y = 0; y < COLS; y++) {
			norm_sob[x * COLS + y] = ((SobelImage[x * COLS + y] - min) / (max - min));
		}
	}
}


//BOOL CALLBACK DlgBx(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
//{
//    switch(Message)
//    {
//        case WM_INITDIALOG:
//        return TRUE;
//        case WM_COMMAND:
//            switch(LOWORD(wParam))
//            {
//                case IDOK:
//					pred_1 = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, TRUE);
//					pred_2 = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, TRUE);
//                    EndDialog(hDlg, IDOK);
//					return TRUE;
//					break;
//                case IDCANCEL:
//                    if (MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
//						MB_ICONQUESTION | MB_YESNO) == IDYES)
//					{
//						EndDialog(hDlg, IDCANCEL);
//					}
//					return TRUE;
//					break;
//            }
//        break;
//        default:
//            return FALSE;
//    }
//    return TRUE;
//}
void NeutralTest() {
	HDC hDC;
	char message[320];
	int l;
	

	for (l = 0; l < 2000; l++) {
		hDC = GetDC(MainWnd);
		sprintf(message, "Neutral %d", l + 1);
		TextOut(hDC, 0, 0, message, strlen(message));
		ReleaseDC(MainWnd, hDC);
	}

}

void BallonTest() {
	HDC hDC;
	char message[320];
	int l;


	for (l = 0; l < 2000; l++) {
		hDC = GetDC(MainWnd);
		sprintf(message, "ballon %d", l + 1);
		TextOut(hDC, 0, 0, message, strlen(message));
		ReleaseDC(MainWnd, hDC);
	}

}


LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam)

{
HMENU				hMenu;
OPENFILENAME		ofn;
FILE				*fpt;
HDC					hDC;
char				header[320],text[320];
int					BYTES,xPos,yPos;

switch (uMsg)
  {
  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
	  case ID_DISPLAY_RESTOREORIGINAL:
		  PaintImage();
		  rthread = 0;
		  bthread = 0;
		  nready = 0;
		  break;

	  case ID_DISPLAY_SOBELIMAGE:
		  org2sob = (org2sob + 1) % 2;
		  if (org2sob) { PaintSobel();}
		  else { PaintImage();}		  
		  break;

	  case ID_FILE_LOAD:
		if (OriginalImage != NULL)
		  {
		  free(OriginalImage);
		  OriginalImage=NULL;
		  }
		memset(&(ofn),0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.lpstrFile=filename;
		filename[0]=0;
		ofn.nMaxFile=MAX_FILENAME_CHARS;
		ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
		ofn.lpstrFilter = "PNM files\0*.pnm\0All files\0*.*\0\0";
		if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
		  break;		/* user cancelled load */
		if ((fpt=fopen(filename,"rb")) == NULL)
		  {
		  MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		  break;
		  }
		fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
		if (strcmp(header,"P6") != 0  ||  BYTES != 255)
		  {
		  MessageBox(NULL,"Not a PPM (P6 RGB) image",filename,MB_OK | MB_APPLMODAL);
		  fclose(fpt);
		  break;
		  }
		OriginalImage=(unsigned char *)calloc(ROWS*COLS*3,1);
		header[0]=fgetc(fpt);	/* whitespace character after header */
		fread(OriginalImage,1,ROWS*COLS*3,fpt);
		fclose(fpt);
		Greyscaler();
		Sobeler();
		SetWindowText(hWnd,filename);
		SetWindowPos(hWnd, 0, 0, 0, COLS+20, ROWS+60, SWP_SHOWWINDOW | SWP_NOMOVE);
		PaintImage();
		break;

      case ID_FILE_QUIT:
        DestroyWindow(hWnd);
        break;
      }
    break;
  case WM_SIZE:		  /* could be used to detect when window size changes */
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_PAINT:
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_WINDOWPOSCHANGED:
	  PaintImage();
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;
  
  case WM_LBUTTONDOWN:
	  //PostMessage(MainWnd, WM_COMMAND, ID_DISPLAY_RESTOREORIGINAL, 0); /* to forget the old points */
	  //keystat = GetKeyState(VK_SHIFT);/*shift has already been pressed hence GetKeyState and not GetAsyncKeyState*/
	  //keystat = 0;
	  if (nready) {/*neutral stuff neutral = 0; keystat = 0;*/
		  //_beginthread(NeutralTest, 0, MainWnd);
		  keystat = GetKeyState(VK_SHIFT);
	  }
	  else {/*if shift is not held, ldown is 1 and point collection will start in mousemoove*/
		  PaintImage();
		  ldown = 1;
		  neutral = 0;
		  Totalpts = 0;
		  rubberct = 0;
	  }
	  if (nready && keystat) {
		  if (rthread == 1) {
			  npointct = rubberct;
			  int d;
			  for (d = 0; d < npointct; d++) {
				  npoint_x[d] = point_x[d];
				  npoint_y[d] = point_y[d];
			  }
			  anchor = 0;
		  }
		  else if (bthread == 1) {
			  npointct = ballonct;
			  int d;
			  for (d = 0; d < npointct; d++) {
				  npoint_x[d] = bpoint_x[d];
				  npoint_y[d] = bpoint_y[d];
			  }
			  anchor = 0;
		  }

		  xPos = LOWORD(lParam);
		  yPos = HIWORD(lParam);
		  hDC = GetDC(MainWnd);
		  int a, b, c;
		  for (a = 0; a < npointct; a++) {
			  if (npoint_x[a] >= xPos - 3 && npoint_x[a] <= xPos + 3 && npoint_y[a] >= yPos - 3 && npoint_y[a] <= yPos + 3) {
				  anchor = a; /* Higher Number in in Xpos - 3 will lead to bigger point handle */
				  break;
			  }
		  }
		  ReleaseDC(MainWnd, hDC);
	  }

	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_LBUTTONUP:
	  //keystat = GetKeyState(VK_SHIFT);
	  //if (nready) {/*neutral stuff neutral = 0; keystat = 0;*/
	  ////_beginthread(NeutralTest, 0, MainWnd);
		 // keystat = GetKeyState(VK_SHIFT);
	  //}
	  if(nready && keystat) {
		  npoint_x[anchor] = LOWORD(lParam); /*On button up you store this point for in the anchor index*/
		  npoint_y[anchor] = HIWORD(lParam);
		  //PaintImage();
		  hDC = GetDC(MainWnd);
		  int a,b,c;
		  for (a = 0; a < npointct; a++) {
			  for (b = -1; b <= 1; b++) {
				  for (c = -1; c <= 1; c++) {
					  if (a == anchor) { SetPixel(hDC, npoint_x[a]+b, npoint_y[a]+c, RGB(155, 20, 100)); }
					  else { SetPixel(hDC, npoint_x[a] + b, npoint_y[a] + c, RGB(0, 200, 0)); }
				  }
			  }
		  }
		  ReleaseDC(MainWnd, hDC);
		  _beginthread(Neutral, 0, MainWnd);
		  nready = 0; rthread = 0; bthread = 0;
	  }
	  else {
		  ldown = 0;
		  //PaintImage();
		  //sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
		  //hDC = GetDC(MainWnd);
		  //rubberct = 0;
		  //TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
		  //for (i = 0; i < Totalpts; i++) {
			 // if (i % 5 == 0){
				//  pnt_x[rubberct] = point_x[i];
				//  pnt_y[rubberct] = point_y[i];
				//  rubberct++;
		  //    }		  
		  //}
		  //int a, b;
		  //for (x = 0; x < rubberct; x++) {
			 // for (a = -1; a <= 1; a++)
			 // {
				//  for (b = -1; b <= 1; b++) {
				//	  SetPixel(hDC, pnt_x[x] + a, pnt_y[x] + b, RGB(0, 255, 0));	/* color the cursor position red */
				//  }
			 // }
			 // ReleaseDC(MainWnd, hDC);
		  //}
		  _beginthread(Rubberband, 0, MainWnd);
		  rthread = 1; bthread = 0; nready = 1;
	  }
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_RBUTTONDOWN:
	//PostMessage(MainWnd, WM_COMMAND, ID_DISPLAY_RESTOREORIGINAL, 0);
	ballonct = 0;
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;

  case WM_RBUTTONUP:
	  PaintImage();
	  xPos = LOWORD(lParam);
	  yPos = HIWORD(lParam);
	  hDC = GetDC(MainWnd);
	  if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
	  {
		  
		  for (r = 0; r < 2 * M_PI; r += 0.1) {
			  pnt_x[ballonct] = xPos + 10*cos(r);
			  pnt_y[ballonct] = yPos + 10*sin(r);
			  ballonct++;
			  SetPixel(hDC, xPos + 10 * cos(r), yPos + 10 * sin(r), RGB(255, 0, 0));
		  }
		  int temp = 0;
		  for (int a = 0; a < ballonct; a += 3) {
			  bpoint_x[temp] = pnt_x[a];
			  bpoint_y[temp] = pnt_y[a];
			  temp++;
		  }
		  ballonct = temp;
		  //int temp = 0;
		  //for (r = 0; r < ballonct; r += 3) {
			 // bpoint_x[temp] = bpoint_x[ballonct];
			 // bpoint_y[temp] = bpoint_y[ballonct];
			 // temp++;
		  //}
		  //ballonct = temp;
		  //sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
		  //hDC = GetDC(MainWnd);
		  //TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
		  //int a, b,c;
		  //for (c = 0; c < temp; c++) {
			 // for (a = -5; a <= 5; a++)
			 // {
				//  for (b = -5; b <= 5; b++) {
				//	  SetPixel(hDC, bpoint_x[c] + a, bpoint_y[c] + b, RGB(255, 0, 0));	/* color the cursor position red */
				//  }
			 // }
		  //}
		  /*ReleaseDC(MainWnd, hDC);*/
	  }
	  ReleaseDC(MainWnd, hDC);
	  _beginthread(Balloon, 0, MainWnd);
	  bthread = 1; rthread = 0; nready = 1;
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_MOUSEMOVE:
	if (ldown == 1)
	  {
	  xPos=LOWORD(lParam);
	  yPos=HIWORD(lParam);
	  if (xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
		{
		  if (Totalpts % 5 == 0) {
			  point_x[rubberct] = xPos;
			  point_y[rubberct] = yPos;
			  rubberct++;
		  }
		  Totalpts++;
		//sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
		hDC=GetDC(MainWnd);
		//TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
		int a, b;
		for (a = -1; a <= 1; a++) 
		{
			for (b = -1; b <= 1; b++) {
				SetPixel(hDC, xPos+a, yPos+b, RGB(255, 0, 0));	/* color the cursor position red */
			}
		}
		ReleaseDC(MainWnd,hDC);
		}
	  }
	if (neutral = 1) {/* get points for neutral method */ }
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_KEYDOWN:
	if (wParam == 's'  ||  wParam == 'S')
	  PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* send message to self */
	if (wParam == 'j' || wParam == 'J')
	{
		mode_trig = 1;
	}
	if (wParam == 'p' || wParam == 'P') {
		if (play_mode == 1){
			PostMessage(MainWnd, WM_COMMAND, ID_GROWMODE_STEP, 0);
		}
		else{
			PostMessage(MainWnd, WM_COMMAND, ID_GROWMODE_PLAY, 0);
		}
	}
	if (wParam == 'q' || wParam == 'Q')
	{
		PostMessage(MainWnd, WM_COMMAND, ID_DISPLAY_RESTOREORIGINAL, 0);
	}
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
	hDC=GetDC(MainWnd);
	SetPixel(hDC,TimerCol,TimerRow,RGB(0,0,255));	/* color the animation pixel blue */
	ReleaseDC(MainWnd,hDC);
	TimerRow++;
	TimerCol+=2;
	break;
  case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
    break;
  }

hMenu=GetMenu(MainWnd);

if (ShowPixelCoords == 1)
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);

if (play_mode == 1)
CheckMenuItem(hMenu, ID_GROWMODE_PLAY, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
CheckMenuItem(hMenu, ID_GROWMODE_PLAY, MF_UNCHECKED);

if (step_mode == 1)
CheckMenuItem(hMenu, ID_GROWMODE_STEP, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
CheckMenuItem(hMenu, ID_GROWMODE_STEP, MF_UNCHECKED);

if (org2sob == 1)
CheckMenuItem(hMenu, ID_DISPLAY_SOBELIMAGE, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
CheckMenuItem(hMenu, ID_DISPLAY_SOBELIMAGE, MF_UNCHECKED);


DrawMenuBar(hWnd);

return(0L);
}

void PaintSobel()

{
	PAINTSTRUCT			Painter;
	HDC					hDC;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO* bm_info;
	int					i, r, c, DISPLAY_ROWS, DISPLAY_COLS;
	unsigned char* DisplayImage;

	if (OriginalImage == NULL)
		return;		/* no image to draw */

	/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
	DISPLAY_ROWS = ROWS;
	DISPLAY_COLS = COLS;
	if (DISPLAY_ROWS % 4 != 0)
		DISPLAY_ROWS = (DISPLAY_ROWS / 4 + 1) * 4;
	if (DISPLAY_COLS % 4 != 0)
		DISPLAY_COLS = (DISPLAY_COLS / 4 + 1) * 4;
	DisplayImage = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS, 1);
	for (r = 0; r < ROWS; r++)
		for (c = 0; c < COLS; c++)
			DisplayImage[r * DISPLAY_COLS + c] = norm_sobel[r * COLS + c];

	BeginPaint(MainWnd, &Painter);
	hDC = GetDC(MainWnd);
	bm_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bm_info_header.biWidth = DISPLAY_COLS;
	bm_info_header.biHeight = -DISPLAY_ROWS;
	bm_info_header.biPlanes = 1;
	bm_info_header.biBitCount = 8;
	bm_info_header.biCompression = BI_RGB;
	bm_info_header.biSizeImage = 0;
	bm_info_header.biXPelsPerMeter = 0;
	bm_info_header.biYPelsPerMeter = 0;
	bm_info_header.biClrUsed = 256;
	bm_info_header.biClrImportant = 256;
	bm_info = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	bm_info->bmiHeader = bm_info_header;
	for (i = 0; i < 256; i++)
	{
		bm_info->bmiColors[i].rgbBlue = bm_info->bmiColors[i].rgbGreen = bm_info->bmiColors[i].rgbRed = i;
		bm_info->bmiColors[i].rgbReserved = 0;
	}

	SetDIBitsToDevice(hDC, 0, 0, DISPLAY_COLS, DISPLAY_ROWS, 0, 0,
		0, /* first scan line */
		DISPLAY_ROWS, /* number of scan lines */
		DisplayImage, bm_info, DIB_RGB_COLORS);
	ReleaseDC(MainWnd, hDC);
	EndPaint(MainWnd, &Painter);

	free(DisplayImage);
	free(bm_info);
}


void PaintImage()

{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			*bm_info;
int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
unsigned char		*DisplayImage;

if (OriginalImage == NULL)
  return;		/* no image to draw */

		/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
DISPLAY_ROWS=ROWS;
DISPLAY_COLS=COLS;
if (DISPLAY_ROWS % 4 != 0)
  DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
if (DISPLAY_COLS % 4 != 0)
  DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
for (r=0; r<ROWS; r++)
  for (c=0; c<COLS; c++)
	DisplayImage[r*DISPLAY_COLS+c] = GreyscaleImage[r*COLS+c];

BeginPaint(MainWnd,&Painter);
hDC=GetDC(MainWnd);
bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
bm_info_header.biWidth=DISPLAY_COLS;
bm_info_header.biHeight=-DISPLAY_ROWS; 
bm_info_header.biPlanes=1;
bm_info_header.biBitCount=8; 
bm_info_header.biCompression=BI_RGB; 
bm_info_header.biSizeImage=0; 
bm_info_header.biXPelsPerMeter=0; 
bm_info_header.biYPelsPerMeter=0;
bm_info_header.biClrUsed=256;
bm_info_header.biClrImportant=256;
bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
bm_info->bmiHeader=bm_info_header;
for (i=0; i<256; i++)
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
			  0, /* first scan line */
			  DISPLAY_ROWS, /* number of scan lines */
			  DisplayImage,bm_info,DIB_RGB_COLORS);
ReleaseDC(MainWnd,hDC);
EndPaint(MainWnd,&Painter);

free(DisplayImage);
free(bm_info);
}
