#include "import.h"

class Filling
{

public:
	void RecFloodFill(HDC hdc, int x, int y, COLORREF c)
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

	void NonRecFloodFill(HDC hdc, int x, int y, COLORREF c)
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

	void BaryCentric(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF c)
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
};