#include <windows.h>
#include <algorithm>
#include <limits.h>
#include <vector>
#include "../include/clipping.h"
#ifndef IMPORT_PATH
#define IMPORT_PATH "../include/import.h"
#endif
#include IMPORT_PATH

// Dynamic window boundaries (user can set)
int Clipping::CLIP_X_MIN = 0;
int Clipping::CLIP_Y_MIN = 0;
int Clipping::CLIP_X_MAX = 800;
int Clipping::CLIP_Y_MAX = 600;
const int Clipping::SUTH_EDGES[4] = {0, 1, 2, 3};

void Clipping::SetClipWindow(int xmin, int ymin, int xmax, int ymax) {
    CLIP_X_MIN = xmin;
    CLIP_Y_MIN = ymin;
    CLIP_X_MAX = xmax;
    CLIP_Y_MAX = ymax;
}

bool Clipping::inside(int x, int y, int edge) {
    switch (edge) {
        case 0: return x >= CLIP_X_MIN; // Left
        case 1: return x <= CLIP_X_MAX; // Right
        case 2: return y >= CLIP_Y_MIN; // Bottom
        case 3: return y <= CLIP_Y_MAX; // Top
    }
    return false;
}

POINT Clipping::intersect(POINT p1, POINT p2, int edge) {
    POINT i = p1;
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    
    // Avoid division by zero
    if (dx == 0 && dy == 0) return p1;
    
    switch (edge) {
        case 0: // Left
            i.x = CLIP_X_MIN;
            i.y = p1.y + (int)((CLIP_X_MIN - p1.x) * dy / dx);
            break;
        case 1: // Right
            i.x = CLIP_X_MAX;
            i.y = p1.y + (int)((CLIP_X_MAX - p1.x) * dy / dx);
            break;
        case 2: // Bottom
            i.y = CLIP_Y_MIN;
            i.x = p1.x + (int)((CLIP_Y_MIN - p1.y) * dx / dy);
            break;
        case 3: // Top
            i.y = CLIP_Y_MAX;
            i.x = p1.x + (int)((CLIP_Y_MAX - p1.y) * dx / dy);
            break;
    }
    return i;
}

std::vector<POINT> Clipping::SutherlandHodgmanClip(const POINT* input, int n) {
    if (n < 3) return std::vector<POINT>();
    
    std::vector<POINT> poly(input, input + n);
    for (int e = 0; e < 4; ++e) {
        std::vector<POINT> output;
        for (size_t i = 0; i < poly.size(); ++i) {
            POINT curr = poly[i];
            POINT prev = poly[(i + poly.size() - 1) % poly.size()];
            bool currIn = inside(curr.x, curr.y, SUTH_EDGES[e]);
            bool prevIn = inside(prev.x, prev.y, SUTH_EDGES[e]);
            
            if (currIn) {
                if (!prevIn) {
                    POINT intersection = intersect(prev, curr, SUTH_EDGES[e]);
                    output.push_back(intersection);
                }
                output.push_back(curr);
            } else if (prevIn) {
                output.push_back(intersect(prev, curr, SUTH_EDGES[e]));
            }
        }
        poly = output;
        if (poly.empty()) break;
    }
    return poly;
}

void Clipping::ClippingPolygon(HDC hdc, const POINT* points, int n, COLORREF color) {
    if (n < 3) return;
    
    std::vector<POINT> clipped = SutherlandHodgmanClip(points, n);
    if (clipped.size() < 3) return;

    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    
    // Draw the clipped polygon
    MoveToEx(hdc, clipped[0].x, clipped[0].y, NULL);
    for (size_t i = 1; i < clipped.size(); ++i) {
        LineTo(hdc, clipped[i].x, clipped[i].y);
    }
    LineTo(hdc, clipped[0].x, clipped[0].y); // Close the polygon
    
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}

/**
 * @brief Draws a line with clipping using the Cohen-Sutherland algorithm.
 * @param hdc Handle to the device context.
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @param color The color of the line.
 * This function implements the Cohen-Sutherland line clipping algorithm to draw a line segment within a clipping window.
 */
void Clipping::ClippingLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int code1 = computeCode(x1, y1);
    int code2 = computeCode(x2, y2);
    bool accept = false;
    
    while (true) {
        if ((code1 | code2) == 0) {
            // Both endpoints inside
            accept = true;
            break;
        } else if (code1 & code2) {
            // Both endpoints share an outside zone
            break;
        } else {
            int codeOut = code1 ? code1 : code2;
            int x, y;
            
            if (codeOut & TOP) {
                x = x1 + (x2 - x1) * (CLIP_Y_MAX - y1) / (y2 - y1);
                y = CLIP_Y_MAX;
            } else if (codeOut & BOTTOM) {
                x = x1 + (x2 - x1) * (CLIP_Y_MIN - y1) / (y2 - y1);
                y = CLIP_Y_MIN;
            } else if (codeOut & RIGHT) {
                y = y1 + (y2 - y1) * (CLIP_X_MAX - x1) / (x2 - x1);
                x = CLIP_X_MAX;
            } else { // LEFT
                y = y1 + (y2 - y1) * (CLIP_X_MIN - x1) / (x2 - x1);
                x = CLIP_X_MIN;
            }
            
            if (codeOut == code1) {
                x1 = x; y1 = y; code1 = computeCode(x1, y1);
            } else {
                x2 = x; y2 = y; code2 = computeCode(x2, y2);
            }
        }
    }
    
    if (accept) {
        HPEN hPen = CreatePen(PS_SOLID, 1, color);
        HGDIOBJ oldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);
    }
}

void Clipping::ClipPointSquare(HDC hdc, int x, int y, COLORREF color) {
    if (x >= CLIP_X_MIN && x <= CLIP_X_MAX && y >= CLIP_Y_MIN && y <= CLIP_Y_MAX) {
        SetPixel(hdc, x, y, color);
    }
}

void Clipping::Clippingpoint(HDC hdc, int x, int y, COLORREF color) {
    if (x >= CLIP_X_MIN && x <= CLIP_X_MAX && y >= CLIP_Y_MIN && y <= CLIP_Y_MAX) {
        SetPixel(hdc, x, y, color);
    }
}

int Clipping::computeCode(int x, int y) {
    int code = INSIDE;
    if (x < CLIP_X_MIN) code |= LEFT;
    else if (x > CLIP_X_MAX) code |= RIGHT;
    if (y < CLIP_Y_MIN) code |= BOTTOM;
    else if (y > CLIP_Y_MAX) code |= TOP;
    return code;
}
