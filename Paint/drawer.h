#pragma once
#include <Windows.h>

class Drawer
{
public:
	Drawer();
	~Drawer();

	void drawPencil(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS ptsEnd);
	void drawLine(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam);
	void drawRectangle(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam, bool isFill);
	void drawEllipse(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam, bool isFill);
	void drawPolyLine(HDC hdc, HDC memDC, POINTS ptsBegin, POINTS* ptsEnd, bool fPrevLine, LPARAM lParam);
};