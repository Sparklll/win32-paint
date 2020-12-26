
#include <Windows.h>
#include "stdafx.h"
#include "drawer.h"


Drawer::Drawer()
{
}


Drawer::~Drawer()
{
}

void Drawer::drawPencil(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS ptsEnd)
{
	MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
	LineTo(hdc, ptsEnd.x, ptsEnd.y);

	MoveToEx(memDC, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
	LineTo(memDC, ptsEnd.x, ptsEnd.y);
}

void Drawer::drawLine(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam)
{
	if (fPrevLine)
	{
		MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
		LineTo(hdc, ptsEnd->x, ptsEnd->y);
		StretchBlt(hdc, 0, 0, 
			GetDeviceCaps(hdc, HORZRES),
			GetDeviceCaps(hdc, VERTRES), 
			memDC, 0, 0,
			GetDeviceCaps(memDC, HORZRES), 
			GetDeviceCaps(memDC, VERTRES), SRCCOPY);
	}
	*ptsEnd = MAKEPOINTS(lParam);

	MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
	LineTo(hdc, ptsEnd->x, ptsEnd->y);
}

void Drawer::drawRectangle(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam, bool isFill)
{
	if (fPrevLine)
	{
		Rectangle(hdc, ptsBegin.x, ptsBegin.y, ptsEnd->x, ptsEnd->y);
		StretchBlt(hdc, 0, 0, 
			GetDeviceCaps(hdc, HORZRES),
			GetDeviceCaps(hdc, VERTRES), 
			memDC, 0, 0,
			GetDeviceCaps(memDC, HORZRES),
			GetDeviceCaps(memDC, VERTRES), SRCCOPY);
	}
	*ptsEnd = MAKEPOINTS(lParam);
	if (!isFill)
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, ptsBegin.x, ptsBegin.y, ptsEnd->x, ptsEnd->y);
}

void Drawer::drawEllipse(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam, bool isFill)
{
	if (fPrevLine)
	{
		Ellipse(hdc, ptsBegin.x, ptsBegin.y, ptsEnd->x, ptsEnd->y);
		StretchBlt(hdc, 0, 0, 
			GetDeviceCaps(hdc, HORZRES),
			GetDeviceCaps(hdc, VERTRES), 
			memDC, 0, 0,
			GetDeviceCaps(memDC, HORZRES), 
			GetDeviceCaps(memDC, VERTRES), SRCCOPY);
	}
	*ptsEnd = MAKEPOINTS(lParam);
	if (!isFill)
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Ellipse(hdc, ptsBegin.x, ptsBegin.y, ptsEnd->x, ptsEnd->y);
}

void Drawer::drawPolyLine(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam)
{
	if (fPrevLine)
	{
		MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
		LineTo(hdc, ptsEnd->x, ptsEnd->y);
		StretchBlt(hdc, 0, 0, 
			GetDeviceCaps(hdc, HORZRES),
			GetDeviceCaps(hdc, VERTRES), 
			memDC, 0, 0,
			GetDeviceCaps(memDC, HORZRES), 
			GetDeviceCaps(memDC, VERTRES), SRCCOPY);
	}
	*ptsEnd = MAKEPOINTS(lParam);

	MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
	LineTo(hdc, ptsEnd->x, ptsEnd->y);
}