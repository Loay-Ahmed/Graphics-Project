// Header for lines.cpp
#pragma once
#include <windows.h>

class Lines {
public:
    static void InterpolatedColoredLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c1, COLORREF c2);
    static void LineBresenhamDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c);
    static void DrawLineByMidPoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c);
};
