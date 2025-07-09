// Header for curves_second_degree.cpp
#pragma once
#include <windows.h>
#include <utility>

class SecondDegreeCurve {
public:
    static void DrawCircle(HDC hdc, int xc, int yc, int x2, int y2, COLORREF c);
    static std::pair<int, int> BresenhamCircle(HDC hdc, int xc, int yc, int r, COLORREF c);
    static void InterpolatedColoredCurve(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF c1, COLORREF c2);
    static void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c);
    static void itreativepolar(HDC hdc, int xc, int yc, int r, COLORREF c);
    static void directcircle(HDC hdc, int xc, int yc, int r, COLORREF c);
    static void ModfiedBresenhamcircle(HDC hdc, int xc, int yc, int r, COLORREF c);
};
