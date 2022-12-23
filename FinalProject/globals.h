
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320
#define MAX_QUEUE 10000
#define window 21
#define window_1 7

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;
int		org2sob,ldown,keystat,neutral,nready,anchor;
int		rthread, bthread, nthread;
int		pred_1 = 100, pred_2 = 200,play_mode,step_mode,restore,red,blue,green,yellow,mode_trig;
int		c_x,c_y,i,Totalpts,point_x[3000], point_y[3000],bpoint_x[100], bpoint_y[100],rubberct,ballonct,npointct;
int		pnt_x[3000], pnt_y[3000], x, y, npoint_x[3000],npoint_y[3000];
double r;

		// Image data
unsigned char *OriginalImage,*GreyscaleImage,*norm_sobel,*norm_sob;
float	*SobelImage;
int				ROWS,COLS;

#define TIMER_SECOND	1			/* ID of timer used for animation */

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void PaintImage();
void PaintSobel();
void RegionGrowThread(void *);
void RegionGrow(unsigned char*,unsigned char*, int, int, int, int, int, int, int*, int*);