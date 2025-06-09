#undef UNICODE
#undef _UNICODE
#include "../include/common.h"
#include "../include/filling.h"
#include "../include/curves_third_degree.h"
#include "../include/curves_second_degree.h"
#include "../include/lines.h"
#include "../include/tasks_and_assignments.h"
#include "../include/import.h"
#include "../include/ellipse.h"
#include "../include/clipping.h"
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <variant>
#include <optional>
#include <cmath>

using namespace std;

// ===== Drawing Modes and Shape Types =====
enum DrawingMode {
    MODE_NONE,
    MODE_BEZIER_CURVE,
    MODE_LINE_DDA,
    MODE_CIRCLE,
    MODE_HERMITE_CURVE,
    MODE_FLOOD_FILL,
    MODE_INTERPOLATED_LINE,
    MODE_ELLIPSE_MIDPOINT,
    MODE_ELLIPSE_POLAR,
    MODE_ELLIPSE_EQUATION
};

enum MenuShapeType {
    SHAPE_NONE,
    SHAPE_LINE,
    SHAPE_CIRCLE,
    SHAPE_ELLIPSE,
    SHAPE_RECT,
    SHAPE_POLYGON,
    SHAPE_SPLINE,
    SHAPE_CLIP,
    SHAPE_FILL,
    SHAPE_POINT
};

// ===== Drawing Algorithm Types =====
enum LineAlgorithm { LINE_DDA, LINE_MIDPOINT, LINE_PARAMETRIC };
enum CircleAlgorithm { CIRCLE_DIRECT, CIRCLE_POLAR, CIRCLE_ITER_POLAR, CIRCLE_MIDPOINT, CIRCLE_MOD_MIDPOINT };
enum EllipseAlgorithm { ELLIPSE_DIRECT, ELLIPSE_POLAR, ELLIPSE_MIDPOINT };
enum ClippingWindowType {
    CLIP_RECTANGLE,
    CLIP_SQUARE
};

// ===== Global Variables =====
static DrawingMode currentMode = MODE_NONE;
static HWND hwndButtons[11];
static const int BUTTON_WIDTH = 150;
static const int BUTTON_HEIGHT = 30;
static const int BUTTON_SPACING = 10;
static const int DRAWING_AREA_LEFT = BUTTON_WIDTH + 20;

static MenuShapeType currentShape = SHAPE_NONE;
static LineAlgorithm currentLineAlg = LINE_DDA;
static CircleAlgorithm currentCircleAlg = CIRCLE_DIRECT;
static EllipseAlgorithm currentEllipseAlg = ELLIPSE_DIRECT;
static ClippingWindowType currentClipWindowType = CLIP_RECTANGLE;
static COLORREF currentColor = RGB(0,0,0);
static std::vector<POINT> userPoints;
static std::vector<std::string> drawnShapes;

// ===== Layer System =====
struct LayerLine { POINT p1, p2; COLORREF color; LineAlgorithm alg; };
struct LayerCircle { POINT center; int r; COLORREF color; CircleAlgorithm alg; };
struct LayerEllipse { POINT center; int a, b; COLORREF color; EllipseAlgorithm alg; };
struct LayerRect { POINT p1, p2; COLORREF color; };
struct LayerPolygon { std::vector<POINT> pts; COLORREF color; };
struct LayerPoint { POINT pt; COLORREF color; };
using LayerShape = std::variant<LayerLine, LayerCircle, LayerEllipse, LayerRect, LayerPolygon, LayerPoint>;

struct Layer {
    LayerShape shape;
};

// ===== State Management =====
static std::vector<Layer> layers;
static std::optional<LayerPolygon> currentPolygon;
static std::optional<POINT> linePreviewStart;
static std::optional<POINT> linePreviewCurrent;
static std::optional<POINT> clipWindowStart;
static std::optional<POINT> clipWindowCurrent;
static std::optional<LayerPolygon> originalPolygonLayer;
static std::optional<LayerPolygon> lastPolygonClipped;

// ===== Utility Functions =====
void log_debug(const std::string& msg) {
    std::ofstream log("clipping_debug.log", std::ios::app);
    log << msg << std::endl;
}

// Function to draw a polygon using line algorithms
void DrawPolygon(HDC hdc, const std::vector<POINT>& points, COLORREF color) {
    if (points.size() < 2) return;

    // Draw lines between consecutive points using midpoint algorithm
    for (size_t i = 0; i < points.size() - 1; i++) {
        Lines::DrawLineByMidPoint(hdc, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color);
    }
}

// ===== Window Procedure =====
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int x1, y1, x2, y2;
    static vector<double> points;
    static int counter = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        // Initialize window properties
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(255,255,255)));
        HCURSOR hCursor = LoadCursor(NULL, IDC_CROSS);
        SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)hCursor);

        // Create menu structure
        HMENU hMenuBar = CreateMenu();
        
        // File menu
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
        AppendMenu(hShapeMenu, MF_STRING, 2009, "Point");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hShapeMenu, "Draw Shape");

        // Algorithm menus
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

        // Clipping window type menu
        HMENU hClipTypeMenu = CreatePopupMenu();
        AppendMenu(hClipTypeMenu, MF_STRING, 8001, "Rectangle Window");
        AppendMenu(hClipTypeMenu, MF_STRING, 8002, "Square Window");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hClipTypeMenu, "Clipping Window Type");

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
            // File menu handlers
            if (id == 1001) { /* Save logic (not implemented) */ }
            else if (id == 1002) { /* Load logic (not implemented) */ }
            else if (id == 1003) {
                // Clear all layers and reset state
                layers.clear();
                userPoints.clear();
                currentPolygon.reset();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // Shape menu handlers
            else if (id == 2001) { 
                currentShape = SHAPE_LINE; 
                userPoints.clear(); 
                currentPolygon.reset(); 
                linePreviewStart.reset();
                linePreviewCurrent.reset();
            }
            else if (id == 2002) { currentShape = SHAPE_CIRCLE; userPoints.clear(); currentPolygon.reset(); }
            else if (id == 2003) { currentShape = SHAPE_ELLIPSE; userPoints.clear(); currentPolygon.reset(); }
            else if (id == 2004) { currentShape = SHAPE_RECT; userPoints.clear(); currentPolygon.reset(); }
            else if (id == 2005) {
                currentShape = SHAPE_POLYGON;
                userPoints.clear();
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
                linePreviewCurrent.reset();
                clipWindowStart.reset();
                clipWindowCurrent.reset();
            }
            else if (id == 2009) { // Point
                currentShape = SHAPE_POINT;
                userPoints.clear();
                currentPolygon.reset();
                linePreviewStart.reset();
                linePreviewCurrent.reset();
            }
            // Help menu handler
            else if (id == 7001) {
                MessageBox(hWnd,
                    "How to use the Graphics Project:\n\n"
                    "- Select a shape or tool from the 'Draw Shape' menu.\n"
                    "- For lines: Left-click to set start, then end point.\n"
                    "- For polygons: Left-click to add points, right-click to finish.\n"
                    "- For circles/ellipses/rectangles: Left-click two points.\n"
                    "- For curves: Select Hermite or Bezier, then click required control points.\n"
                    "- For clipping: Select 'Clipping', then left-click two corners of the window.\n"
                    "  Use the 'Clipping Window Type' menu to choose Rectangle or Square.\n"
                    "  Rectangle window supports line, point, and polygon clipping.\n"
                    "  Square window supports only line and point clipping.\n"
                    "- Use the 'Color' menu to change drawing color.\n"
                    "- Use 'Clear' to erase all.\n\n"
                    "Tip: Right-click cancels in-progress lines or polygons.\n\n"
                    "Note: Only lines, points, and (for rectangle window) polygons are clipped. Other shapes are ignored.",
                    "Manual / Help", MB_OK | MB_ICONINFORMATION);
            }
            // Algorithm menu handlers
            else if (id >= 3001 && id <= 3003) { currentLineAlg = (LineAlgorithm)(id - 3001); }
            else if (id >= 4001 && id <= 4005) { currentCircleAlg = (CircleAlgorithm)(id - 4001); }
            else if (id >= 5001 && id <= 5003) { currentEllipseAlg = (EllipseAlgorithm)(id - 5001); }
            // Color menu handler
            else if (id == 6001) {
                CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
                static COLORREF custColors[16] = {0};
                cc.hwndOwner = hWnd;
                cc.lpCustColors = custColors;
                cc.rgbResult = currentColor;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColor(&cc)) {
                    currentColor = cc.rgbResult;
                    if (currentPolygon) currentPolygon->color = currentColor;
                }
            }
            // Clipping window type handlers
            else if (id == 8001) { currentClipWindowType = CLIP_RECTANGLE; }
            else if (id == 8002) { currentClipWindowType = CLIP_SQUARE; }
        }
        break;

    case WM_LBUTTONDOWN:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // Handle clipping window creation
            if (currentShape == SHAPE_CLIP) {
                if (!clipWindowStart) {
                    clipWindowStart = POINT{x, y};
                    clipWindowCurrent = POINT{x, y};
                } else {
                    // Create clipping window from two points
                    int x0 = clipWindowStart->x;
                    int y0 = clipWindowStart->y;
                    int x1 = x;
                    int y1 = y;

                    // Calculate window boundaries
                    int xmin = std::min(x0, x1);
                    int xmax = std::max(x0, x1);
                    int ymin = std::min(y0, y1);
                    int ymax = std::max(y0, y1);

                    // Adjust for square window if needed
                    if (currentClipWindowType == CLIP_SQUARE) {
                        int dx = x1 - x0;
                        int dy = y1 - y0;
                        int side = max(abs(dx), abs(dy));

                        if (dx >= 0 && dy >= 0) { 
                            xmax = x0 + side; ymax = y0 + side; 
                        } else if (dx < 0 && dy >= 0) { 
                            xmin = x0 - side; xmax = x0; ymax = y0 + side; 
                        } else if (dx >= 0 && dy < 0) { 
                            xmax = x0 + side; ymin = y0 - side; ymax = y0; 
                        } else { 
                            xmin = x0 - side; ymin = y0 - side; xmax = x0; ymax = y0; 
                        }
                    }

                    // Set clipping window
                    Clipping::SetClipWindow(xmin, ymin, xmax, ymax);

                    // Only process the last layer if it exists
                    if (!layers.empty()) {
                        Layer& lastLayer = layers.back();
                        std::visit([&](auto&& shape) {
                            using T = std::decay_t<decltype(shape)>;
                            if constexpr (std::is_same_v<T, LayerLine>) {
                                // For lines, create a new clipped line
                                int x1 = shape.p1.x, y1 = shape.p1.y;
                                int x2 = shape.p2.x, y2 = shape.p2.y;
                                int code1 = Clipping::computeCode(x1, y1);
                                int code2 = Clipping::computeCode(x2, y2);
                                bool accept = false;
                                
                                while (true) {
                                    if ((code1 | code2) == 0) {
                                        accept = true;
                                        break;
                                    } else if (code1 & code2) {
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
                                    layers.back() = Layer{LayerLine{{x1, y1}, {x2, y2}, shape.color, shape.alg}};
                                } else {
                                    layers.pop_back(); // Remove the line if it's completely outside
                                }
                            } else if constexpr (std::is_same_v<T, LayerPolygon>) {
                                if (shape.pts.size() >= 3) {
                                    std::vector<POINT> clipped = Clipping::SutherlandHodgmanClip(shape.pts.data(), (int)shape.pts.size());
                                    if (clipped.size() >= 3) {
                                        // Replace the last layer with the clipped version
                                        layers.back() = Layer{LayerPolygon{clipped, shape.color}};
                                    } else {
                                        layers.pop_back(); // Remove the polygon if it's completely outside
                                    }
                                }
                            } else if constexpr (std::is_same_v<T, LayerPoint>) {
                                // For points, check if they're inside the clipping window
                                if (shape.pt.x >= Clipping::CLIP_X_MIN && shape.pt.x <= Clipping::CLIP_X_MAX &&
                                    shape.pt.y >= Clipping::CLIP_Y_MIN && shape.pt.y <= Clipping::CLIP_Y_MAX) {
                                    // Point is inside, keep it as is
                                } else {
                                    // Point is outside, remove it
                                    layers.pop_back();
                                }
                            }
                        }, lastLayer.shape);
                    }

                    // Reset state
                    userPoints.clear();
                    clipWindowStart.reset();
                    clipWindowCurrent.reset();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            // Handle polygon point addition
            else if (currentShape == SHAPE_POLYGON) {
                if (!currentPolygon) {
                    currentPolygon = LayerPolygon{{}, currentColor};
                }
                currentPolygon->color = currentColor;
                
                POINT newPoint = {x, y};
                std::vector<POINT> testPoints = currentPolygon->pts;
                testPoints.push_back(newPoint);
                
                if (Common::isValidPolygon(testPoints)) {
                    currentPolygon->pts.push_back(newPoint);
                } else {
                    MessageBox(hWnd, "This point is too close to existing points. Please choose a different point.", 
                              "Invalid Point", MB_OK | MB_ICONWARNING);
                }
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // Handle line drawing
            else if (currentShape == SHAPE_LINE) {
                if (!linePreviewStart) {
                    linePreviewStart = POINT{x, y};
                    linePreviewCurrent = POINT{x, y};
                } else {
                    layers.push_back(Layer{LayerLine{*linePreviewStart, POINT{x, y}, currentColor, currentLineAlg}});
                    linePreviewStart.reset();
                    linePreviewCurrent.reset();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            // Handle circle drawing
            else if (currentShape == SHAPE_CIRCLE) {
                userPoints.push_back(POINT{x, y});
                if (userPoints.size() == 2) {
                    int r = (int)hypot(userPoints[1].x - userPoints[0].x, userPoints[1].y - userPoints[0].y);
                    layers.push_back(Layer{LayerCircle{userPoints[0], r, currentColor, currentCircleAlg}});
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            // Handle ellipse drawing
            else if (currentShape == SHAPE_ELLIPSE) {
                userPoints.push_back(POINT{x, y});
                if (userPoints.size() == 2) {
                    int a = abs(userPoints[1].x - userPoints[0].x);
                    int b = abs(userPoints[1].y - userPoints[0].y);
                    layers.push_back(Layer{LayerEllipse{userPoints[0], a, b, currentColor, currentEllipseAlg}});
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            // Handle rectangle drawing
            else if (currentShape == SHAPE_RECT) {
                userPoints.push_back(POINT{x, y});
                if (userPoints.size() == 2) {
                    layers.push_back(Layer{LayerRect{userPoints[0], userPoints[1], currentColor}});
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            // Handle spline/curve drawing
            else if (currentShape == SHAPE_SPLINE) {
                userPoints.push_back(POINT{x, y});
                if (userPoints.size() == 4) {
                    HDC hdc = GetDC(hWnd);
                    ThirdDegreeCurve::BezierCurve(hdc, userPoints[0].x, userPoints[0].y, userPoints[1].x, userPoints[1].y,
                        userPoints[2].x, userPoints[2].y, userPoints[3].x, userPoints[3].y, currentColor, currentColor, false);
                    userPoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
        }
        break;

    case WM_MOUSEMOVE:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            // Update preview positions
            if (currentShape == SHAPE_LINE && linePreviewStart) {
                linePreviewCurrent = POINT{x, y};
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (currentShape == SHAPE_CLIP && clipWindowStart) {
                clipWindowCurrent = POINT{x, y};
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;

    case WM_RBUTTONDOWN:
        {
            // Handle polygon completion
            if (currentShape == SHAPE_POLYGON && currentPolygon && currentPolygon->pts.size() >= 3) {
                if (currentPolygon->pts.front().x != currentPolygon->pts.back().x ||
                    currentPolygon->pts.front().y != currentPolygon->pts.back().y) {
                    currentPolygon->pts.push_back(currentPolygon->pts.front());
                }
                
                layers.push_back(Layer{*currentPolygon});
                originalPolygonLayer = *currentPolygon;
                currentPolygon.reset();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // Cancel line preview
            else if (currentShape == SHAPE_LINE && linePreviewStart) {
                linePreviewStart.reset();
                linePreviewCurrent.reset();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // Cancel clipping window preview
            else if (currentShape == SHAPE_CLIP && clipWindowStart) {
                clipWindowStart.reset();
                clipWindowCurrent.reset();
                userPoints.clear();
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Draw all layers
            for (const auto& layer : layers) {
                std::visit([&](auto&& shape) {
                    using T = std::decay_t<decltype(shape)>;
                    if constexpr (std::is_same_v<T, LayerLine>) {
                        if (shape.alg == LINE_DDA)
                            Lines::LineBresenhamDDA(hdc, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color);
                        else if (shape.alg == LINE_MIDPOINT)
                            Lines::DrawLineByMidPoint(hdc, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerCircle>) {
                        if (shape.alg == CIRCLE_POLAR)
                            SecondDegreeCurve::DrawCircle(hdc, shape.center.x, shape.center.y, shape.center.x + shape.r, shape.center.y, shape.color);
                        else if (shape.alg == CIRCLE_MIDPOINT)
                            SecondDegreeCurve::BresenhamCircle(hdc, shape.center.x, shape.center.y, shape.r, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerEllipse>) {
                        if (shape.alg == ELLIPSE_DIRECT)
                            Ellipse::DrawEllipseEquation(hdc, shape.center.x, shape.center.y, shape.a, shape.b, shape.color);
                        else if (shape.alg == ELLIPSE_POLAR)
                            Ellipse::DrawEllipsePolar(hdc, shape.center.x, shape.center.y, shape.a, shape.b, shape.color);
                        else if (shape.alg == ELLIPSE_MIDPOINT)
                            Ellipse::DrawEllipseMidPoint(hdc, shape.center.x, shape.center.y, shape.a, shape.b, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerRect>) {
                        POINT rectPoints[5] = {
                            shape.p1,
                            {shape.p2.x, shape.p1.y},
                            shape.p2,
                            {shape.p1.x, shape.p2.y},
                            shape.p1
                        };
                        DrawPolygon(hdc, std::vector<POINT>(rectPoints, rectPoints + 5), shape.color);
                    } else if constexpr (std::is_same_v<T, LayerPolygon>) {
                        if (shape.pts.size() >= 2) {
                            DrawPolygon(hdc, shape.pts, shape.color);
                        }
                    } else if constexpr (std::is_same_v<T, LayerPoint>) {
                        SetPixel(hdc, shape.pt.x, shape.pt.y, shape.color);
                    }
                }, layer.shape);
            }

            // Draw previews
            if (currentPolygon && currentPolygon->pts.size() > 1) {
                DrawPolygon(hdc, currentPolygon->pts, currentPolygon->color);
            }
            if (currentShape == SHAPE_LINE && linePreviewStart && linePreviewCurrent) {
                HPEN hPen = CreatePen(PS_DOT, 1, currentColor);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                MoveToEx(hdc, linePreviewStart->x, linePreviewStart->y, NULL);
                LineTo(hdc, linePreviewCurrent->x, linePreviewCurrent->y);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            if (currentShape == SHAPE_CLIP && clipWindowStart && clipWindowCurrent) {
                int x0 = clipWindowStart->x, y0 = clipWindowStart->y;
                int x1 = clipWindowCurrent->x, y1 = clipWindowCurrent->y;
                if (currentClipWindowType == CLIP_SQUARE) {
                    int side = max(abs(x1 - x0), abs(y1 - y0));
                    x1 = x0 + (x1 >= x0 ? side : -side);
                    y1 = y0 + (y1 >= y0 ? side : -side);
                }
                HPEN hPen = CreatePen(PS_DOT, 1, RGB(0,0,255));
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                Rectangle(hdc, x0, y0, x1, y1);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }

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

// ===== Main Entry Point =====
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = "GraphicsClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "GraphicsClass", "Graphics Project",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800 + DRAWING_AREA_LEFT, 600,
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