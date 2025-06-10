#pragma once
#include <windows.h>
#include <list>
#include <queue>
#include <vector>
#include <algorithm>
#include "../include/common.h"
#include "../include/lines.h"

class Filling {
public:
    // Point structure for both convex and non-convex filling
    struct Point {
        double x, y;
        Point() : x(0), y(0) {}
        Point(double x, double y) : x(x), y(y) {}
        Point(const POINT& p) : x(p.x), y(p.y) {}
        operator POINT() const { return {static_cast<LONG>(x), static_cast<LONG>(y)}; }
    };

    // Flood Fill Methods
    static void RecursiveFloodFill(HDC hdc, int x, int y, COLORREF color);
    static void NonRecursiveFloodFill(HDC hdc, int x, int y, COLORREF color);

    // Barycentric Fill Method
    static void BarycentricFill(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color);

    // Convex Fill Methods
    static void ConvexFill(HDC hdc, const std::vector<Point>& points, COLORREF color);
    static void ConvexFill(HDC hdc, const std::vector<POINT>& points, COLORREF color);

    // Non-Convex Fill Methods
    static void NonConvexFill(HDC hdc, const std::vector<Point>& points, COLORREF color);
    static void NonConvexFill(HDC hdc, const std::vector<POINT>& points, COLORREF color);

    // --- Extra menu ---
    static void FillQuarterWithSmallCircles(HDC hdc, int xc, int yc, int R, int quarter, COLORREF c);
    static void FillRectangleWithBezierWaves(HDC hdc, int left, int top, int right, int bottom, COLORREF c);

private:
    // Edge table structures
    struct ConvexEdgeTable {
        int xleft, xright;
        ConvexEdgeTable() : xleft(10000), xright(-10000) {}
    };
    typedef std::list<int> NonConvexEdgeTable[800];
    typedef ConvexEdgeTable ConvexEdgeTableArray[800];

    // Helper methods for convex filling
    static void InitConvexEdgeTable(ConvexEdgeTableArray& table);
    static void EdgeToConvexTable(const Point& v1, const Point& v2, ConvexEdgeTableArray& table);
    static void PolygonToConvexTable(const std::vector<Point>& points, ConvexEdgeTableArray& table);
    static void ConvexTableToScreen(HDC hdc, const ConvexEdgeTableArray& table, COLORREF color);

    // Helper methods for non-convex filling
    static void InitNonConvexEdgeTable(NonConvexEdgeTable& table);
    static void EdgeToNonConvexTable(const Point& v1, const Point& v2, NonConvexEdgeTable& table);
    static void PolygonToNonConvexTable(const std::vector<Point>& points, NonConvexEdgeTable& table);
    static void NonConvexTableToScreen(HDC hdc, const NonConvexEdgeTable& table, COLORREF color);

    // Utility methods
    static std::vector<Point> ConvertToPoints(const std::vector<POINT>& points);
};