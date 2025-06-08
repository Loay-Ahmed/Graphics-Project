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
#include <commdlg.h> // For color dialog
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

// Add menu and state variables
enum MenuShapeType {
    SHAPE_NONE,
    SHAPE_LINE,
    SHAPE_CIRCLE,
    SHAPE_ELLIPSE,
    SHAPE_RECT,
    SHAPE_POLYGON,
    SHAPE_SPLINE,
    SHAPE_CLIP,
    SHAPE_FILL
};

// Drawing algorithm choices
enum LineAlgorithm { LINE_DDA, LINE_MIDPOINT, LINE_PARAMETRIC };
enum CircleAlgorithm { CIRCLE_DIRECT, CIRCLE_POLAR, CIRCLE_ITER_POLAR, CIRCLE_MIDPOINT, CIRCLE_MOD_MIDPOINT };
enum EllipseAlgorithm { ELLIPSE_DIRECT, ELLIPSE_POLAR, ELLIPSE_MIDPOINT };

// Global variables for control
static DrawingMode currentMode = MODE_NONE;
static HWND hwndButtons[11];  // Updated for 11 buttons
static const int BUTTON_WIDTH = 150;
static const int BUTTON_HEIGHT = 30;
static const int BUTTON_SPACING = 10;
static const int DRAWING_AREA_LEFT = BUTTON_WIDTH + 20;  // Space for control panel

static MenuShapeType currentShape = SHAPE_NONE;
static LineAlgorithm currentLineAlg = LINE_DDA;
static CircleAlgorithm currentCircleAlg = CIRCLE_DIRECT;
static EllipseAlgorithm currentEllipseAlg = ELLIPSE_DIRECT;
static COLORREF currentColor = RGB(0,0,0);
static std::vector<POINT> userPoints;
static std::vector<std::string> drawnShapes; // For save/load (not implemented)

// --- Layered shape system ---
#include <variant>
#include <optional>

// Shape types for layers
struct LayerLine { POINT p1, p2; COLORREF color; LineAlgorithm alg; };
struct LayerCircle { POINT center; int r; COLORREF color; CircleAlgorithm alg; };
struct LayerEllipse { POINT center; int a, b; COLORREF color; EllipseAlgorithm alg; };
struct LayerRect { POINT p1, p2; COLORREF color; };
struct LayerPolygon { std::vector<POINT> pts; COLORREF color; };
// Add more as needed
using LayerShape = std::variant<LayerLine, LayerCircle, LayerEllipse, LayerRect, LayerPolygon>;

struct Layer {
    LayerShape shape;
};

static std::vector<Layer> layers;
static std::optional<LayerPolygon> currentPolygon; // For collecting polygon points
static std::optional<POINT> linePreviewStart; // For line preview

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int x1, y1, x2, y2;
    static vector<double> points;
    static int counter = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        // Set white background
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(255,255,255)));
        // Set custom cursor
        HCURSOR hCursor = LoadCursor(NULL, IDC_CROSS);
        SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
        // Create menu
        HMENU hMenuBar = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        AppendMenu(hFileMenu, MF_STRING, 1001, "Save");
        AppendMenu(hFileMenu, MF_STRING, 1002, "Load");
        AppendMenu(hFileMenu, MF_STRING, 1003, "Clear");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, "File");
        // Shape menu
        HMENU hShapeMenu = CreatePopupMenu();
        AppendMenu(hShapeMenu, MF_STRING, 2001, "Line");
        AppendMenu(hShapeMenu, MF_STRING, 2002, "Circle");
        AppendMenu(hShapeMenu, MF_STRING, 2003, "Ellipse");
        AppendMenu(hShapeMenu, MF_STRING, 2004, "Rectangle");
        AppendMenu(hShapeMenu, MF_STRING, 2005, "Polygon");
        AppendMenu(hShapeMenu, MF_STRING, 2006, "Spline");
        AppendMenu(hShapeMenu, MF_STRING, 2007, "Clipping");
        AppendMenu(hShapeMenu, MF_STRING, 2008, "Filling");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hShapeMenu, "Draw Shape");
        // Algorithm submenus
        HMENU hLineMenu = CreatePopupMenu();
        AppendMenu(hLineMenu, MF_STRING, 3001, "DDA");
        AppendMenu(hLineMenu, MF_STRING, 3002, "Midpoint");
        AppendMenu(hLineMenu, MF_STRING, 3003, "Parametric");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hLineMenu, "Line Algorithm");
        HMENU hCircleMenu = CreatePopupMenu();
        AppendMenu(hCircleMenu, MF_STRING, 4001, "Direct");
        AppendMenu(hCircleMenu, MF_STRING, 4002, "Polar");
        AppendMenu(hCircleMenu, MF_STRING, 4003, "Iterative Polar");
        AppendMenu(hCircleMenu, MF_STRING, 4004, "Midpoint");
        AppendMenu(hCircleMenu, MF_STRING, 4005, "Modified Midpoint");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hCircleMenu, "Circle Algorithm");
        HMENU hEllipseMenu = CreatePopupMenu();
        AppendMenu(hEllipseMenu, MF_STRING, 5001, "Direct");
        AppendMenu(hEllipseMenu, MF_STRING, 5002, "Polar");
        AppendMenu(hEllipseMenu, MF_STRING, 5003, "Midpoint");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEllipseMenu, "Ellipse Algorithm");
        // Color menu
        HMENU hColorMenu = CreatePopupMenu();
        AppendMenu(hColorMenu, MF_STRING, 6001, "Choose Color");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hColorMenu, "Color");
        // Help menu
        HMENU hHelpMenu = CreatePopupMenu();
        AppendMenu(hHelpMenu, MF_STRING, 7001, "Manual / Help");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, "Help");
        SetMenu(hWnd, hMenuBar);
    }
    break;
    case WM_COMMAND:
        {
            int id = LOWORD(wParam);
            // File menu
            if (id == 1001) { /* Save logic (not implemented) */ }
            else if (id == 1002) { /* Load logic (not implemented) */ }
            else if (id == 1003) {
                // Clear all layers and previews, reset state, and redraw
                layers.clear();
                userPoints.clear();
                currentPolygon.reset();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // Shape menu
            else if (id == 2001) { 
                currentShape = SHAPE_LINE; 
                userPoints.clear(); 
                currentPolygon.reset(); 
                linePreviewStart.reset(); // Cancel any preview
            }
            else if (id == 2002) { currentShape = SHAPE_CIRCLE; userPoints.clear(); currentPolygon.reset(); }
            else if (id == 2003) { currentShape = SHAPE_ELLIPSE; userPoints.clear(); currentPolygon.reset(); }
            else if (id == 2004) { currentShape = SHAPE_RECT; userPoints.clear(); currentPolygon.reset(); }
            else if (id == 2005) {
                currentShape = SHAPE_POLYGON;
                userPoints.clear();
                // Always start a new polygon with the current color
                currentPolygon = LayerPolygon{{}, currentColor};
            }
            else if (id == 2006) { // Spline/Curve
                currentShape = SHAPE_SPLINE;
                userPoints.clear();
                currentPolygon.reset();
                linePreviewStart.reset();
            }
            else if (id == 2007) { // Clipping
                currentShape = SHAPE_CLIP;
                userPoints.clear();
                currentPolygon.reset();
                linePreviewStart.reset();
            }
            else if (id == 7001) { // Show manual
                MessageBox(hWnd,
                    "How to use the Graphics Project:\n\n"
                    "- Select a shape or tool from the 'Draw Shape' menu.\n"
                    "- For lines: Left-click to set start, then end point.\n"
                    "- For polygons: Left-click to add points, right-click to finish.\n"
                    "- For circles/ellipses/rectangles: Left-click two points.\n"
                    "- For curves: Select Hermite or Bezier, then click required control points.\n"
                    "- For clipping: Select 'Clipping', then left-click two corners of the rectangle. All lines will be clipped to this window.\n"
                    "- Use the 'Color' menu to change drawing color.\n"
                    "- Use 'Clear' to erase all.\n\n"
                    "Tip: Right-click cancels in-progress lines or polygons.",
                    "Manual / Help", MB_OK | MB_ICONINFORMATION);
            }
            // Algorithm menus
            else if (id >= 3001 && id <= 3003) { currentLineAlg = (LineAlgorithm)(id - 3001); }
            else if (id >= 4001 && id <= 4005) { currentCircleAlg = (CircleAlgorithm)(id - 4001); }
            else if (id >= 5001 && id <= 5003) { currentEllipseAlg = (EllipseAlgorithm)(id - 5001); }
            // Color menu
            else if (id == 6001) {
                CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
                static COLORREF custColors[16] = {0};
                cc.hwndOwner = hWnd;
                cc.lpCustColors = custColors;
                cc.rgbResult = currentColor;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColor(&cc)) {
                    currentColor = cc.rgbResult;
                    // Update in-progress polygon color if any
                    if (currentPolygon) currentPolygon->color = currentColor;
                }
            }
        }
        break;

    case WM_LBUTTONDOWN:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            HDC hdc = GetDC(hWnd); // Fix: declare hdc for curve drawing
            if (currentShape == SHAPE_CLIP) {
                userPoints.push_back({x, y});
                if (userPoints.size() == 2) {
                    // Set the clipping window boundaries
                    int xmin = min(userPoints[0].x, userPoints[1].x);
                    int xmax = max(userPoints[0].x, userPoints[1].x);
                    int ymin = min(userPoints[0].y, userPoints[1].y);
                    int ymax = max(userPoints[0].y, userPoints[1].y);
                    Clipping::SetClipWindow(xmin, ymin, xmax, ymax);
                    std::vector<Layer> newLayers;
                    for (const auto& layer : layers) {
                        if (auto* line = std::get_if<LayerLine>(&layer.shape)) {
                            int cx0 = line->p1.x, cy0 = line->p1.y, cx1 = line->p2.x, cy1 = line->p2.y;
                            int code1 = Clipping::computeCode(cx0, cy0);
                            int code2 = Clipping::computeCode(cx1, cy1);
                            bool accept = false;
                            while (true) {
                                if ((code1 | code2) == 0) { accept = true; break; }
                                else if (code1 & code2) { break; }
                                int codeOut = code1 ? code1 : code2;
                                int x, y;
                                if (codeOut & Clipping::TOP) {
                                    x = cx0 + (cx1 - cx0) * (Clipping::CLIP_Y_MAX - cy0) / (cy1 - cy0);
                                    y = Clipping::CLIP_Y_MAX;
                                } else if (codeOut & Clipping::BOTTOM) {
                                    x = cx0 + (cx1 - cx0) * (Clipping::CLIP_Y_MIN - cy0) / (cy1 - cy0);
                                    y = Clipping::CLIP_Y_MIN;
                                } else if (codeOut & Clipping::RIGHT) {
                                    y = cy0 + (cy1 - cy0) * (Clipping::CLIP_X_MAX - cx0) / (cx1 - cx0);
                                    x = Clipping::CLIP_X_MAX;
                                } else { // LEFT
                                    y = cy0 + (cy1 - cy0) * (Clipping::CLIP_X_MIN - cx0) / (cx1 - cx0);
                                    x = Clipping::CLIP_X_MIN;
                                }
                                if (codeOut == code1) { cx0 = x; cy0 = y; code1 = Clipping::computeCode(cx0, cy0); }
                                else { cx1 = x; cy1 = y; code2 = Clipping::computeCode(cx1, cy1); }
                            }
                            if (accept) {
                                newLayers.push_back(Layer{LayerLine{{cx0, cy0}, {cx1, cy1}, line->color, line->alg}});
                            }
                        } else {
                            newLayers.push_back(layer); // Keep other shapes as-is
                        }
                    }
                    layers = std::move(newLayers);
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            else if (currentShape == SHAPE_POLYGON) {
                // Always use the current color for the preview polygon
                if (!currentPolygon) currentPolygon = LayerPolygon{{}, currentColor};
                currentPolygon->color = currentColor;
                currentPolygon->pts.push_back({x, y});
                InvalidateRect(hWnd, NULL, TRUE);
            } else if (currentShape == SHAPE_LINE) {
                if (!linePreviewStart) {
                    linePreviewStart = POINT{x, y};
                } else {
                    layers.push_back(Layer{LayerLine{*linePreviewStart, POINT{x, y}, currentColor, currentLineAlg}});
                    linePreviewStart.reset();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            } else if (currentShape == SHAPE_CIRCLE) {
                userPoints.push_back({x, y});
                if (userPoints.size() == 2) {
                    int r = (int)hypot(userPoints[1].x - userPoints[0].x, userPoints[1].y - userPoints[0].y);
                    layers.push_back(Layer{LayerCircle{userPoints[0], r, currentColor, currentCircleAlg}});
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            } else if (currentShape == SHAPE_ELLIPSE) {
                userPoints.push_back({x, y});
                if (userPoints.size() == 2) {
                    int a = abs(userPoints[1].x - userPoints[0].x);
                    int b = abs(userPoints[1].y - userPoints[0].y);
                    layers.push_back(Layer{LayerEllipse{userPoints[0], a, b, currentColor, currentEllipseAlg}});
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            } else if (currentShape == SHAPE_RECT) {
                userPoints.push_back({x, y});
                if (userPoints.size() == 2) {
                    layers.push_back(Layer{LayerRect{userPoints[0], userPoints[1], currentColor}});
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            } else if (currentShape == SHAPE_SPLINE) {
                userPoints.push_back({x, y});
                if (userPoints.size() == 4) { // Bezier: 4 control points
                    ThirdDegreeCurve::BezierCurve(hdc, userPoints[0].x, userPoints[0].y, userPoints[1].x, userPoints[1].y,
                        userPoints[2].x, userPoints[2].y, userPoints[3].x, userPoints[3].y, currentColor, currentColor, false);
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            ReleaseDC(hWnd, hdc); // Release DC if acquired
        }
        break;
    case WM_MOUSEMOVE:
        {
            if (currentShape == SHAPE_LINE && linePreviewStart) {
                InvalidateRect(hWnd, NULL, FALSE); // Redraw for preview
            }
        }
        break;
    case WM_RBUTTONDOWN:
        {
            if (currentShape == SHAPE_POLYGON && currentPolygon && currentPolygon->pts.size() >= 3) {
                // Finalize the polygon: add as a new layer
                layers.push_back(Layer{*currentPolygon});
                currentPolygon.reset();
                InvalidateRect(hWnd, NULL, TRUE);
            } else if (currentShape == SHAPE_LINE && linePreviewStart) {
                // Cancel in-progress line
                linePreviewStart.reset();
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            for (const auto& layer : layers) {
                std::visit([&](auto&& shape) {
                    using T = std::decay_t<decltype(shape)>;
                    if constexpr (std::is_same_v<T, LayerLine>) {
                        if (shape.alg == LINE_DDA)
                            Lines::LineBresenhamDDA(hdc, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color);
                        else if (shape.alg == LINE_MIDPOINT)
                            Lines::DrawLineByMidPoint(hdc, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color);
                        else if (shape.alg == LINE_PARAMETRIC)
                            ; // TODO: Add parametric line drawing here
                    } else if constexpr (std::is_same_v<T, LayerCircle>) {
                        if (shape.alg == CIRCLE_POLAR)
                            SecondDegreeCurve::DrawCircle(hdc, shape.center.x, shape.center.y, shape.center.x + shape.r, shape.center.y, shape.color);
                        else if (shape.alg == CIRCLE_MIDPOINT)
                            SecondDegreeCurve::BresenhamCircle(hdc, shape.center.x, shape.center.y, shape.r, shape.color);
                        // else if (shape.alg == CIRCLE_DIRECT || ...)
                    } else if constexpr (std::is_same_v<T, LayerEllipse>) {
                        if (shape.alg == ELLIPSE_DIRECT)
                            Ellipse::DrawEllipseEquation(hdc, shape.center.x, shape.center.y, shape.a, shape.b, shape.color);
                        else if (shape.alg == ELLIPSE_POLAR)
                            Ellipse::DrawEllipsePolar(hdc, shape.center.x, shape.center.y, shape.a, shape.b, shape.color);
                        else if (shape.alg == ELLIPSE_MIDPOINT)
                            Ellipse::DrawEllipseMidPoint(hdc, shape.center.x, shape.center.y, shape.a, shape.b, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerRect>) {
                        Rectangle(hdc, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y);
                    } else if constexpr (std::is_same_v<T, LayerPolygon>) {
                        if (shape.pts.size() >= 3) {
                            HPEN hPen = CreatePen(PS_SOLID, 1, shape.color);
                            HGDIOBJ oldPen = SelectObject(hdc, hPen);
                            Polygon(hdc, shape.pts.data(), (int)shape.pts.size());
                            SelectObject(hdc, oldPen);
                            DeleteObject(hPen);
                        }
                    }
                }, layer.shape);
            }
            // Draw current polygon preview (always use current color)
            if (currentPolygon && currentPolygon->pts.size() > 1) {
                HPEN hPen = CreatePen(PS_DOT, 1, currentPolygon->color);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                Polyline(hdc, currentPolygon->pts.data(), (int)currentPolygon->pts.size());
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            // Draw line preview if in progress
            if (currentShape == SHAPE_LINE && linePreviewStart) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                HPEN hPen = CreatePen(PS_DOT, 1, currentColor);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                MoveToEx(hdc, linePreviewStart->x, linePreviewStart->y, NULL);
                LineTo(hdc, pt.x, pt.y);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            // Draw Hermite/Bezier preview if needed (future extension)
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