#pragma once
#include <windows.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


class Ellipse {
public:
    static void DrawEllipseEquation(HDC hdc, int xc, int yc, int a, int b, COLORREF c);
    static void DrawEllipseMidPoint(HDC hdc, int xc, int yc, int a, int b, COLORREF c);
    static void DrawEllipsePolar(HDC hdc, int xc, int yc, int a, int b, COLORREF c);
    
};
