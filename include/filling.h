// Header for filling.cpp
#pragma once
#include <windows.h>
#include <queue>

class Filling {
public:
    static void RecFloodFill(HDC hdc, int x, int y, COLORREF c);
    static void NonRecFloodFill(HDC hdc, int x, int y, COLORREF c);
    static void BaryCentric(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF c);
};
