#include "stdafx.h"
#include "paintapp.h"
#include "drawer.h"

#include <vector>
#include <string>
#include <CommCtrl.h>
#include <commdlg.h>
#include <fstream>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <queue>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PAINTAPP, szWindowClass, MAX_LOADSTRING);

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAINTAPP));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_PENCILCURSOR));
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PAINTAPP);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALLICON));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, 
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0, 
		CW_USEDEFAULT, 
		0,
		nullptr, 
		nullptr, 
		hInstance, 
		nullptr);

	if (!hWnd)
	{
		return false;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return true;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int window_width;
	static bool clicked = false;

	static int x = 0;
	static int y = 0;

	static POINTS ptsBegin;
	static POINTS ptsEnd;
	static BOOL fPrevLine = false;

	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	HCURSOR pencilCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_PENCILCURSOR));
	HCURSOR defaultCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_DEFAULTCURSOR));
	HCURSOR eraserCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ERASERCURSOR));
	HCURSOR fillCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_FILLCURSOR));



	switch (message)
	{
		case WM_CREATE:
		{
			// init pen/logbrush/brush
			lb.lbColor = penColor;
			lb.lbStyle = BS_SOLID;
			hPen = ExtCreatePen(PS_GEOMETRIC | penStyle, penWidth, &lb, 0, NULL);
			eraser = CreatePen(PS_SOLID, penWidth, eraserColor);
			hShapeBrush = CreateSolidBrush(shapeFillColor);
			hFillBrush = CreateSolidBrush(areaFillColor);

			hdc = GetDC(hWnd);

			// memDC/memBM
			memDC = CreateCompatibleDC(hdc);

			memBM = CreateCompatibleBitmap(hdc, 
				GetDeviceCaps(hdc, HORZRES),
				GetDeviceCaps(hdc, VERTRES));

			SelectObject(memDC, memBM);
			SelectObject(memDC, hPen);
			PatBlt(memDC, 0, 0, 
				GetDeviceCaps(hdc, HORZRES), 
				GetDeviceCaps(hdc, VERTRES), WHITENESS);

			// gradientDC/gradientBM
			gradientDC = CreateCompatibleDC(hdc);
			gradientBM = CreateCompatibleBitmap(hdc,
				GetDeviceCaps(hdc, HORZRES),
				GetDeviceCaps(hdc, VERTRES));
			SelectObject(gradientDC, gradientBM);
			PatBlt(gradientDC, 0, 0,
				GetDeviceCaps(hdc, HORZRES),
				GetDeviceCaps(hdc, VERTRES), WHITENESS);
			

			// init status bar		
			hStatus = CreateWindow(STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
				hWnd, (HMENU)ID_STATUSBAR, hInst, NULL);
			std::wstring st1 = L"clicked: " + std::to_wstring(clicked);
			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)st1.c_str());
		}
		break;

		case WM_COMMAND:
		{
			MenuCommand(hWnd, wParam);
		}
		break;

		case WM_SIZE:
		{
			window_width = LOWORD(lParam);

			SendMessage(hStatus, WM_SIZE, 0, 0);
			int statwidths[3] = { window_width / 5, 2 * window_width / 5, -1 };
			SendMessage(hStatus, SB_SETPARTS, 3, (LPARAM)statwidths);
			UpdatePenStatusBox();
		}
		break;

		case WM_LBUTTONDOWN:
		{
			clicked = true;
			UpdateClickStatusBox(clicked);

			if (currentTool == ID_FILLTOOL && isGradientMode) {

				HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hFillBrush);
				int dcWidth = GetDeviceCaps(memDC, HORZRES);
				int dcHeight = GetDeviceCaps(memDC, VERTRES);
				GradientFloodFill(memDC, dcWidth, dcHeight, x, y, areaFillColor);

				StretchBlt(hdc, 0, 0,
					GetDeviceCaps(hdc, HORZRES),
					GetDeviceCaps(hdc, VERTRES),
					memDC, 0, 0,
					GetDeviceCaps(memDC, HORZRES),
					GetDeviceCaps(memDC, VERTRES), SRCCOPY);

				SelectObject(memDC, oldBrush);
				break;
			}

			if (currentTool == ID_FILLTOOL) {

				HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hFillBrush);
				COLORREF pixelColor = GetPixel(memDC, x, y);
				ExtFloodFill(memDC, x, y, pixelColor, FLOODFILLSURFACE);
				
				StretchBlt(hdc, 0, 0,
					GetDeviceCaps(hdc, HORZRES),
					GetDeviceCaps(hdc, VERTRES),
					memDC, 0, 0,
					GetDeviceCaps(memDC, HORZRES),
					GetDeviceCaps(memDC, VERTRES), SRCCOPY);

				SelectObject(memDC, oldBrush);
				break;
			}

			SelectObject(hdc, hPen);
			SelectObject(memDC, hPen);
			if ((shapeToDraw == ID_SHAPES_POLYLINE && !isPolyLineDrawing))
			{
				ptsBegin = MAKEPOINTS(lParam);
				isPolyLineDrawing = true;
			}
			else if (shapeToDraw != ID_SHAPES_POLYLINE) {
				ptsBegin = MAKEPOINTS(lParam);
			}
			
		}
		break;

		case WM_LBUTTONUP:
		{
			clicked = false;
			UpdateClickStatusBox(clicked);

			if (currentTool == ID_FILLTOOL) {
				break;
			}

			if (shapeToDraw == ID_SHAPES_PENCIL || shapeToDraw == ID_SHAPES_LINE)
			{
				ptsEnd = MAKEPOINTS(lParam);
				MoveToEx(memDC, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
				LineTo(memDC, ptsEnd.x, ptsEnd.y);
			}
			else if (shapeToDraw == ID_SHAPES_RECTANGLE)
			{
				if (!isFill)
					SelectObject(memDC, GetStockObject(NULL_BRUSH));
				else
					SelectObject(memDC, hShapeBrush);
				ptsEnd = MAKEPOINTS(lParam);
				Rectangle(memDC, ptsBegin.x, ptsBegin.y, ptsEnd.x, ptsEnd.y);
			}
			else if (shapeToDraw == ID_SHAPES_ELLIPSE)
			{
				if (!isFill)
					SelectObject(memDC, GetStockObject(NULL_BRUSH));
				else
					SelectObject(memDC, hShapeBrush);
				ptsEnd = MAKEPOINTS(lParam);
				Ellipse(memDC, ptsBegin.x, ptsBegin.y, ptsEnd.x, ptsEnd.y);
			}
			else if (shapeToDraw == ID_SHAPES_POLYLINE)
			{
				fPrevLine = false;
				ptsEnd = MAKEPOINTS(lParam);
				MoveToEx(memDC, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
				LineTo(memDC, ptsEnd.x, ptsEnd.y);
				ptsBegin = ptsEnd;
			}
		
			fPrevLine = false;
			ClipCursor(NULL);
			ReleaseCapture();
		}
		break;

		case WM_RBUTTONDOWN:
		{
			if (shapeToDraw == ID_SHAPES_POLYLINE)
			{
				isPolyLineDrawing = false;
				SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam);
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			UpdateStatusBar(clicked, x, y);

			if (currentTool == ID_FILLTOOL) {
				break;
			}

			SelectObject(hdc, hPen);
			SelectObject(memDC, hPen);

			switch (shapeToDraw)
			{
			case ID_SHAPES_PENCIL:
			{
				if (wParam & MK_LBUTTON)
				{
					ptsEnd = MAKEPOINTS(lParam);
					drawer.drawPencil(hdc, memDC, ptsBegin, ptsEnd);
					fPrevLine = true;
					ptsBegin = ptsEnd;
				}
				break;
			}
			case ID_SHAPES_LINE:
			{
				if (wParam & MK_LBUTTON)
				{
					drawer.drawLine(hdc, memDC, ptsBegin, &ptsEnd, fPrevLine, lParam);
					fPrevLine = true;
				}
				break;
			}
			case ID_SHAPES_RECTANGLE:
			{
				if (wParam & MK_LBUTTON)
				{
					if (isFill)
						SelectObject(hdc, hShapeBrush);
					drawer.drawRectangle(hdc, memDC, ptsBegin, &ptsEnd, fPrevLine, lParam, isFill);
					fPrevLine = true;
				}
				break;
			}
			case ID_SHAPES_ELLIPSE:
			{
				if (wParam & MK_LBUTTON)
				{
					if (isFill)
						SelectObject(hdc, hShapeBrush);
					drawer.drawEllipse(hdc, memDC, ptsBegin, &ptsEnd, fPrevLine, lParam, isFill);
					fPrevLine = true;
				}
				break;
			}
			case ID_SHAPES_POLYLINE:
			{
				if (isPolyLineDrawing)
				{
					drawer.drawPolyLine(hdc, memDC, ptsBegin, &ptsEnd, fPrevLine, lParam);
					fPrevLine = true;
				}
				break;
			}	
		}
		}
		break;

		case WM_MOUSELEAVE:
		{
			clicked = false;
			UpdateClickStatusBox(clicked);
		}
		break;

		case WM_SETCURSOR:
		{
			if (currentTool == ID_PENTOOL
				&& shapeToDraw != ID_SHAPES_PENCIL) {
				SetCursor(defaultCursor);
			}
			else if (currentTool == ID_PENTOOL) {
				SetCursor(pencilCursor);
			}
			else if (currentTool == ID_ERASERTOOL) {
				SetCursor(eraserCursor);
			}
			else if (currentTool == ID_FILLTOOL) {
				SetCursor(fillCursor);
			}
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			
			StretchBlt(hdc, 0, 0, 
				GetDeviceCaps(hdc, HORZRES),
				GetDeviceCaps(hdc, VERTRES), 
				memDC, 0, 0,
				GetDeviceCaps(memDC, HORZRES), 
				GetDeviceCaps(memDC, VERTRES), SRCCOPY);


			EndPaint(hWnd, &ps);
			break;
		}
		break;


		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void MenuCommand(HWND hWnd, WPARAM param)
{
	HMENU hMenu;
	hMenu = GetMenu(hWnd);

	switch (param)
	{
	case IDM_CHOOSETOOL_PEN:
		currentTool = ID_PENTOOL;
		UpdatePen();
		SetMenuTool(IDM_CHOOSETOOL_PEN, hMenu);
		break;
	case IDM_CHOOSETOOL_ERASER:
		shapeToDraw = ID_SHAPES_PENCIL;
		SetMenuShape(shapeToDraw, hMenu);
		currentTool = ID_ERASERTOOL;
		UpdatePen();
		SetMenuTool(IDM_CHOOSETOOL_ERASER, hMenu);
		break;
	case IDM_CHOOSETOOL_FILL:
		currentTool = ID_FILLTOOL;
		SetMenuTool(IDM_CHOOSETOOL_FILL, hMenu);
		break;
	case IDM_CHOOSETOOL_FILLCOLOR:
		_ChooseColor(hWnd, areaFillColor);
		UpdateFillBrush();
		break;
	case IDM_CHOOSETOOL_GRADIENTMODE:
		isGradientMode = !isGradientMode;
		ToggleMenuGradientMode(hMenu);
		break;
	case IDM_PENSETTINGS_PENCOLOR:
		_ChooseColor(hWnd, penColor);
		UpdateFillBrush();
		UpdatePen();
		break;
	case ID_SHAPES_FILLCOLOR:
		_ChooseColor(hWnd, shapeFillColor);
		UpdateShapeBrush();
		break;
	case ID_SHAPES_FILLSHAPE:
		isFill = !isFill;
		ToggleMenuFill(hMenu);
		break;
	case ID_PENWIDTH_1PX:
		penWidth = 1;
		UpdatePen();
		SetMenuPenWidth(ID_PENWIDTH_1PX, hMenu);
		break;
	case ID_PENWIDTH_2PX:
		penWidth = 2;
		UpdatePen();
		SetMenuPenWidth(ID_PENWIDTH_2PX, hMenu);
		break;
	case ID_PENWIDTH_4PX:
		penWidth = 4;
		UpdatePen();
		SetMenuPenWidth(ID_PENWIDTH_4PX, hMenu);
		break;
	case ID_PENWIDTH_6PX:
		penWidth = 6;
		UpdatePen();
		SetMenuPenWidth(ID_PENWIDTH_6PX, hMenu);
		break;
	case ID_PENWIDTH_8PX:
		penWidth = 8;
		UpdatePen();
		SetMenuPenWidth(ID_PENWIDTH_8PX, hMenu);
		break;
	case ID_PENWIDTH_10PX:
		penWidth = 10;
		UpdatePen();
		SetMenuPenWidth(ID_PENWIDTH_10PX, hMenu);
		break;
	case ID_PENSTYLE_SOLID:
		penStyle = PS_SOLID;
		UpdatePen();
		SetMenuPenStyle(ID_PENSTYLE_SOLID, hMenu);
		break;
	case ID_PENSTYLE_DOT:
		penStyle = PS_DOT;
		UpdatePen();
		SetMenuPenStyle(ID_PENSTYLE_DOT, hMenu);
		break;
	case ID_PENSTYLE_DASH:
		penStyle = PS_DASH;
		UpdatePen();
		SetMenuPenStyle(ID_PENSTYLE_DASH, hMenu);
		break;
	case ID_PENSTYLE_DASHDOT:
		penStyle = PS_DASHDOT;
		UpdatePen();
		SetMenuPenStyle(ID_PENSTYLE_DASHDOT, hMenu);
		break;
	case ID_SHAPES_PENCIL:
		shapeToDraw = ID_SHAPES_PENCIL;
		SetMenuShape(ID_SHAPES_PENCIL, hMenu);
		break;
	case ID_SHAPES_LINE:
		shapeToDraw = ID_SHAPES_LINE;
		SetMenuShape(ID_SHAPES_LINE, hMenu);
		break;
	case ID_SHAPES_POLYLINE:
		shapeToDraw = ID_SHAPES_POLYLINE;
		SetMenuShape(ID_SHAPES_POLYLINE, hMenu);
		break;
	case ID_SHAPES_RECTANGLE:
		shapeToDraw = ID_SHAPES_RECTANGLE;
		SetMenuShape(ID_SHAPES_RECTANGLE, hMenu);
		break;
	case ID_SHAPES_ELLIPSE:
		shapeToDraw = ID_SHAPES_ELLIPSE;
		SetMenuShape(ID_SHAPES_ELLIPSE, hMenu);
		break;
	case IDM_CLEARSCREEN:
	{
		RECT rect = { 0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES) };
		InvalidateRect(hWnd, 0, true);
		FillRect(memDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
		UpdateWindow(hWnd);
	}
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	default:
		break;
	}
}

void SetMenuTool(int tool, HMENU hMenu)
{
	static int previousTool = IDM_CHOOSETOOL_PEN;

	if (previousTool != tool) 
	{
		CheckMenuItem(hMenu, tool, MF_CHECKED);
		CheckMenuItem(hMenu, previousTool, MF_UNCHECKED);
		previousTool = tool;
	}
}

void SetMenuPenStyle(int penStyle, HMENU hMenu)
{
	static int previousPenStyle = ID_PENSTYLE_SOLID;

	if (previousPenStyle != penStyle) 
	{
		CheckMenuItem(hMenu, penStyle, MF_CHECKED);
		CheckMenuItem(hMenu, previousPenStyle, MF_UNCHECKED);
		previousPenStyle = penStyle;
	}
}

void SetMenuPenWidth(int penWidth, HMENU hMenu)
{
	static int previousPenWidth = ID_PENWIDTH_2PX;

	if (previousPenWidth != penWidth)
	{
		CheckMenuItem(hMenu, penWidth, MF_CHECKED);
		CheckMenuItem(hMenu, previousPenWidth, MF_UNCHECKED);
		previousPenWidth = penWidth;
	}
}

void SetMenuShape(int shape, HMENU hMenu)
{
	static int previousShape = ID_SHAPES_PENCIL;
	
	if (previousShape != shape) 
	{
		CheckMenuItem(hMenu, shape, MF_CHECKED);
		CheckMenuItem(hMenu, previousShape, MF_UNCHECKED);
		previousShape = shape;
	}
}

void ToggleMenuGradientMode(HMENU hMenu) 
{
	if (isGradientMode == true) {
		CheckMenuItem(hMenu, IDM_CHOOSETOOL_GRADIENTMODE, MF_CHECKED);
	}
	else {
		CheckMenuItem(hMenu, IDM_CHOOSETOOL_GRADIENTMODE, MF_UNCHECKED);
	}
}

void ToggleMenuFill(HMENU hMenu)
{
	if (isFill == true) {
		CheckMenuItem(hMenu, ID_SHAPES_FILLSHAPE, MF_CHECKED);
	}
	else {
		CheckMenuItem(hMenu, ID_SHAPES_FILLSHAPE, MF_UNCHECKED);
	}
}

void UpdatePen()
{

	DeleteObject(hPen);
	DeleteObject(eraser);

	// as well updating eraser width
	eraser = CreatePen(PS_SOLID, penWidth, eraserColor);

	lb.lbColor = penColor;
	hPen = ExtCreatePen(PS_GEOMETRIC | penStyle, penWidth, &lb, 0, NULL);

	if (currentTool == ID_ERASERTOOL) {
		hPen = eraser;
	}

	UpdatePenStatusBox();
}

void UpdateShapeBrush()
{
	DeleteObject(hShapeBrush);

	hShapeBrush = CreateSolidBrush(shapeFillColor);
}

void UpdateFillBrush()
{
	DeleteObject(hFillBrush);

	hFillBrush = CreateSolidBrush(areaFillColor);
}

void _ChooseColor(HWND hWnd, COLORREF &color)
{
	CHOOSECOLOR chooseColor;
	static COLORREF choosenColors[16];
	static DWORD rgbCurrent;

	ZeroMemory(&chooseColor, sizeof(CHOOSECOLOR));
	chooseColor.lStructSize = sizeof(CHOOSECOLOR);
	chooseColor.hwndOwner = hWnd;
	chooseColor.lpCustColors = choosenColors;
	chooseColor.rgbResult = rgbCurrent;

	chooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;


	if (ChooseColor(&chooseColor) == TRUE)
	{
		color = chooseColor.rgbResult;
	}
}

void UpdateStatusBar(bool clicked, int x, int y)
{
	UpdateClickStatusBox(clicked);
	UpdateCoordStatusBox(x, y);
	UpdatePenStatusBox();
}

void UpdateClickStatusBox(bool clickStatus)
{
	std::wstring st1 = L"clicked: " + std::to_wstring(clickStatus);
	SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)st1.c_str());
}

void UpdateCoordStatusBox(int x, int y) 
{
	std::wstring st2 = L"x: " + std::to_wstring(x) + L"; y: " + std::to_wstring(y);
	SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)st2.c_str());
}

void UpdatePenStatusBox()
{
	std::wstring st = L"width: " + std::to_wstring(penWidth) + L";    style: ";

	switch (penStyle) {
	case PS_SOLID:
		st += L"Solid";
		break;
	case PS_DOT:
		st += L"Dot";
		break;
	case PS_DASH:
		st += L"Dash";
		break;
	case PS_DASHDOT:
		st += L"Dash-Dot";
		break;
	default:
		st += L"Other style";
	}

	SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM)st.c_str());
}



void GradientFloodFill(HDC dc, int dcWidth, int dcHeight, int x, int y, COLORREF newColor)
{
	
	Gdiplus::Graphics gradientGraphics(gradientDC);

	GraphicsPath path;
	Rect clientRect(0, 0, dcWidth, dcHeight);
	path.AddRectangle(clientRect);
	PathGradientBrush pthGrBrush(&path);

	Color pathCenterColor;
	pathCenterColor.SetFromCOLORREF(areaFillColor);
	pthGrBrush.SetCenterColor(pathCenterColor);

	Color colors[] = { Color(Color::White)};

	int count = 1;
	pthGrBrush.SetSurroundColors(colors, &count);

	gradientGraphics.FillRectangle(&pthGrBrush, 0, 0, dcWidth, dcHeight);





	int *visitedPixels = new int[dcWidth * dcHeight];
	std::fill(visitedPixels, visitedPixels + dcWidth * dcHeight, 0);

	std::queue<std::pair<int, int>> pixelQueue;

	COLORREF oldColor = GetPixel(dc, x, y);

	pixelQueue.push({x, y});
	visitedPixels[x*dcHeight + dcWidth] = 1;

	while (pixelQueue.empty() != 1)
	{
		std::pair<int, int> coordinate = pixelQueue.front();
		int x = coordinate.first;
		int y = coordinate.second;

		COLORREF gradientPixelColor = GetPixel(gradientDC, x, y);
		SetPixel(dc, x, y, gradientPixelColor);
		pixelQueue.pop();

		if (isCoordinateValid(x + 1, y, dcWidth, dcHeight)
			&& visitedPixels[(x + 1) * dcHeight + y] == 0
			&& GetPixel(dc, x + 1, y) == oldColor)
		{
			pixelQueue.push({ x + 1, y });
			visitedPixels[(x + 1) * dcHeight + y] = 1;
		}

		if (isCoordinateValid(x - 1, y, dcWidth, dcHeight)
			&& visitedPixels[(x - 1) * dcHeight + y] == 0
			&& GetPixel(dc, x - 1, y) == oldColor)
		{
			pixelQueue.push({ x - 1, y });
			visitedPixels[(x - 1) * dcHeight + y] = 1;
		}

		if (isCoordinateValid(x, y + 1, dcWidth, dcHeight)
			&& visitedPixels[x * dcHeight + (y + 1)] == 0
			&& GetPixel(dc, x, y + 1) == oldColor)
		{
			pixelQueue.push({ x, y + 1 });
			visitedPixels[x * dcHeight + (y + 1)] = 1;
		}

		if (isCoordinateValid(x, y - 1, dcWidth, dcHeight)
			&& visitedPixels[x * dcHeight + (y - 1)] == 0
			&& GetPixel(dc, x, y - 1) == oldColor)
		{
			pixelQueue.push({ x, y - 1 });
			visitedPixels[x * dcHeight + (y - 1)] = 1;
		}
	}

	delete[] visitedPixels;
}

bool isCoordinateValid(int x, int y, int dcWidth, int dcHeight)
{
	if (x < 0 || y < 0) {
		return false;
	}
	if (x >= dcWidth || y >= dcHeight) {
		return false;
	}
	return true;
}