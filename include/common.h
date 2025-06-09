// Header for common.cpp
#pragma once

#include <windows.h>
#include <vector>
#include <cmath>

// Define operator== for POINT structure
inline bool operator==(const POINT& a, const POINT& b) {
    return a.x == b.x && a.y == b.y;
}

class Common {
public:
    static int Round(double x);
    static COLORREF interpolateColors(COLORREF c1, COLORREF c2, double t);
    static std::vector<int> matrixMult(std::vector<std::vector<int>> m1, std::vector<int> m2);
    static bool isValidPolygon(const std::vector<POINT>& points);
};
