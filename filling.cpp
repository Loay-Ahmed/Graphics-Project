#include "import.h"

class Filling
{
	public:
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
				int x = Round(t1 * x1 + t2 * x2 + (1 - t1 - t2) * x3);
				int y = Round(t1 * y1 + t2 * y2 + (1 - t1 - t2) * y3);
				SetPixel(hdc, x, y, c);
			}
		}
	}
};
static void FillSquareWithVerticalHermiteWaves(HDC hdc, int left, int top, int size, COLORREF c)
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

			HermiteCurve::Draw(hdc, x0, y0, x1, y1, tx0, ty0, tx1, ty1, c);
		}
	}
}
