// Header for common.cpp
#pragma once

#include <windows.h>
#include <vector>

class Common {
public:
    static int Round(double x);
    static COLORREF interpolateColors(COLORREF c1, COLORREF c2, double t);
    static std::vector<int> matrixMult(std::vector<std::vector<int>> m1, std::vector<int> m2);
};
