// Header for tasks_and_assignments.cpp
#pragma once
#include <windows.h>

class TasksAndAssignments {
public:
    static void pizzaCircle(HDC hdc, int xc, int yc, int r, COLORREF c);
    static void BezierInterpolatedCurve(HDC hdc, int x1, int y1, COLORREF c1, int x2, int y2, COLORREF c2, int x3, int y3, COLORREF c3, int x4, int y4, COLORREF c4);
};
