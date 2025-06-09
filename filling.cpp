#include "import.h"
#include "common.cpp"
#include "lines.cpp"
#include "curves_second_degree.cpp"
#include "curves_third_degree.cpp"
using namespace std;
class Filling
{

public:
    static void RecFloodFill(HDC hdc, int x, int y, COLORREF c)
    {
        COLORREF p = GetPixel(hdc, x, y);

        if (p == c)
            return;

        SetPixel(hdc, x, y, c);

        RecFloodFill(hdc, x + 1, y, c);
        RecFloodFill(hdc, x, y + 1, c);
        RecFloodFill(hdc, x - 1, y, c);
        RecFloodFill(hdc, x, y - 1, c);
    }
    struct point
    {
        int x, y;
        point(int x, int y) : x(x), y(y) {}
    };

    static void NonRecFloodFill(HDC hdc, int x, int y, COLORREF c)
    {
        static int dx[4] = {1, -1, 0, 0};
        static int dy[4] = {0, 0, 1, -1};
        queue<point> st;

        st.push(point(x, y));

        while (!st.empty())
        {
            point p = st.front();
            st.pop();
            COLORREF c1 = GetPixel(hdc, p.x, p.y);
            if (c1 == c)
                continue;
            SetPixel(hdc, p.x, p.y, c);
            for (int i = 0; i < 4; i++)
                st.push(point(p.x + dx[i], p.y + dy[i]));
        }
    }

    static void BaryCentric(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF c)
    {

        for (double t1 = 0; t1 < 1; t1 += 0.001)
        {
            for (double t2 = 0; t2 < 1 - t1; t2 += 0.001)
            {
                int x = Common::Round(t1 * x1 + t2 * x2 + (1 - t1 - t2) * x3);
                int y = Common::Round(t1 * y1 + t2 * y2 + (1 - t1 - t2) * y3);
                SetPixel(hdc, x, y, c);
            }
        }
    }
    static void FillCircleQuarter(HDC hdc, int xc, int yc, int radius, int quarter, COLORREF c)
    {
        // Clamp quarter input between 1 and 4
        if (quarter < 1 || quarter > 4) return;

        int startAngleDeg = 0, endAngleDeg = 0;

        switch (quarter)
        {
            case 1: // top-right quarter (0 to 90 degrees)
                startAngleDeg = 0;
                endAngleDeg = 90;
                break;
            case 2: // top-left quarter (90 to 180 degrees)
                startAngleDeg = 90;
                endAngleDeg = 180;
                break;
            case 3: // bottom-left quarter (180 to 270 degrees)
                startAngleDeg = 180;
                endAngleDeg = 270;
                break;
            case 4: // bottom-right quarter (270 to 360 degrees)
                startAngleDeg = 270;
                endAngleDeg = 360;
                break;
        }

        // Draw lines from center to circumference at small angle increments in the quarter
        for (double angleDeg = startAngleDeg; angleDeg <= endAngleDeg; angleDeg += 0.5)
        {
            double angleRad = angleDeg * 3.14159265358979323846 / 180.0;

            int xEnd = xc + (int)(radius * cos(angleRad));
            int yEnd = yc - (int)(radius * sin(angleRad)); // y axis inverted in GDI

            // Draw line from center to circumference point
            // Use existing line drawing function or just SetPixel in a loop

            // For simplicity, use your Lines::DrawLineByMidPoint or LineBresenhamDDA
            Lines::LineBresenhamDDA(hdc, xc, yc, xEnd, yEnd, c);
        }
    }


    static void FillQuarterWithSmallCircles(HDC hdc, int xc, int yc, int R, int quarter, COLORREF c)
    {
        const int maxRadius = 4;
        const int minRadius = 1;

        for (int y = -R; y <= R; y += 2 * maxRadius)
        {
            for (int x = -R; x <= R; x += 2 * maxRadius)
            {
                double distSquared = x * x + y * y;
                if (distSquared <= R * R)
                {
                    bool inQuarter = false;
                    switch (quarter)
                    {
                        case 1: inQuarter = (x >= 0 && y <= 0); break;
                        case 2: inQuarter = (x <= 0 && y <= 0); break;
                        case 3: inQuarter = (x <= 0 && y >= 0); break;
                        case 4: inQuarter = (x >= 0 && y >= 0); break;
                    }

                    if (inQuarter)
                    {
                        double dist = sqrt(distSquared);
                        double ratio = dist / R;
                        int rSmall = static_cast<int>(minRadius + (1.0 - ratio) * (maxRadius - minRadius));

                        if (rSmall < 1) rSmall = 1;

                        SecondDegreeCurve::BresenhamCircle(hdc, xc + x, yc + y, rSmall, c);
                    }
                }
            }
        }
    }

    static void FillRectangleWithBezierWaves(HDC hdc, int left, int top, int right, int bottom, COLORREF c)
    {
        int waveHeight = 15;   // bigger amplitude to overlap vertically
        int waveLength = 40;   // length of wave cycle
        int stepY = 6;         // smaller vertical step for denser waves

        for (int y = top; y <= bottom; y += stepY)
        {
            for (int x = left; x < right; x += waveLength)
            {
                int x1 = x;
                int y1 = y;
                int x2 = x + waveLength / 4;
                int y2 = y - waveHeight;
                int x3 = x + 3 * waveLength / 4;
                int y3 = y + waveHeight;
                int x4 = x + waveLength;
                int y4 = y;

                ThirdDegreeCurve::BezierCurve(hdc, x1, y1, x2, y2, x3, y3, x4, y4, c, c, false);
            }
        }
    }

};