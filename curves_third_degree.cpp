#include "import.h"

class ThirdDegreeCurve
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

	// Matrix multiplication for hermite and bezier
	static vector<int> matrixMult(vector<vector<int>> m1, vector<int> m2)
	{
		int rows = m1.size();
		int shared = m1[0].size(); // should be equal to m2.size()
		int cols = m2.size();

		vector<int> result(rows, 0);

		for (int i = 0; i < rows; ++i)
		{
			// for (int k = 0; k < cols; ++k) {
			for (int j = 0; j < shared; ++j)
			{
				result[i] += m1[i][j] * m2[j];
			}
			//}
		}

		return result;
	}

	// Hermite algorithm
	static void
	HermiteCurve(HDC hdc, int x1, int y1, int u1, int v1, int x2, int y2, int u2, int v2, COLORREF c1, COLORREF c2)
	{
		vector<vector<int>> H = {
			{2, -2, 1, 1},
			{-3, 3, -2, -1},
			{0, 0, 1, 0},
			{1, 0, 0, 0}};

		vector<int> Gx = {x1, x2, u1, u2}; // 4x1
		vector<int> Gy = {y1, y2, v1, v2}; // 4x1

		// Multiply H (4x4) * Gx/Gy (4x1) => 4x1
		vector<int> Cx = matrixMult(H, Gx);
		vector<int> Cy = matrixMult(H, Gy);

		for (double t = 0; t <= 1.0; t += 0.0001)
		{
			double t2 = t * t;
			double t3 = t2 * t;
			int x = Round(Cx[0] * t3 + Cx[1] * t2 + Cx[2] * t + Cx[3]);
			int y = Round(Cy[0] * t3 + Cy[1] * t2 + Cy[2] * t + Cy[3]);
			COLORREF c = interpolateColors(c1, c2, t);
			SetPixel(hdc, x, y, c);
		}
	}

	//
	//
	//
	//
	//
	// Bezier by matrix
	static void BezierCurve(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, COLORREF c1, COLORREF c2, bool interpolate)
	{
		vector<vector<int>> H = {
			{-1, 3, -3, 1},
			{3, -6, 3, 0},
			{-3, 3, 0, 0},
			{1, 0, 0, 0}};

		vector<int> Gx = {x1, x2, x3, x4}; // 4x1
		vector<int> Gy = {y1, y2, y3, y4}; // 4x1

		// Multiply H (4x4) * Gx/Gy (4x1) => 4x1
		vector<int> Cx = matrixMult(H, Gx);
		vector<int> Cy = matrixMult(H, Gy);

		for (double t = 0; t <= 1.0; t += 0.0001)
		{
			double t2 = t * t;
			double t3 = t2 * t;
			int x = Round(Cx[0] * t3 + Cx[1] * t2 + Cx[2] * t + Cx[3]);
			int y = Round(Cy[0] * t3 + Cy[1] * t2 + Cy[2] * t + Cy[3]);
			COLORREF c = interpolateColors(c1, c2, t);
			SetPixel(hdc, x, y, interpolate ? c : c1);
		}
	}
	//
	//
	//
	//
	//
	//

	// Recursive bezier that calculates the calculates points
	static vector<double> Bezier(vector<double> points, double t)
	{
		if (points.size() == 2)
			return {points[0], points[1]};

		vector<double> newPoints;
		for (int i = 0; i < points.size() - 2; i += 2)
		{
			double x = (1 - t) * points[i] + t * points[i + 2];
			double y = (1 - t) * points[i + 1] + t * points[i + 3];
			newPoints.push_back(x);
			newPoints.push_back(y);
		}
		return Bezier(newPoints, t);
	}

	// Recursive bezier that calculates the draws points
	static void RecBezier(HDC hdc, vector<double> points, COLORREF c1, COLORREF c2)
	{
		for (double t = 0; t < 1; t += 0.00005)
		{
			vector<double> point = Bezier(points, t);
			SetPixel(hdc, point[0], point[1], interpolateColors(c1, c2, t));
		}
	}
};