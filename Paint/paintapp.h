#pragma once

#include "resource.h"
#include "drawer.h"

#define MAX_LOADSTRING 100

#define ID_PENTOOL 10000
#define ID_ERASERTOOL 10001
#define ID_FILLTOOL 100002


// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hStatus;									// status bar
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HDC hdc;

HDC memDC;
HBITMAP memBM;

HDC gradientDC;
HBITMAP gradientBM;

Drawer drawer;
HPEN tempPen;
HPEN eraser;
HPEN hPen;
HBRUSH hShapeBrush;
HBRUSH hFillBrush;
LOGBRUSH lb;



bool isPolyLineDrawing = false;
bool isFill = false;
bool isGradientMode = false;

COLORREF eraserColor = RGB(255, 255, 255); // white eraser
COLORREF penColor = RGB(0, 0, 0); // default black
int penWidth = 2; // default 2px
int penStyle = PS_SOLID; // default solid style

COLORREF areaFillColor = RGB(255, 255, 255); // default white
COLORREF shapeFillColor = RGB(255, 255, 255); // default white
int shapeToDraw = ID_SHAPES_PENCIL; // default pencil
int currentTool = ID_PENTOOL; //default pen




// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void MenuCommand(HWND, WPARAM);
void SetMenuTool(int tool, HMENU hMenu);
void SetMenuPenStyle(int penStyle, HMENU hMenu);
void SetMenuPenWidth(int penWidth, HMENU hMenu);
void SetMenuShape(int shape, HMENU hMenu);
void _ChooseColor(HWND hWnd, COLORREF& color);
void ToggleMenuFill(HMENU hMenu);
void ToggleMenuGradientMode(HMENU hMenu);
void UpdatePen();
void UpdateFillBrush();
void UpdateShapeBrush();
void UpdateStatusBar(bool clicked, int x, int y);
void UpdatePenStatusBox();
void UpdateClickStatusBox(bool clickStatus);
void UpdateCoordStatusBox(int x, int y);
void GradientFloodFill(HDC dc, int dcWidth, int dcHeight, int x, int y, COLORREF newColor);
bool isCoordinateValid(int x, int y, int dcWidth, int dcHeight);