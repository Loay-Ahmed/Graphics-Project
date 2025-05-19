#include "import.h"
// #include "curves_second_degree.cpp"

class TasksAndAssignments
{public:
    static int Round(double x)
    {
        return (int)(x + 0.5);
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
    // Interpolate colors
    static COLORREF interpolateColors(COLORREF c1, COLORREF c2, double t)
    {
    	int r = Round(GetRValue(c1) * t + (1 - t) * GetRValue(c2));
    	int g = Round(GetGValue(c1) * t + (1 - t) * GetGValue(c2));
    	int b = Round(GetBValue(c1) * t + (1 - t) * GetBValue(c2));
    	return RGB(r, g, b);
    }
    static void pizzaCircle(HDC hdc, int xc, int yc, int r , COLORREF c)
    {
        int x = 0, y = r;

        // Draw line vertical and horizontal
        Lines lines;
        lines.LineBresenham(hdc, xc - r, yc, xc + r, yc, c);
        lines.LineBresenham(hdc, xc, yc - r, xc, yc + r, c);

        // Draw circle by bresenham
        SecondDegreeCurve second;
        pair<int, int> point = second.BresenhamCircle(hdc, xc, yc, r , c);
        x = point.first, y = point.second;

        lines.LineBresenham(hdc, xc - x, yc - y, xc + x, yc + y, c);
        lines.LineBresenham(hdc, xc + x, yc - y, xc - x, yc + y, c);
    }

    static void BezierInterpolatedCurve(HDC hdc, int x1, int y1 , COLORREF c1, int x2, int y2 , COLORREF c2, int x3, int y3 , COLORREF c3, int x4, int y4 , COLORREF c4)
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
            COLORREF c;
            if (t < 1.0 / 3)
            {
                c = interpolateColors(c1, c2, t);
            }
            else if (t > 1.0 / 3 && t < 2.0 / 3)
            {
                c = interpolateColors(c2, c3, t);
            }
            else if (t > 2.0 / 3 && t < 1.0)
            {
                c = interpolateColors(c3, c4, t);
            }
            SetPixel(hdc, x, y, c);
        }
    }

    //
};