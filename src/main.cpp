#undef UNICODE
#undef _UNICODE
#include "../include/common.h"
#include "../include/filling.h"
#include "../include/curves_third_degree.h"
#include "../include/curves_second_degree.h"
#include "../include/lines.h"
#include "../include/tasks_and_assignments.h"
#include "../include/import.h"
#include "../include/ellipse.h" // Added include for ellipse.h
#include "../include/clipping.h"
using namespace std;

// Drawing modes
enum DrawingMode {
    MODE_NONE,
    MODE_BEZIER_CURVE,
    MODE_LINE_DDA,
    MODE_CIRCLE,
    MODE_HERMITE_CURVE,
    MODE_PIZZA,
    MODE_FLOOD_FILL,
    MODE_INTERPOLATED_LINE,
    MODE_ELLIPSE_MIDPOINT,
    MODE_ELLIPSE_POLAR,
    MODE_ELLIPSE_EQUATION
};

// Global variables for control
static DrawingMode currentMode = MODE_NONE;
static HWND hwndButtons[11];  // Updated for 11 buttons
static const int BUTTON_WIDTH = 150;
static const int BUTTON_HEIGHT = 30;
static const int BUTTON_SPACING = 10;
static const int DRAWING_AREA_LEFT = BUTTON_WIDTH + 20;  // Space for control panel

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int x1, y1, x2, y2;
    static vector<double> points;
    static int counter = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        // Example: Set the clipping window dynamically (user could set via dialog/UI)
        // For now, set to (150, 150, 350, 350) for demonstration
        Clipping::SetClipWindow(150, 150, 350, 350);

        // Add new button labels for ellipses
        const char* buttonLabels[] = {
            "Bezier Curve",
            "Line DDA",
            "Circle",
            "Hermite Curve",
            "Pizza Circle",
            "Flood Fill",
            "Interpolated Line",
            "Ellipse MidPoint",
            "Ellipse Polar",
            "Ellipse Equation",
            "Clear"
        };
        for (int i = 0; i < 11; i++) {
            hwndButtons[i] = CreateWindow(
                "BUTTON",
                buttonLabels[i],
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                5, 5 + i * (BUTTON_HEIGHT + BUTTON_SPACING),
                BUTTON_WIDTH, BUTTON_HEIGHT,
                hWnd, (HMENU)(i + 1),
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL
            );
        }
    }
    break;
    case WM_COMMAND:
        {
            int buttonId = LOWORD(wParam);
            switch (buttonId) {
                case 1: currentMode = MODE_BEZIER_CURVE; counter = 0; points.clear(); break;
                case 2: currentMode = MODE_LINE_DDA; counter = 0; points.clear(); break;
                case 3: currentMode = MODE_CIRCLE; counter = 0; points.clear(); break;
                case 4: currentMode = MODE_HERMITE_CURVE; counter = 0; points.clear(); break;
                case 5: currentMode = MODE_PIZZA; counter = 0; points.clear(); break;
                case 6: currentMode = MODE_FLOOD_FILL; counter = 0; points.clear(); break;
                case 7: currentMode = MODE_INTERPOLATED_LINE; counter = 0; points.clear(); break;
                case 8: currentMode = MODE_ELLIPSE_MIDPOINT; counter = 0; points.clear(); break;
                case 9: currentMode = MODE_ELLIPSE_POLAR; counter = 0; points.clear(); break;
                case 10: currentMode = MODE_ELLIPSE_EQUATION; counter = 0; points.clear(); break;
                case 11: // Clear
                    InvalidateRect(hWnd, NULL, TRUE);
                    UpdateWindow(hWnd);
                    currentMode = MODE_NONE;
                    counter = 0;
                    points.clear();
                    break;
            }
        }
        break;

    case WM_LBUTTONDOWN:
        {
            // Get coordinates relative to drawing area
            int x = LOWORD(lParam) - DRAWING_AREA_LEFT;
            int y = HIWORD(lParam);
            
            // Ignore clicks in the control panel area
            if (x < 0) return 0;

            HDC hdc = GetDC(hWnd);
            COLORREF c1 = RGB(50, 100, 0);
            COLORREF c2 = RGB(108, 200, 255);

            switch (currentMode) {
                case MODE_BEZIER_CURVE:
                    points.push_back(x);
                    points.push_back(y);
                    SetPixel(hdc, x + DRAWING_AREA_LEFT, y, c1);
                    counter++;
                    
                    if (counter == 4) {
                        ThirdDegreeCurve::BezierCurve(hdc, 
                            points[0] + DRAWING_AREA_LEFT, points[1],
                            points[2] + DRAWING_AREA_LEFT, points[3],
                            points[4] + DRAWING_AREA_LEFT, points[5],
                            points[6] + DRAWING_AREA_LEFT, points[7],
                            c1, c2, false);
                        points.clear();
                        counter = 0;
                    }
                    break;

                case MODE_LINE_DDA:
                    if (counter == 0) {
                        x1 = x;
                        y1 = y;
                        counter = 1;
                    } else {
                        Lines::LineBresenhamDDA(hdc, x1 + DRAWING_AREA_LEFT, y1, x + DRAWING_AREA_LEFT, y, c1);
                        counter = 0;
                    }
                    break;

                case MODE_CIRCLE:
                    if (counter == 0) {
                        x1 = x;
                        y1 = y;
                        counter = 1;
                    } else {
                        int r = Common::Round(sqrt(pow(x - x1, 2) + pow(y - y1, 2)));
                        SecondDegreeCurve::BresenhamCircle(hdc, x1 + DRAWING_AREA_LEFT, y1, r, c1);
                        counter = 0;
                    }
                    break;

                case MODE_PIZZA:
                    if (counter == 0) {
                        x1 = x;
                        y1 = y;
                        counter = 1;
                    } else {
                        int r = Common::Round(sqrt(pow(x - x1, 2) + pow(y - y1, 2)));
                        TasksAndAssignments::pizzaCircle(hdc, x1 + DRAWING_AREA_LEFT, y1, r, c1);
                        counter = 0;
                    }
                    break;

                case MODE_HERMITE_CURVE:
                    if (counter == 0) {
                        x1 = x;
                        y1 = y;
                        counter = 1;
                    } else {
                        ThirdDegreeCurve::HermiteCurve(hdc, 
                            x1 + DRAWING_AREA_LEFT, y1, x1 + 20 + DRAWING_AREA_LEFT, y1 - 20,
                            x + DRAWING_AREA_LEFT, y, x + 20 + DRAWING_AREA_LEFT, y - 20,
                            c1, c2);
                        counter = 0;
                    }
                    break;

                case MODE_INTERPOLATED_LINE:
                    if (counter == 0) {
                        x1 = x;
                        y1 = y;
                        counter = 1;
                    } else {
                        Lines::InterpolatedColoredLine(hdc, x1 + DRAWING_AREA_LEFT, y1, x + DRAWING_AREA_LEFT, y, c1, c2);
                        counter = 0;
                    }
                    break;

                case MODE_ELLIPSE_MIDPOINT:
                case MODE_ELLIPSE_POLAR:
                case MODE_ELLIPSE_EQUATION:
                    if (counter == 0) {
                        x1 = x;
                        y1 = y;
                        counter = 1;
                    } else {
                        int a = abs(x - x1);
                        int b = abs(y - y1);
                        if (currentMode == MODE_ELLIPSE_MIDPOINT)
                            Ellipse::DrawEllipseMidPoint(hdc, x1 + DRAWING_AREA_LEFT, y1, a, b, c1);
                        else if (currentMode == MODE_ELLIPSE_POLAR)
                            Ellipse::DrawEllipsePolar(hdc, x1 + DRAWING_AREA_LEFT, y1, a, b, c1);
                        else if (currentMode == MODE_ELLIPSE_EQUATION)
                            Ellipse::DrawEllipseEquation(hdc, x1 + DRAWING_AREA_LEFT, y1, a, b, c1);
                        counter = 0;
                    }
                    break;
            }
            ReleaseDC(hWnd, hdc);
        }
        break;

    case WM_RBUTTONDOWN:
        {
            if (currentMode == MODE_FLOOD_FILL) {
                HDC hdc = GetDC(hWnd);
                int x = LOWORD(lParam) - DRAWING_AREA_LEFT;
                int y = HIWORD(lParam);
                if (x >= 0) {  // Only flood fill in drawing area
                    COLORREF c2 = RGB(50, 100, 0);
                    Filling::NonRecFloodFill(hdc, x + DRAWING_AREA_LEFT, y, c2);
                }
                ReleaseDC(hWnd, hdc);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Draw the dynamic clipping window border
            Rectangle(hdc, Clipping::CLIP_X_MIN, Clipping::CLIP_Y_MIN, Clipping::CLIP_X_MAX, Clipping::CLIP_Y_MAX);
            // Test: Draw a polygon, a line, and a point, all partially outside the window
            POINT poly[5] = {
                {80, 80}, {420, 120}, {380, 420}, {120, 380}, {80, 80}
            };
            Clipping::ClippingPolygon(hdc, poly, 5, RGB(0, 0, 255));
            Clipping::ClippingLine(hdc, 50, 50, 450, 450, RGB(255, 0, 0));
            Clipping::ClipPointSquare(hdc, 90, 90, RGB(0, 255, 0)); // outside
            Clipping::ClipPointSquare(hdc, 200, 200, RGB(0, 255, 0)); // inside
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = "GraphicsClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create window with extra space for control panel
    HWND hwnd = CreateWindow(
        "GraphicsClass", "Graphics Project",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800 + DRAWING_AREA_LEFT, 600,  // Width includes control panel
        NULL, NULL,
        hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}