#ifndef COMMON_H
#define COMMON_H

#include "import.h"

class Common
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
};
#endif