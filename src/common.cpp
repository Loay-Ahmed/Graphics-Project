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