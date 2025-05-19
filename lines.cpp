#include "import.h"

class Lines
{
    static int Round(double x)
    {
        return (int)(x + 0.5);
    }
    // Interpolate colors
    static COLORREF interpolateColors(COLORREF c1, COLORREF c2, double t)
    {
        int r = Round(GetRValue(c1) * t + (1 - t) * GetRValue(c2));
        int g = Round(GetGValue(c1) * t + (1 - t) * GetGValue(c2));
        int b = Round(GetBValue(c1) * t + (1 - t) * GetBValue(c2));
        return RGB(r, g, b);
    }

public:
    void InterpolatedColoredLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c1, COLORREF c2)
    {
        int dx = x2 - x1;
        int dy = y2 - y1;
        double steps = 1.0 / max(abs(dx), abs(dy));

        for (double t = 0; t <= 1; t += steps)
        {
            COLORREF c = interpolateColors(c1, c2, t);
            SetPixel(hdc, Round(dx * t + x1), Round(dy * t + y1), c);
        }
    }

    // Line Bresenham using midpoint algo
    void LineBresenham(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
    {
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);

        int sx = (x2 > x1) ? 1 : -1;
        int sy = (y2 > y1) ? 1 : -1;

        int x = x1;
        int y = y1;

        SetPixel(hdc, x, y, c);

        if (dx > dy)
        {
            int d = 2 * dy - dx;
            int dE = 2 * dy;
            int dNE = 2 * (dy - dx);

            while (x != x2)
            {
                x += sx;
                if (d < 0)
                {
                    d += dE;
                }
                else
                {
                    y += sy;
                    d += dNE;
                }
                SetPixel(hdc, x, y, c);
            }
        }
        else
        {
            int d = 2 * dx - dy;
            int dE = 2 * dx;
            int dNE = 2 * (dx - dy);

            while (y != y2)
            {
                y += sy;
                if (d < 0)
                {
                    d += dE;
                }
                else
                {
                    x += sx;
                    d += dNE;
                }
                SetPixel(hdc, x, y, c);
            }
        }
    }

    // Draws line by midpoint as binary search algorithm
    void DrawLineByMidPoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
    {
        SetPixel(hdc, x1, y1, c);
        SetPixel(hdc, x2, y2, c);

        int avgX = Round((x1 + x2) / 2);
        int avgY = Round((y1 + y2) / 2);

        for (int i = 0; i < 100000; i++)
        {
            i++;
            i--;
        }

        if ((x1 == avgX && y1 == avgY) || (x2 == avgX && avgY == y2))
        {
            return;
        }
        DrawLineByMidPoint(hdc, x1, y1, avgX, avgY, c);
        SetPixel(hdc, avgX, avgY, c);
        DrawLineByMidPoint(hdc, avgX, avgY, x2, y2, c);
    }
};