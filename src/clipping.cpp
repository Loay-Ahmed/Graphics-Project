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
int Clipping::CLIP_X_MIN = 100;
int Clipping::CLIP_Y_MIN = 100;
int Clipping::CLIP_X_MAX = 400;
int Clipping::CLIP_Y_MAX = 400;
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
    switch (edge) {
        case 0: // Left
            i.x = CLIP_X_MIN;
            i.y = p1.y + (int)((CLIP_X_MIN - p1.x) * dy / (dx == 0 ? 1 : dx));
            break;
        case 1: // Right
            i.x = CLIP_X_MAX;
            i.y = p1.y + (int)((CLIP_X_MAX - p1.x) * dy / (dx == 0 ? 1 : dx));
            break;
        case 2: // Bottom
            i.y = CLIP_Y_MIN;
            i.x = p1.x + (int)((CLIP_Y_MIN - p1.y) * dx / (dy == 0 ? 1 : dy));
            break;
        case 3: // Top
            i.y = CLIP_Y_MAX;
            i.x = p1.x + (int)((CLIP_Y_MAX - p1.y) * dx / (dy == 0 ? 1 : dy));
            break;
    }
    return i;
}

std::vector<POINT> Clipping::SutherlandHodgmanClip(const POINT* input, int n) {
    std::vector<POINT> poly(input, input + n);
    for (int e = 0; e < 4; ++e) {
        std::vector<POINT> output;
        for (size_t i = 0; i < poly.size(); ++i) {
            POINT curr = poly[i];
            POINT prev = poly[(i + poly.size() - 1) % poly.size()];
            bool currIn = inside(curr.x, curr.y, SUTH_EDGES[e]);
            bool prevIn = inside(prev.x, prev.y, SUTH_EDGES[e]);
            if (currIn) {
                if (!prevIn) output.push_back(intersect(prev, curr, SUTH_EDGES[e]));
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

void Clipping::ClippingPolygon(HDC hdc, POINT* points, int n, COLORREF color) {
    std::vector<POINT> clipped = SutherlandHodgmanClip(points, n);
    if (clipped.size() < 3) return;
    std::vector<std::vector<Node>> edgeTable(10000);
    int minY = INT_MAX, maxY = INT_MIN;
    for (size_t i = 0; i < clipped.size(); i++) {
        POINT p1 = clipped[i];
        POINT p2 = clipped[(i + 1) % clipped.size()];
        if (p1.y == p2.y) continue;
        int ymin = std::min(p1.y, p2.y);
        int ymax = std::max(p1.y, p2.y);
        int x = (p1.y < p2.y) ? p1.x : p2.x;
        double Minv = (double)(p2.x - p1.x) / (p2.y - p1.y);
        edgeTable[ymin].emplace_back(x, ymax, Minv);
        minY = std::min(minY, ymin);
        maxY = std::max(maxY, ymax);
    }
    LinkedList AET;
    for (int y = minY; y < maxY; y++) {
        for (Node& e : edgeTable[y]) {
            AET.add(e.x, e.maxY, e.Minv);
        }
        AET.removeMaxY(y);
        AET.sort();
        Node* current = AET.head;
        while (current && current->next) {
            int xStart = (int)std::ceil(current->x);
            int xEnd = (int)std::floor(current->next->x);
            for (int x = xStart; x <= xEnd; x++) {
                SetPixel(hdc, x, y, color);
            }
            current = current->next->next;
        }
        Node* updater = AET.head;
        while (updater) {
            updater->x += updater->Minv;
            updater = updater->next;
        }
    }
    AET.clear();
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
    int code1 = Clipping::computeCode(x1, y1);
    int code2 = Clipping::computeCode(x2, y2);
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
            if (codeOut & Clipping::TOP) {
                x = x1 + (x2 - x1) * (Clipping::CLIP_Y_MAX - y1) / (y2 - y1);
                y = Clipping::CLIP_Y_MAX;
            } else if (codeOut & Clipping::BOTTOM) {
                x = x1 + (x2 - x1) * (Clipping::CLIP_Y_MIN - y1) / (y2 - y1);
                y = Clipping::CLIP_Y_MIN;
            } else if (codeOut & Clipping::RIGHT) {
                y = y1 + (y2 - y1) * (Clipping::CLIP_X_MAX - x1) / (x2 - x1);
                x = Clipping::CLIP_X_MAX;
            } else { // LEFT
                y = y1 + (y2 - y1) * (Clipping::CLIP_X_MIN - x1) / (x2 - x1);
                x = Clipping::CLIP_X_MIN;
            }
            if (codeOut == code1) {
                x1 = x; y1 = y; code1 = Clipping::computeCode(x1, y1);
            } else {
                x2 = x; y2 = y; code2 = Clipping::computeCode(x2, y2);
            }
        }
    }
    if (accept) {
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
    }
}

void Clipping::ClipPointSquare(HDC hdc, int x, int y, COLORREF color) {
    if (x >= CLIP_X_MIN && x <= CLIP_X_MAX && y >= CLIP_Y_MIN && y <= CLIP_Y_MAX) {
        SetPixel(hdc, x, y, color);
    }
}

void Clipping::Clippingpoint(HDC hdc, int x, int y, COLORREF color) {
    int width = GetDeviceCaps(hdc, HORZRES);
    int height = GetDeviceCaps(hdc, VERTRES);
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    SetPixel(hdc, x, y, color);
}
