#include "import.h"

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

//for convex
typedef struct Entrytable {
    int xleft, xright;

} Entrytable;
typedef Entrytable edgetable[800];

struct point
{
    double x, y;
    point()
    {
        x = 0; y = 0;
    }
};

void init(edgetable tbl)
{
    for (int i = 0; i < 800; i++)
    {
        tbl[i].xleft = 10000;
        tbl[i].xright = -10000;
    }
}

void edge2table(point v1, point v2, edgetable tbl)
{
    if (v1.y == v2.y) return;
    if (v1.y > v2.y) { swap(v1, v2); }
    int y = v1.y;
    double x = v1.x;
    double minv = (v2.x - v1.x) / (v2.y - v1.y);
    while (y < v2.y)
    {
        if (x < tbl[y].xleft) tbl[y].xleft = (int)ceil(x);
        if (x > tbl[y].xright) tbl[y].xright = (int)floor(x);
        y++;
        x += minv;
    }
}

void polygon2table(point p[], int n, edgetable tbl) {
    point v1 = p[n - 1];
    for (int i = 0; i < n; i++)
    {
        point v2 = p[i];
        edge2table(v1, v2, tbl);
        v1 = p[i];
    }
}

void table2screen(HDC hdc, edgetable tbl, COLORREF c)
{
    for (int i = 0; i < 800; i++)
    {
        if (tbl[i].xleft < tbl[i].xright)
        {
            LineBresenham(hdc, tbl[i].xleft, i, tbl[i].xright, i, c);
        }
    }
}

void convexfill(HDC hdc, point p[], int n, COLORREF c)
{
    edgetable tbl;
    init(tbl);
    polygon2table(p, n, tbl);
    table2screen(hdc, tbl, c);
}
};
