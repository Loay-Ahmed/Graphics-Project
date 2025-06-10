#include "../include/filling.h"
#include "../include/common.h"
#include "../include/lines.h"
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