#include "common.cpp"
#include "filling.cpp"
#include "curves_third_degree.cpp"
#include "curves_second_degree.cpp"
#include "lines.cpp"
#include "tasks_and_assignments.cpp"
#include "import.h"
using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int x1, y1, x2, y2;
    static std::vector<double> points;
    static int counter = 0;

    // --- NEW --- mode flag for switching behaviors
    static bool circleDrawingMode = false;
    static int circleRadius = 50;      // example fixed radius, can be changed or made dynamic
    static int circleQuarter = 1;      // quarter to fill (1-4)
    static bool circleCenterSet = false;

    switch (message)
    {
        case WM_KEYDOWN:
            if (wParam == 'C' || wParam == 'c') // toggle mode on pressing 'C'
            {
                circleDrawingMode = !circleDrawingMode;
                points.clear();
                counter = 0;
                circleCenterSet = false;
                InvalidateRect(hWnd, NULL, TRUE); // force redraw if needed
            }
            break;

        case WM_LBUTTONDOWN:
        {
            HDC hdc = GetDC(hWnd);

            if (circleDrawingMode)
            {
                if (!circleCenterSet)
                {
                    // First click: set circle center
                    x1 = LOWORD(lParam);
                    y1 = HIWORD(lParam);
                    circleCenterSet = true;

                    // Optionally, draw the circle outline here
                    // For now, just draw a circle using Bresenham or your method
                    int r = circleRadius;
                    SecondDegreeCurve::BresenhamCircle(hdc, x1, y1, r, RGB(0, 0, 0)); // black circle outline
                }
                else
                {
                    // Second click: select quarter (1 to 4)
                    // For simplicity, compute quarter based on relative position from center
                    int x = LOWORD(lParam);
                    int y = HIWORD(lParam);

                    int dx = x - x1;
                    int dy = y1 - y; // invert y because GDI y-axis downwards

                    if (dx >= 0 && dy >= 0)
                        circleQuarter = 1; // top-right
                    else if (dx < 0 && dy >= 0)
                        circleQuarter = 2; // top-left
                    else if (dx < 0 && dy < 0)
                        circleQuarter = 3; // bottom-left
                    else
                        circleQuarter = 4; // bottom-right

                    COLORREF fillColor = RGB(255, 0, 0); // red fill color
                    Filling::FillCircleQuarter(hdc, x1, y1, circleRadius, circleQuarter, fillColor);

                    // Reset for next circle
                    circleCenterSet = false;
                }
                ReleaseDC(hWnd, hdc);
            }
            else
            {
                // Your existing Bezier points logic unchanged

                COLORREF c1 = RGB(50, 100, 0);

                // use this for [ InterpolatedColoredCurve, BezierCurve, RecBezier ]
                points.push_back(LOWORD(lParam));
                points.push_back(HIWORD(lParam));
                SetPixel(hdc, points[counter * 2], points[counter * 2 + 1], c1);

                // use this for [ InterpolatedColoredCurve, BezierCurve, RecBezier ]
                counter++;

                // for two or less points use this
                // x1 = LOWORD(lParam);
                // y1 = HIWORD(lParam);

                // use this for interpolated colors
                COLORREF c2 = RGB(108, 200, 255);

                if (counter == 4)
                { // adjust counter for number of points

                    // SecondDegreeCurve::InterpolatedColoredCurve(hdc, points[0], points[1], points[2], points[3], points[4], points[5], c1, c2);
                    ThirdDegreeCurve::BezierCurve(hdc, points[0], points[1], points[2], points[3], points[4], points[5], points[6], points[7], c1, c2, false);
                    // ThirdDegreeCurve::BezierInterpolatedCurve(hdc, points[0], points[1], RGB(255, 0, 0), points[2], points[3], RGB(255, 255, 0), points[4], points[5], RGB(0, 255, 0), points[6], points[7], RGB(0, 0, 255));
                    // ThirdDegreeCurve::RecBezier(hdc, points, c1, c2);
                    // Filling::BaryCentric(hdc, points[0], points[1], points[2], points[3], points[4], points[5], c2);
                    // use this for [ InterpolatedColoredCurve, BezierCurve, RecBezier ]
                    points.clear();
                    counter = 0;
                }

                ReleaseDC(hWnd, hdc);
            }

            break;
        }

        case WM_LBUTTONUP:
        {
            HDC hdc = GetDC(hWnd);

            // use for two points algorithms + Hermite
            // x2 = LOWORD(lParam);
            // y2 = HIWORD(lParam);
            // COLORREF c1 = RGB(50, 0, 0);
            // COLORREF c2 = RGB(108, 200, 255);

            // int r = Round(sqrt(abs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))));
            // SecondDegreeCurve::BresenhamCircle(hdc, xc, yc, r, c);
            // Lines::DrawLineByMidPoint(hdc, x1, y1, x2, y2, c1);
            // Lines::InterpolatedColoredLine(hdc, x1, y1, x2, y2, c1, c2);
            // ThirdDegreeCurve::HermiteCurve(hdc, x1, y1, x1 + 20, y1 - 20, x2, y2, x2 + 20, y2 - 20, c1, c2);
            // Lines::LineBresenhamDDA(hdc, x1, y1, x2, y2, c1);
            // ReleaseDC(hWnd, hdc);
            // TasksAndAssignments::pizzaCircle(hdc, x1, y1, r, c1);
            break;
        }

        case WM_RBUTTONDOWN:
        {
            HDC hdc = GetDC(hWnd);
            x2 = LOWORD(lParam);
            y2 = HIWORD(lParam);
            COLORREF c2 = RGB(50, 100, 0);
            Filling::NonRecFloodFill(hdc, x2, y2, c2);
            ReleaseDC(hWnd, hdc);
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            EndPaint(hWnd, &ps);
        }
            break;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hi, HINSTANCE pi, LPSTR cmd, int nsh)
{
    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.lpszClassName = "MyClass";
    wc.lpszMenuName = NULL;
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hi;
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("MyClass", "Hello World!", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hi, 0);
    ShowWindow(hwnd, nsh);
    UpdateWindow(hwnd);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
