#include "../include/tasks_and_assignments.h"
#include "../include/lines.h"
#include "../include/curves_second_degree.h"
#include "../include/common.h"
#include <vector>
using namespace std;

void TasksAndAssignments::pizzaCircle(HDC hdc, int xc, int yc, int r, COLORREF c)
{
    int x = 0, y = r;

    // Draw line vertical and horizontal
    Lines::LineBresenhamDDA(hdc, xc - r, yc, xc + r, yc, c);
    Lines::LineBresenhamDDA(hdc, xc, yc - r, xc, yc + r, c);

    // Draw circle by bresenham
    pair<int, int> point = SecondDegreeCurve::BresenhamCircle(hdc, xc, yc, r, c);
    x = point.first;
    y = point.second;

    Lines::LineBresenhamDDA(hdc, xc - x, yc - y, xc + x, yc + y, c);
    Lines::LineBresenhamDDA(hdc, xc + x, yc - y, xc - x, yc + y, c);
}

void TasksAndAssignments::BezierInterpolatedCurve(HDC hdc, int x1, int y1, COLORREF c1, int x2, int y2, COLORREF c2, int x3, int y3, COLORREF c3, int x4, int y4, COLORREF c4)
{
    vector<vector<int>> H = {
        {-1, 3, -3, 1},
        {3, -6, 3, 0},
        {-3, 3, 0, 0},
        {1, 0, 0, 0}};

    vector<int> Gx = {x1, x2, x3, x4}; // 4x1
    vector<int> Gy = {y1, y2, y3, y4}; // 4x1

    // Multiply H (4x4) * Gx/Gy (4x1) => 4x1
    vector<int> Cx = Common::matrixMult(H, Gx);
    vector<int> Cy = Common::matrixMult(H, Gy);

    for (double t = 0; t <= 1.0; t += 0.0001)
    {
        double t2 = t * t;
        double t3 = t2 * t;
        int x = Common::Round(Cx[0] * t3 + Cx[1] * t2 + Cx[2] * t + Cx[3]);
        int y = Common::Round(Cy[0] * t3 + Cy[1] * t2 + Cy[2] * t + Cy[3]);
        COLORREF c;
        if (t < 1.0 / 3)
        {
            c = Common::interpolateColors(c1, c2, t);
        }
        else if (t > 1.0 / 3 && t < 2.0 / 3)
        {
            c = Common::interpolateColors(c2, c3, t);
        }
        else if (t > 2.0 / 3 && t < 1.0)
        {
            c = Common::interpolateColors(c3, c4, t);
        }
        SetPixel(hdc, x, y, c);
    }
}