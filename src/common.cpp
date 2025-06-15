#include "../include/common.h"
#include <cmath>

int Common::Round(double x)
{
    return (int)(x + 0.5);
}

COLORREF Common::interpolateColors(COLORREF c1, COLORREF c2, double t)
{
    int r = Round(GetRValue(c1) * t + (1 - t) * GetRValue(c2));
    int g = Round(GetGValue(c1) * t + (1 - t) * GetGValue(c2));
    int b = Round(GetBValue(c1) * t + (1 - t) * GetBValue(c2));
    return RGB(r, g, b);
}

std::vector<int> Common::matrixMult(std::vector<std::vector<int>> m1, std::vector<int> m2)
{
    int rows = m1.size();
    int shared = m1[0].size(); // should be equal to m2.size()
    int cols = m2.size();

    std::vector<int> result(rows, 0);

    for (int i = 0; i < rows; ++i)
    {
        // for (int k = 0; k < cols; ++k) {
        for (int j = 0; j < shared; ++j)
        {
            result[i] += m1[i][j] * m2[j];
        }
        //}
    }

    return result;
}

bool Common::isValidPolygon(const std::vector<POINT>& points) {
    if (points.size() < 2) return true; // Need at least two points to check distance

    POINT newPoint = points.back();
    // Check distance to all other points except itself
    for (size_t i = 0; i < points.size() - 1; ++i) {
        double distance = std::hypot(newPoint.x - points[i].x, newPoint.y - points[i].y);
        if (distance < 5) { // Threshold for minimum distance
            return false;
        }
    }
    return true;
}

bool Common::IsConvex(const std::vector<POINT>& points) {
    if (points.size() < 3) return false;

    // Calculate cross products of consecutive edges
    int n = points.size();
    int sign = 0;  // 0 means we haven't determined the sign yet

    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (i + 2) % n;

        // Calculate vectors
        int dx1 = points[j].x - points[i].x;
        int dy1 = points[j].y - points[i].y;
        int dx2 = points[k].x - points[j].x;
        int dy2 = points[k].y - points[j].y;

        // Calculate cross product
        int cross = dx1 * dy2 - dy1 * dx2;

        // Skip if cross product is zero (collinear points)
        if (cross == 0) continue;

        // Determine sign
        if (sign == 0) {
            sign = (cross > 0) ? 1 : -1;
        }
        // If sign changes, polygon is not convex
        else if ((cross > 0 && sign < 0) || (cross < 0 && sign > 0)) {
            return false;
        }
    }

    return true;
}