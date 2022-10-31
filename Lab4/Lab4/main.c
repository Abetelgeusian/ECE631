
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

hWnd=CreateWindow("PLUS","Lab4 program",
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

MessageBox(NULL, TEXT("The Keybinds are: \n Press S to show pixel coordinates of the mouse cursor \n Press P to switch between Play and Step mode \n Press Q to clear the image \n Press J to Step mode \n While in Step mode, you will see clicked point appear after pressing/holding J."), TEXT("Keybinds"), MB_OK);

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



BOOL CALLBACK DlgBx(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:
        return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
					pred_1 = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, TRUE);
					pred_2 = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, TRUE);
                    EndDialog(hDlg, IDOK);
					return TRUE;
					break;
                case IDCANCEL:
                    if (MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
						MB_ICONQUESTION | MB_YESNO) == IDYES)
					{
						EndDialog(hDlg, IDCANCEL);
					}
					return TRUE;
					break;
            }
        break;
        default:
            return FALSE;
    }
    return TRUE;
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
	  case ID_SHOWPIXELCOORDS:
		ShowPixelCoords=(ShowPixelCoords+1)%2;
		PaintImage();
		break;

	  case ID_REGIONGROW_PREDICATES:
		  DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, DlgBx);
		  break;

	  case ID_GROWMODE_PLAY:
		  play_mode = (play_mode + 1) % 2;
		  if (play_mode == 1) { step_mode = 0; restore = 0; mode_trig = 1; } // You will need a trigger for the loop in regiongrow fucntion.
		  break;

	  case ID_GROWMODE_STEP:
		  step_mode = (step_mode + 1) % 2;
		  if (step_mode == 1) { play_mode = 0; restore = 0;}
		  break;

	  case ID_GROWMODE_RESTOREORIGINAL:
		  restore = (restore + 1) % 2;
		  if (restore == 1) { play_mode = 0; step_mode = 0; ThreadRunning = 0; PaintImage();}
		  break;

	  case ID_COLORS_RED:
		  red = (red + 1) % 2;
		  if (red == 1) { green = 0; blue = 0; yellow = 0; }
		  break;

	  case ID_COLORS_BLUE:
		  blue = (blue + 1) % 2;
		  if (blue == 1) { red = 0; green = 0; yellow = 0; }
		  break;
	  case ID_COLORS_YELLOW:
		  yellow = (yellow + 1) % 2;
		  if (yellow == 1) { green = 0; blue = 0; red = 0; }
		  break;

	  case ID_COLORS_GREEN:
		  green = (green + 1) % 2;
		  if (green == 1) { red = 0; blue = 0; yellow = 0; }
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
		ofn.lpstrFilter = "PPM files\0*.ppm\0All files\0*.*\0\0";
		if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
		  break;		/* user cancelled load */
		if ((fpt=fopen(filename,"rb")) == NULL)
		  {
		  MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		  break;
		  }
		fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
		if (strcmp(header,"P5") != 0  ||  BYTES != 255)
		  {
		  MessageBox(NULL,"Not a PPM (P5 greyscale) image",filename,MB_OK | MB_APPLMODAL);
		  fclose(fpt);
		  break;
		  }
		OriginalImage=(unsigned char *)calloc(ROWS*COLS,1);
		header[0]=fgetc(fpt);	/* whitespace character after header */
		fread(OriginalImage,1,ROWS*COLS,fpt);
		fclose(fpt);
		SetWindowText(hWnd,filename);
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
  
  case WM_LBUTTONDOWN:
	  if (play_mode == 1 || step_mode == 1) {
		  c_x = LOWORD(lParam);
		  c_y = HIWORD(lParam);
		  ThreadRunning = 1;
		  _beginthread(RegionGrowThread, 0, MainWnd);
	  }
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;
  case WM_RBUTTONDOWN:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_MOUSEMOVE:
	if (ShowPixelCoords == 1)
	  {
	  xPos=LOWORD(lParam);
	  yPos=HIWORD(lParam);
	  if (xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
		{
		sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
		hDC=GetDC(MainWnd);
		TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
		SetPixel(hDC,xPos,yPos,RGB(255,0,0));	/* color the cursor position red */
		ReleaseDC(MainWnd,hDC);
		}
	  }
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
		PostMessage(MainWnd, WM_COMMAND, ID_GROWMODE_RESTOREORIGINAL, 0);
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


DrawMenuBar(hWnd);

return(0L);
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
	DisplayImage[r*DISPLAY_COLS+c]=OriginalImage[r*COLS+c];

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

void RegionGrowThread(HWND AnimationWindowHandle) 
{
	int * indices, RegionSize;
	unsigned char* labels;
	int TotalRegions;
	labels = (unsigned char *)calloc(ROWS * COLS, sizeof(unsigned char));
	indices = (int*)calloc(ROWS * COLS, sizeof(int));
	TotalRegions = 1;
	RegionGrow(OriginalImage, labels, ROWS, COLS, c_y, c_x, 0, TotalRegions,indices, &RegionSize);
}

// The following code is used as is from the coding challenge in class with minor changes.

void RegionGrow(unsigned char* image,	/* image data */
	unsigned char* labels,	/* segmentation labels */
	int ROWS, int COLS,	/* size of image */
	int r, int c,		/* pixel to paint from */
	int paint_over_label,	/* image label to paint over */
	int new_label,		/* image label for painting */
	int* indices,		/* output:  indices of pixels painted */
	int* count /* output:  count of pixels painted */)
{
	HDC hDC;
	int distance;
	int	r2, c2;
	int	queue[MAX_QUEUE], qh, qt;
	int	average, total;	/* average and total intensity in growing region */

	*count = 0;
	if (labels[r * COLS + c] != paint_over_label)
		return;
	labels[r * COLS + c] = 255;

	average = total = (int)image[r * COLS + c];
	if (indices != NULL)
		indices[0] = r * COLS + c;
	queue[0] = r * COLS + c;
	qh = 1;	/* queue head */
	qt = 0;	/* queue tail */
	(*count) = 1;
	while (qt != qh && ThreadRunning == 1)
	{
		if (mode_trig == 1) {

			if ((*count) % 50 == 0)	/* recalculate average after each 50 pixels join */
			{
				average = total / (*count);
				// printf("new avg=%d\n",average);
			}
			for (r2 = -1; r2 <= 1; r2++)
				for (c2 = -1; c2 <= 1; c2++)
				{
					if (r2 == 0 && c2 == 0)
						continue;

					if ((queue[qt] / COLS + r2) < 0 || (queue[qt] / COLS + r2) >= ROWS ||
						(queue[qt] % COLS + c2) < 0 || (queue[qt] % COLS + c2) >= COLS)
						continue;

					if (labels[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] != paint_over_label)
						continue;

					/* test criteria to join region */

					if (abs((int)(image[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2])
						- average) > pred_1)
						continue;

					distance = sqrt(SQR(r - (queue[qt] / COLS + r2)) + SQR(c-(queue[qt] % COLS + c2)));
					if (distance > pred_2) { continue; }

					labels[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] = 255;

					hDC = GetDC(MainWnd);
					if (red == 1) { SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RGB(255, 0, 0)); }
					if (blue == 1) { SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RGB(0, 0, 255)); }
					if (yellow == 1) { SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RGB(255, 255, 0)); }
					if (green == 1) { SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RGB(0, 255, 0)); }
					ReleaseDC(MainWnd, hDC);

					if (indices != NULL)
						indices[*count] = (queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2;
					total += image[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2];
					(*count)++;
					queue[qh] = (queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2;
					qh = (qh + 1) % MAX_QUEUE;
					if (qh == qt)
					{
						printf("Max queue size exceeded\n");
						exit(0);
					}
				}
			qt = (qt + 1) % MAX_QUEUE;
			if (play_mode == 1) { Sleep(1);}
			else if (step_mode == 1) { mode_trig = 0;}
		}
	}
}
