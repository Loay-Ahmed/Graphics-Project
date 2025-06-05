# include "../include/ellipse.h"
# include "../include/common.h"

/**
 * @brief Draws an ellipse using the midpoint algorithm.
 * @param hdc Handle to the device context.
 * @param xc X-coordinate of the center of the ellipse.
 * @param yc Y-coordinate of the center of the ellipse.
 * @param a Semi-major axis length.
 * @param b Semi-minor axis length.
 * @param c Color of the ellipse.
 * This function uses the midpoint algorithm to draw an ellipse by calculating points in the first octant and reflecting them to other octants.
 * The algorithm is efficient and ensures that the ellipse is drawn smoothly without gaps.
 */
void Ellipse::DrawEllipseMidPoint(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    int x = 0, y = b;
    double d = b * b - a * a * b + 0.25 * a * a;
    
    while (b * b * x <= a * a * y) {
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
        if (d < 0) {
            d += 2 * b * b * (2 * x + 3);
        } else {
            d += 2 * b * b * (2 * x + 3) - 4 * a * a * (y - 1);
            y--;
        }
        x++;
    }

    d = b * b * (x + 0.5) * (x + 0.5) / (a * a) + a * a / 4 - b * b;
    while (y >= 0) {
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
        if (d > 0) {
            d -= 4 * a * a * (y - 1);
        } else {
            d += 2 * b * b * (2 * x + 3);
            x++;
        }
        y--;
    }
}

/**
 * @brief Draws an ellipse using the polar coordinate method.
 * @param hdc Handle to the device context.
 * @param xc X-coordinate of the center of the ellipse.
 * @param yc Y-coordinate of the center of the ellipse.
 * @param a Semi-major axis length.
 * @param b Semi-minor axis length.
 * @param c Color of the ellipse.
 * This function calculates points on the ellipse using polar coordinates and draws them using SetPixel.
 */

void Ellipse::DrawEllipsePolar(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    double angle = 0.0;
    double step = 1.0 / std::max(a, b); // Adjust step size based on the ellipse dimensions
    while (angle < 2 * M_PI) {
        int x = Common::Round(xc + a * cos(angle));
        int y = Common::Round(yc + b * sin(angle));
        SetPixel(hdc, x, y, c);
        angle += step;
    
    }
}

/**
 * @brief Draws an ellipse using the standard ellipse equation.
 * @param hdc Handle to the device context.
 * @param xc X-coordinate of the center of the ellipse.
 * @param yc Y-coordinate of the center of the ellipse.
 * @param a Semi-major axis length.
 * @param b Semi-minor axis length.
 * @param c Color of the ellipse.
 * This function calculates points on the ellipse using the standard equation and draws them using SetPixel.
 */
void Ellipse::DrawEllipseEquation(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    if (a == 0 && b == 0) {
        SetPixel(hdc, xc, yc, c);
        return;
    }
    if (a == 0) {
        for (int y = -b; y <= b; y++) {
            SetPixel(hdc, xc, yc + y, c);
        }
        return;
    }
    if (b == 0) {
        for (int x = -a; x <= a; x++) {
            SetPixel(hdc, xc + x, yc, c);
        }
        return;
    }

    // First loop: x-based drawing
    for (int x = -a; x <= a; x++) {
        double y_sq = (1.0 - (double)(x * x) / (a * a)) * (b * b);
        if (y_sq >= 0) {
            int y = round(sqrt(y_sq));
            SetPixel(hdc, xc + x, yc + y, c);
            SetPixel(hdc, xc + x, yc - y, c);
        }
    }

    // Second loop: y-based drawing (needed for tall ellipses)
    for (int y = -b; y <= b; y++) {
        double x_sq = (1.0 - (double)(y * y) / (b * b)) * (a * a);
        if (x_sq >= 0) {
            int x = round(sqrt(x_sq));
            SetPixel(hdc, xc + x, yc + y, c);
            SetPixel(hdc, xc - x, yc + y, c);
        }
    }
}

