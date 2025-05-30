#include "import.h"

class SecondDegreeCurve
{
private:
    // Draw 8 points of the circle
    static void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c)
    {
        int dx[8] = {x, -x, x, -x, y, -y, y, -y};
        int dy[8] = {y, y, -y, -y, x, x, -x, -x};
        for (int i = 0; i < 8; i++)
        {
            SetPixel(hdc, xc + dx[i], yc + dy[i], c);
        }
    }

public:
    // Draw circle using the Common::Rounding algorithm
    static void DrawCircle(HDC hdc, int xc, int yc, int x2, int y2, COLORREF c)
    {
        int r = Common::Round(sqrt(abs((xc - x2) * (xc - x2) + (yc - y2) * (yc - y2))));
        for (int i = xc; i <= xc + r; i++)
        {
            int x = xc - i;
            int y = Common::Round(sqrt(r * r - x * x));
            Draw8Points(hdc, xc, yc, x, y, c);
        }
    }

    // Draw circle using the Bresenham (midpoint) algorithm
    static pair<int, int> BresenhamCircle(HDC hdc, int xc, int yc, int r, COLORREF c)
    {
        int x2 = 0, y2 = r;
        Draw8Points(hdc, xc, yc, x2, y2, c);
        while (x2 < y2)
        {
            int d = Common::Round(pow(x2 + 1, 2) + pow(y2 - 0.5, 2) - pow(r, 2));
            x2++;
            if (d > 0)
            {
                y2--;
            }
            Draw8Points(hdc, xc, yc, x2, y2, c);
        }
        return {x2, y2};
    }
    // Draw line with interpolated color

    // Draw line with interpolated color
    static void InterpolatedColoredCurve(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF c1, COLORREF c2)
    {
        int alphaX = 2 * x3 + 2 * x1 - 4 * x2;
        int betaX = 4 * x2 - 3 * x1 - x3;
        int alphaY = 2 * y3 + 2 * y1 - 4 * y2;
        int betaY = 4 * y2 - 3 * y1 - y3;
        std::cout << "calc alphas\n";
        double steps = 1.0 / 100000;

        for (double t = 0; t <= 1; t += steps)
        {
            std::cout << "in for\n";
            COLORREF c = Common::interpolateColors(c1, c2, t);
            SetPixel(hdc, Common::Round(alphaX * t * t + betaX * t + x1), Common::Round(alphaY * t * t + betaY * t + y1), c);
        }
    }
};