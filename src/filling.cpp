#include "../include/filling.h"
#include "../include/common.h"
#include "../include/lines.h"
#include "../include/curves_second_degree.h"
#include "../include/curves_third_degree.h"
#include <cmath>
using namespace std;

struct point {
    int x, y;
    point(int x, int y) : x(x), y(y) {}
};

void Filling::RecursiveFloodFill(HDC hdc, int x, int y, COLORREF color) {
    COLORREF current = GetPixel(hdc, x, y);
    if (current == color) return;

    SetPixel(hdc, x, y, color);

    RecursiveFloodFill(hdc, x + 1, y, color);
    RecursiveFloodFill(hdc, x, y + 1, color);
    RecursiveFloodFill(hdc, x - 1, y, color);
    RecursiveFloodFill(hdc, x, y - 1, color);
}

void Filling::NonRecursiveFloodFill(HDC hdc, int x, int y, COLORREF color) {
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};
    queue<point> queue;
    queue.push(point(x, y));

    while (!queue.empty()) {
        point p = queue.front();
        queue.pop();
        COLORREF current = GetPixel(hdc, p.x, p.y);
        if (current == color) continue;

        SetPixel(hdc, p.x, p.y, color);
        for (int i = 0; i < 4; i++) {
            queue.push(point(p.x + dx[i], p.y + dy[i]));
        }
    }
}

void Filling::BarycentricFill(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color) {
    for (double t1 = 0; t1 < 1; t1 += 0.001) {
        for (double t2 = 0; t2 < 1 - t1; t2 += 0.001) {
            int x = Common::Round(t1 * x1 + t2 * x2 + (1 - t1 - t2) * x3);
            int y = Common::Round(t1 * y1 + t2 * y2 + (1 - t1 - t2) * y3);
            SetPixel(hdc, x, y, color);
        }
    }
}

// Utility method to convert POINT to Point
vector<Filling::Point> Filling::ConvertToPoints(const vector<POINT>& points) {
    vector<Point> result;
    result.reserve(points.size());
    for (const auto& p : points) {
        result.emplace_back(p);
    }
    return result;
}

// Convex Fill Methods
void Filling::InitConvexEdgeTable(ConvexEdgeTableArray& table) {
    for (int i = 0; i < 800; i++) {
        table[i] = ConvexEdgeTable();
    }
}

void Filling::EdgeToConvexTable(const Point& v1, const Point& v2, ConvexEdgeTableArray& table) {
    if (v1.y == v2.y) return;
    
    Point v1_copy = v1;
    Point v2_copy = v2;
    if (v1_copy.y > v2_copy.y) {
        std::swap(v1_copy, v2_copy);
    }

    int y = v1_copy.y;
    double x = v1_copy.x;
    double minv = (v2_copy.x - v1_copy.x) / (v2_copy.y - v1_copy.y);

    while (y < v2_copy.y) {
        if (x < table[y].xleft) table[y].xleft = (int)ceil(x);
        if (x > table[y].xright) table[y].xright = (int)floor(x);
        y++;
        x += minv;
    }
}

void Filling::PolygonToConvexTable(const vector<Point>& points, ConvexEdgeTableArray& table) {
    Point v1 = points.back();
    for (const Point& v2 : points) {
        EdgeToConvexTable(v1, v2, table);
        v1 = v2;
    }
}

void Filling::ConvexTableToScreen(HDC hdc, const ConvexEdgeTableArray& table, COLORREF color) {
    for (int i = 0; i < 800; i++) {
        if (table[i].xleft < table[i].xright) {
            Lines::LineBresenhamDDA(hdc, table[i].xleft, i, table[i].xright, i, color);
        }
    }
}

void Filling::ConvexFill(HDC hdc, const vector<Point>& points, COLORREF color) {
    ConvexEdgeTableArray table;
    InitConvexEdgeTable(table);
    PolygonToConvexTable(points, table);
    ConvexTableToScreen(hdc, table, color);
}

void Filling::ConvexFill(HDC hdc, const vector<POINT>& points, COLORREF color) {
    ConvexFill(hdc, ConvertToPoints(points), color);
}

// Non-Convex Fill Methods
void Filling::InitNonConvexEdgeTable(NonConvexEdgeTable& table) {
    for (int i = 0; i < 800; i++) {
        table[i].clear();
    }
}

void Filling::EdgeToNonConvexTable(const Point& v1, const Point& v2, NonConvexEdgeTable& table) {
    if (v1.y == v2.y) return;
    
    Point v1_copy = v1;
    Point v2_copy = v2;
    if (v1_copy.y > v2_copy.y) {
        std::swap(v1_copy, v2_copy);
    }

    int y = v1_copy.y;
    double x = v1_copy.x;
    double minv = (v2_copy.x - v1_copy.x) / (v2_copy.y - v1_copy.y);

    while (y < v2_copy.y) {
        table[y].push_back((int)round(x));
        y++;
        x += minv;
    }
}

void Filling::PolygonToNonConvexTable(const vector<Point>& points, NonConvexEdgeTable& table) {
    Point v1 = points.back();
    for (const Point& v2 : points) {
        EdgeToNonConvexTable(v1, v2, table);
        v1 = v2;
    }
}

void Filling::NonConvexTableToScreen(HDC hdc, const NonConvexEdgeTable& table, COLORREF color) {
    for (int y = 0; y < 800; y++) {
        if (table[y].empty()) continue;

        std::list<int> sortedList = table[y];  // Create a copy
        sortedList.sort();  // Sort the copy
        auto it = sortedList.begin();
        while (it != sortedList.end()) {
            int x1 = *it++;
            if (it == sortedList.end()) break;
            int x2 = *it++;
            Lines::LineBresenhamDDA(hdc, x1, y, x2, y, color);
        }
    }
}

void Filling::NonConvexFill(HDC hdc, const vector<Point>& points, COLORREF color) {
    NonConvexEdgeTable table;
    InitNonConvexEdgeTable(table);
    PolygonToNonConvexTable(points, table);
    NonConvexTableToScreen(hdc, table, color);
}

void Filling::NonConvexFill(HDC hdc, const vector<POINT>& points, COLORREF color) {
    NonConvexFill(hdc, ConvertToPoints(points), color);
}



void Filling::FillQuarterWithSmallCircles(HDC hdc, int xc, int yc, int R, int quarter, COLORREF c)
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


    void Filling::FillRectangleWithBezierWaves(HDC hdc, int left, int top, int right, int bottom, COLORREF c)
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

                ThirdDegreeCurve::BezierCurve(hdc, x1, y1, x2, y2, x3, y3, x4, y4, c);
            }
        }
    }

    void Filling::FillCircleQuarter(HDC hdc, int xc, int yc, int radius, int quarter, COLORREF c)
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

    void Filling::FillSquareWithVerticalHermiteWaves(HDC hdc, int left, int top, int size, COLORREF c)
{
	int waveHeight = 15;  // Amplitude
	int waveLength = 40;  // Vertical wave cycle
	int stepX = 6;        // Horizontal step for density

	int right = left + size;
	int bottom = top + size;

	for (int x = left; x <= right; x += stepX)
	{
		for (int y = top; y < bottom; y += waveLength)
		{
			// Hermite curve needs two points and two tangents
			int x0 = x;
			int y0 = y;
			int x1 = x;
			int y1 = y + waveLength;

			// Tangents: create a sine-like wave vertically
			int tx0 = waveHeight;
			int ty0 = 0;
			int tx1 = -waveHeight;
			int ty1 = 0;

			ThirdDegreeCurve::HermiteCurve(hdc, x0, y0, tx0, ty0, x1, y1, tx1, ty1, c);
		}
	}
}
