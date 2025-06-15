// Header for curves_third_degree.cpp
#pragma once
#include <windows.h>
#include <vector>

class ThirdDegreeCurve {
public:
    static void HermiteCurve(HDC hdc, int x1, int y1, int u1, int v1, int x2, int y2, int u2, int v2, COLORREF c);
    static void BezierCurve(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, COLORREF c);
    static std::vector<double> Bezier(std::vector<double> points, double t);
    static void RecBezier(HDC hdc, std::vector<double> points, COLORREF c);
    static void CardinalSplines(HDC hdc, std::vector<double> points, int C, COLORREF color);
};
