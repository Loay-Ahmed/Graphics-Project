// Header for common.cpp
#pragma once

#include <windows.h>
#include <vector>
#include <cmath>
#include <map>

// Define operator== for POINT structure
inline bool operator==(const POINT& a, const POINT& b) {
    return a.x == b.x && a.y == b.y;
}

// Drawing Algorithm Types
// These enums define the algorithms used for drawing lines, circles, ellipses, and filling
// They are used to select the appropriate algorithm based on user input or menu selection
enum LineAlgorithm { LINE_DDA, LINE_MIDPOINT, LINE_PARAMETRIC };
enum CircleAlgorithm { CIRCLE_DIRECT, CIRCLE_POLAR, CIRCLE_ITER_POLAR, CIRCLE_MIDPOINT, CIRCLE_MOD_MIDPOINT };
enum EllipseAlgorithm { ELLIPSE_DIRECT, ELLIPSE_POLAR, ELLIPSE_MIDPOINT };
enum FillAlgorithm { FILL_RECURSIVE_FLOOD, FILL_NONRECURSIVE_FLOOD, FILL_CONVEX, FILL_NONCONVEX };

enum ClippingWindowType {
    CLIP_RECTANGLE,
    CLIP_SQUARE
};

class Common {
public:
    static int Round(double x);
    static COLORREF interpolateColors(COLORREF c1, COLORREF c2, double t);
    static std::vector<int> matrixMult(std::vector<std::vector<int>> m1, std::vector<int> m2);
    static bool isValidPolygon(const std::vector<POINT>& points);
    static bool IsConvex(const std::vector<POINT>& points);
    static std::map<std::pair<int, int>, COLORREF> drawings;
};
