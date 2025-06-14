#include "../include/curves_third_degree.h"
#include "../include/common.h"
#include <vector>
using namespace std;

void ThirdDegreeCurve::HermiteCurve(HDC hdc, int x1, int y1, int u1, int v1, int x2, int y2, int u2, int v2, COLORREF c) {
    vector<vector<int>> H = {
        {2, -2, 1, 1},
        {-3, 3, -2, -1},
        {0, 0, 1, 0},
        {1, 0, 0, 0}};
    vector<int> Gx = {x1, x2, u1, u2};
    vector<int> Gy = {y1, y2, v1, v2};
    vector<int> Cx = Common::matrixMult(H, Gx);
    vector<int> Cy = Common::matrixMult(H, Gy);
    for (double t = 0; t <= 1.0; t += 0.0001) {
        double t2 = t * t;
        double t3 = t2 * t;
        int x = Common::Round(Cx[0] * t3 + Cx[1] * t2 + Cx[2] * t + Cx[3]);
        int y = Common::Round(Cy[0] * t3 + Cy[1] * t2 + Cy[2] * t + Cy[3]);
        SetPixel(hdc, x, y, c);
    }
}

void ThirdDegreeCurve::BezierCurve(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, COLORREF c) {
    vector<vector<int>> H = {
        {-1, 3, -3, 1},
        {3, -6, 3, 0},
        {-3, 3, 0, 0},
        {1, 0, 0, 0}};
    vector<int> Gx = {x1, x2, x3, x4};
    vector<int> Gy = {y1, y2, y3, y4};
    vector<int> Cx = Common::matrixMult(H, Gx);
    vector<int> Cy = Common::matrixMult(H, Gy);
    for (double t = 0; t <= 1.0; t += 0.0001) {
        double t2 = t * t;
        double t3 = t2 * t;
        int x = Common::Round(Cx[0] * t3 + Cx[1] * t2 + Cx[2] * t + Cx[3]);
        int y = Common::Round(Cy[0] * t3 + Cy[1] * t2 + Cy[2] * t + Cy[3]);
        SetPixel(hdc, x, y, c);
    }
}

vector<double> ThirdDegreeCurve::Bezier(vector<double> points, double t) {
    if (points.size() == 2)
        return {points[0], points[1]};
    vector<double> newPoints;
    for (int i = 0; i < points.size() - 2; i += 2) {
        double x = (1 - t) * points[i] + t * points[i + 2];
        double y = (1 - t) * points[i + 1] + t * points[i + 3];
        newPoints.push_back(x);
        newPoints.push_back(y);
    }
    return Bezier(newPoints, t);
}

void ThirdDegreeCurve::RecBezier(HDC hdc, vector<double> points, COLORREF c) {
    for (double t = 0; t < 1; t += 0.00005) {
        vector<double> point = Bezier(points, t);
        SetPixel(hdc, point[0], point[1], c);
    }
}

void ThirdDegreeCurve::CardinalSplines(HDC hdc, vector<double> points, int C, COLORREF color)
{
    int n = points.size() / 2;
    if (n < 4) return; // Need at least 4 points

    // Calculate tangents
    vector<double> qx, qy;
    for (int i = 0; i < n; i++) {
        if (i == 0 || i == n - 1) {
            qx.push_back(0);
            qy.push_back(0);
        } else {
            qx.push_back(C * (points[2 * (i + 1)] - points[2 * (i - 1)]) / 2.0);
            qy.push_back(C * (points[2 * (i + 1) + 1] - points[2 * (i - 1) + 1]) / 2.0);
        }
    }
    // Draw Hermite curves between each pair of points
    for (int i = 1; i < n - 2; i++) {
        HermiteCurve(
            hdc,
            (int)points[2 * i], (int)points[2 * i + 1], (int)qx[i], (int)qy[i],
            (int)points[2 * (i + 1)], (int)points[2 * (i + 1) + 1], (int)qx[i + 1], (int)qy[i + 1],
            color
        );
    }
}