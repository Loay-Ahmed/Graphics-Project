#include "../include/curves_second_degree.h"
#include "../include/common.h"
#include <cmath>
#include <iostream>

// Remove: namespace SecondDegreeCurve {
// Private helper function
void SecondDegreeCurve::Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c)
{
    int dx[8] = {x, -x, x, -x, y, -y, y, -y};
    int dy[8] = {y, y, -y, -y, x, x, -x, -x};
    for (int i = 0; i < 8; i++)
    {
        SetPixel(hdc, xc + dx[i], yc + dy[i], c);
    }
}
// } // End of namespace

void SecondDegreeCurve::DrawCircle(HDC hdc, int xc, int yc, int x2, int y2, COLORREF c)
{
    int r = Common::Round(sqrt(abs((xc - x2) * (xc - x2) + (yc - y2) * (yc - y2))));
    for (int i = xc; i <= xc + r; i++)
    {
        int x = xc - i;
        int y = Common::Round(sqrt(r * r - x * x));
        SecondDegreeCurve::Draw8Points(hdc, xc, yc, x, y, c);
    }
}

std::pair<int, int> SecondDegreeCurve::BresenhamCircle(HDC hdc, int xc, int yc, int r, COLORREF c)
{
    int x2 = 0, y2 = r;
    SecondDegreeCurve::Draw8Points(hdc, xc, yc, x2, y2, c);
    while (x2 < y2)
    {
        int d = Common::Round(pow(x2 + 1, 2) + pow(y2 - 0.5, 2) - pow(r, 2));
        x2++;
        if (d > 0)
        {
            y2--;
        }
        SecondDegreeCurve::Draw8Points(hdc, xc, yc, x2, y2, c);
    }
    return {x2, y2};
}

void SecondDegreeCurve::InterpolatedColoredCurve(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF c1, COLORREF c2) {
    int alphaX = 2 * x3 + 2 * x1 - 4 * x2;
    int betaX = 4 * x2 - 3 * x1 - x3;
    int alphaY = 2 * y3 + 2 * y1 - 4 * y2;
    int betaY = 4 * y2 - 3 * y1 - y3;
    double steps = 1.0 / 100000;

    for (double t = 0; t <= 1; t += steps) {
        COLORREF c = Common::interpolateColors(c1, c2, t);
        SetPixel(hdc, Common::Round(alphaX * t * t + betaX * t + x1), Common::Round(alphaY * t * t + betaY * t + y1), c);
    }
}

// iterative polar circle
void SecondDegreeCurve::itreativepolar(HDC hdc, int xc, int yc, int r, COLORREF c) {
    double x=r,y=0,dtheta=1.0/r;
    double ct= cos(dtheta),st=sin(dtheta);

    SecondDegreeCurve::Draw8Points(hdc,xc,yc,Common::Round(x),Common::Round(y),c);
    while (x>y) {
        double x1=x*ct-y*st;
    
        y=x*st+y*ct;
        x=x1;
        SecondDegreeCurve::Draw8Points(hdc,xc,yc,Common::Round(x),Common::Round(y),c);
    }
}

void SecondDegreeCurve::directcircle(HDC hdc, int xc, int yc, int r, COLORREF c) {

    int x=0,y=r;

    SecondDegreeCurve::Draw8Points(hdc, xc, yc, x, y, c);
    while (y > x) {
        x++;
        y=round(sqrt(r * r - x * x));
        SecondDegreeCurve::Draw8Points(hdc, xc , yc , x, y, c);
     }
}
    
void SecondDegreeCurve::ModfiedBresenhamcircle(HDC hdc, int xc, int yc, int r, COLORREF c) {
    int x=0,y=r,d=1-r;
    int d1=3,d2=5-2*r;

    SecondDegreeCurve::Draw8Points(hdc,xc,yc,x,y,c);
    while (x<y) {
        x++;
        if (d<0) { 
            d+=d1; 
            d1+=2;
            d2+=2;
        } else {
            y--;
            d+=d2;
            d1+=2;
            d2+=4;
        }
        SecondDegreeCurve::Draw8Points(hdc,xc,yc,x,y,c);
    }
}
