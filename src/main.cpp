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
#include "../include/storage.h"
#include "../include/layer.h"
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <variant>
#include <optional>
#include <cmath>

using namespace std;

// ===== Drawing Modes and Shape Types =====
// DrawingMode: Internal logic for current drawing operation
// MenuShapeType: Tracks which shape/tool is currently active from the menu
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

// ===== Menu Shape Types =====
// Used to track which shape/tool is currently active from the menu
enum MenuShapeType {
    SHAPE_NONE,                      // No shape/tool selected
    SHAPE_LINE,                      // Line drawing mode
    SHAPE_CIRCLE,                    // Circle drawing mode
    SHAPE_ELLIPSE,                   // Ellipse drawing mode
    SHAPE_RECT,                      // Rectangle drawing mode
    SHAPE_POLYGON,                   // Polygon drawing mode
    SHAPE_SPLINE,                    // Spline/curve drawing mode
    SHAPE_CLIP,                      // Clipping window/tool mode
    SHAPE_FILL,                      // Filling mode
    SHAPE_POINT,                     // Single point drawing mode
    SHAPE_EXTRA_QUARTER_CIRCLES,     // Extra: Quarter circles filling mode
    SHAPE_EXTRA_RECT_BEZIER_WAVES,   // Extra: Rectangle Bezier waves mode
    SHAPE_EXTRA_CIRCLE_QUARTER,      // Extra: Circle quarter filling mode
    SHAPE_EXTRA_SQUARE_HERMITE_WAVES,  // Extra: Square Hermite waves mode
    SHAPE_CARDINAL_SPLINE             // Cardinal Spline drawing mode
};

// ===== Drawing Algorithm Types =====
// These enums define the algorithms used for drawing lines, circles, ellipses, and filling
// They are used to select the appropriate algorithm based on user input or menu selection
// (Moved to common.h)

//______________________________________________________
// ============= Global State Variables ==============
//______________________________________________________


// Current drawing mode (not used for menu, for internal logic)
static DrawingMode currentMode = MODE_NONE;

// Handles for UI buttons (if any are created)
static HWND hwndButtons[11];

// Button layout constants (not used in this code, but may be for future UI)
static const int BUTTON_WIDTH = 150;
static const int BUTTON_HEIGHT = 30;
static const int BUTTON_SPACING = 10;

// Left margin for drawing area (used in window creation)
static const int DRAWING_AREA_LEFT = BUTTON_WIDTH + 20;

// Currently selected shape/tool from menu
static MenuShapeType currentShape = SHAPE_NONE;

// Currently selected algorithms for each shape type
static LineAlgorithm currentLineAlg = LINE_DDA;
static CircleAlgorithm currentCircleAlg = CIRCLE_DIRECT;
static EllipseAlgorithm currentEllipseAlg = ELLIPSE_DIRECT;

// Currently selected clipping window type (rectangle or square)
static ClippingWindowType currentClipWindowType = CLIP_RECTANGLE;

// Currently selected drawing color
static COLORREF currentColor = RGB(0,0,0);

// Stores user input points for interactive drawing
static std::vector<POINT> userPoints;

// Stores names/types of drawn shapes (not used in this code, but may be for undo/history)
static std::vector<std::string> drawnShapes;

// Currently selected filling algorithm
static FillAlgorithm currentFillAlg = FILL_RECURSIVE_FLOOD;

// Stores the point where filling should start (if any)
static std::optional<POINT> fillPoint;

// ===== Layer System =====
// Each Layer struct represents a drawable object with its parameters and color
// LayerShape is a variant holding any possible drawable type
// using LayerShape = std::variant<LayerLine, LayerCircle, LayerEllipse, LayerRect, LayerPolygon, LayerPoint, LayerFill, LayerQuarterCircleFilling, LayerRectangleBezierWaves, LayerCircleQuarter, LayerSquareHermiteWaves, LayerBezierCurve, LayerCardinalSpline>;

// ===== State Management =====
// These variables track the current drawing state, previews, and extra modes
static std::vector<Layer> layers;
static std::optional<LayerPolygon> currentPolygon;
static std::optional<POINT> linePreviewStart;
static std::optional<POINT> linePreviewCurrent;
static std::optional<POINT> clipWindowStart;
static std::optional<POINT> clipWindowCurrent;
static std::optional<LayerPolygon> originalPolygonLayer;
static std::optional<LayerPolygon> lastPolygonClipped;
// Extra: state for quarter circles filling
static bool extraQuarterCircleActive = false;
static int extraQuarterStage = 0; // 0: none, 1: center set, 2: radius set
static POINT extraQuarterCenter = {0,0};
static int extraQuarterRadius = 0;
static int extraQuarterQuarter = 1;
// Extra: state for rectangle Bezier waves
static bool extraRectBezierActive = false;
static int extraRectBezierStage = 0; // 0: none, 1: first corner set
static POINT extraRectBezierP1 = {0,0};
static POINT extraRectBezierP2 = {0,0};
// Extra: state for circle quarter filling
static bool extraCircleQuarterActive = false;
static int extraCircleQuarterStage = 0; // 0: none, 1: center set, 2: radius set
static POINT extraCircleQuarterCenter = {0,0};
static int extraCircleQuarterRadius = 0;
static int extraCircleQuarterQuarter = 1;
// Extra: state for square Hermite waves
static bool extraSquareHermiteActive = false;
static int extraSquareHermiteStage = 0; // 0: none, 1: top-left set
static POINT extraSquareHermiteTopLeft = {0,0};
static int extraSquareHermiteSize = 0;

// ===== Utility Functions =====
// log_debug: Writes debug messages to a file for troubleshooting
// DrawPolygon: Draws a closed polygon using the midpoint line algorithm
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
// Handles all Windows messages (menu commands, mouse/keyboard events, painting, etc.)
// Main event loop for user interaction and drawing
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int x1, y1, x2, y2;
    static vector<double> points;
    static int counter = 0;
    static std::vector<double> cardinalSplinePoints; // Stores input points for Cardinal Spline

    switch (message)
    {
    case WM_CREATE:
    {
        // Set white background and cross cursor
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(255,255,255)));
        HCURSOR hCursor = LoadCursor(NULL, IDC_CROSS);
        SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)hCursor);

        // Create menu bar and all submenus
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
        AppendMenu(hShapeMenu, MF_STRING, 2010, "Cardinal Spline");
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


        // Clipping window type menu
        HMENU hClipTypeMenu = CreatePopupMenu();
        AppendMenu(hClipTypeMenu, MF_STRING, 8001, "Rectangle Window");
        AppendMenu(hClipTypeMenu, MF_STRING, 8002, "Square Window");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hClipTypeMenu, "Clipping Window Type");

        // Filling algorithm menu
        HMENU hFillMenu = CreatePopupMenu();
        AppendMenu(hFillMenu, MF_STRING, 9001, "Recursive Flood Fill");
        AppendMenu(hFillMenu, MF_STRING, 9002, "Non-Recursive Flood Fill");
        AppendMenu(hFillMenu, MF_STRING, 9003, "Convex Fill");
        AppendMenu(hFillMenu, MF_STRING, 9004, "Non-Convex Fill");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFillMenu, "Fill Algorithm");

        // extra menu
        HMENU hExtraMenu = CreatePopupMenu();
        AppendMenu(hExtraMenu, MF_STRING, 10001, "Quarter Circles filling");
        AppendMenu(hExtraMenu, MF_STRING, 10002, "Rectangle Bezier Waves");
        AppendMenu(hExtraMenu, MF_STRING, 10003, "Circle Quarter Fill");
        AppendMenu(hExtraMenu, MF_STRING, 10004, "Square Hermite Waves");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hExtraMenu, "Extra Draw Methods");

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
            // Handle menu commands for shape selection, color, algorithms, etc.            // File menu handlers
            if (id == 1001) { // Save
                Storage::saveLayersToFile(layers, "layers.txt");
                MessageBox(hWnd, "Layers saved to layers.txt", "Save", MB_OK | MB_ICONINFORMATION);
            } else if (id == 1002) { // Load
                Storage::loadLayersFromFile(layers, "layers.txt");
                InvalidateRect(hWnd, NULL, TRUE);
                MessageBox(hWnd, "Layers loaded from layers.txt", "Load", MB_OK | MB_ICONINFORMATION);
            }
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
            else if (id == 2008) { // Filling
                currentShape = SHAPE_FILL;
                userPoints.clear();
                currentPolygon.reset();
                linePreviewStart.reset();
                linePreviewCurrent.reset();
            }
            else if (id == 2009) { // Point
                currentShape = SHAPE_POINT;
                userPoints.clear();
                currentPolygon.reset();
                linePreviewStart.reset();
                linePreviewCurrent.reset();
            }
            else if (id == 2010) { // Cardinal Spline menu item
                currentShape = SHAPE_CARDINAL_SPLINE;
                userPoints.clear();
                cardinalSplinePoints.clear();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // Help menu handler
            else if (id == 7001) {
                MessageBox(hWnd,
                    "How to use the Graphics Project:\n\n"
                    "- Select a shape or tool from the 'Draw Shape' menu.\n"
                    "- For lines: Left-click to set start, then end point.\n"
                    "- For polygons: Left-click to add points, right-click to finish.\n"
                    "- For circles/ellipses/rectangles: Left-click two points.\n"
                    "- For Bezier Spline: Select 'Spline', then left-click 4 control points.\n"
                    "- For Cardinal Spline: Select 'Cardinal Spline', left-click to add as many points as you want (minimum 4), right-click to draw the spline.\n"
                    "  If you right-click with fewer than 4 points, an error will be shown.\n"
                    "  Small preview circles will appear at each point as you click.\n"
                    "- For clipping: Select 'Clipping', then left-click two corners of the window.\n"
                    "  Use the 'Clipping Window Type' menu to choose Rectangle or Square.\n"
                    "  Rectangle window supports line, point, and polygon clipping.\n"
                    "  Square window supports only line and point clipping.\n"
                    "- Use the 'Color' menu to change drawing color.\n"
                    "- Use 'Clear' to erase all.\n\n"
                    "Extra Draw Methods:\n"
                    "- Quarter Circles Filling: Click center, then radius point, then quarter.\n"
                    "- Rectangle Bezier Waves: Click two corners.\n"
                    "- Circle Quarter Fill: Click center, then radius, then quarter.\n"
                    "- Square Hermite Waves: Click top-left, then size.\n",
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
            // Filling algorithm handlers
            else if (id >= 9001 && id <= 9004) {
                currentFillAlg = (FillAlgorithm)(id - 9001);
            }
            // Extra Draw Methods: activate interactive modes for extra shapes
            if (id == 10001) {
                // Quarter Circles filling: user will click 3 times (center, radius, quarter)
                currentShape = SHAPE_EXTRA_QUARTER_CIRCLES;
                extraQuarterCircleActive = true;
                extraQuarterStage = 0;
                userPoints.clear();
                currentPolygon.reset();
            } else if (id == 10002) {
                // Rectangle Bezier Waves: user will click 2 times (opposite corners)
                currentShape = SHAPE_EXTRA_RECT_BEZIER_WAVES;
                extraRectBezierActive = true;
                extraRectBezierStage = 0;
                userPoints.clear();
                currentPolygon.reset();
            } else if (id == 10003) {
                // Circle Quarter Fill: user will click 3 times (center, radius, quarter)
                currentShape = SHAPE_EXTRA_CIRCLE_QUARTER;
                extraCircleQuarterActive = true;
                extraCircleQuarterStage = 0;
                userPoints.clear();
                currentPolygon.reset();
            } else if (id == 10004) {
                // Square Hermite Waves: user will click 2 times (top-left, size)
                currentShape = SHAPE_EXTRA_SQUARE_HERMITE_WAVES;
                extraSquareHermiteActive = true;
                extraSquareHermiteStage = 0;
                userPoints.clear();
                currentPolygon.reset();
            }
        }
        break;
    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        // Handle all interactive drawing logic based on currentShape
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
                                    int cx, cy;
                                    
                                    if (codeOut & Clipping::TOP) {
                                        cx = x1 + (x2 - x1) * (Clipping::CLIP_Y_MAX - y1) / (y2 - y1);
                                        cy = Clipping::CLIP_Y_MAX;
                                    } else if (codeOut & Clipping::BOTTOM) {
                                        cx = x1 + (x2 - x1) * (Clipping::CLIP_Y_MIN - y1) / (y2 - y1);
                                        cy = Clipping::CLIP_Y_MIN;
                                    } else if (codeOut & Clipping::RIGHT) {
                                        cy = y1 + (y2 - y1) * (Clipping::CLIP_X_MAX - x1) / (x2 - x1);
                                        cx = Clipping::CLIP_X_MAX;
                                    } else { // LEFT
                                        cy = y1 + (y2 - y1) * (Clipping::CLIP_X_MIN - x1) / (x2 - x1);
                                        cx = Clipping::CLIP_X_MIN;
                                    }
                                    
                                    if (codeOut == code1) {
                                        x1 = cx; y1 = cy; code1 = Clipping::computeCode(x1, y1);
                                    } else {
                                        x2 = cx; y2 = cy; code2 = Clipping::computeCode(x2, y2);
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
                                    layers.back() = Layer{LayerPolygon{clipped, shape.color}};
                                } else {
                                    layers.pop_back(); // Remove the polygon if it's completely outside
                                }
                            }
                        } else if constexpr (std::is_same_v<T, LayerPoint>) {
                            if (shape.pt.x >= Clipping::CLIP_X_MIN && shape.pt.x <= Clipping::CLIP_X_MAX &&
                                shape.pt.y >= Clipping::CLIP_Y_MIN && shape.pt.y <= Clipping::CLIP_Y_MAX) {
                                // Point is inside, keep it as is
                            } else {
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
                layers.push_back(Layer{LayerBezierCurve{userPoints[0], userPoints[1], userPoints[2], userPoints[3], currentColor}});
                userPoints.clear();
                InvalidateRect(hWnd, NULL, TRUE);
            } else {
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        // Handle Cardinal Spline point input
        else if (currentShape == SHAPE_CARDINAL_SPLINE) {
            cardinalSplinePoints.push_back(x);
            cardinalSplinePoints.push_back(y);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        // Handle filling
        else if (currentShape == SHAPE_FILL) {
            if (!layers.empty()) {
                Layer& lastLayer = layers.back();
                std::visit([&](auto&& shape) {
                    using T = std::decay_t<decltype(shape)>;
                    if constexpr (std::is_same_v<T, LayerPolygon> || 
                                std::is_same_v<T, LayerRect> ||
                                std::is_same_v<T, LayerCircle> ||
                                std::is_same_v<T, LayerEllipse>) {
                        layers.push_back(Layer{LayerFill{{x, y}, shape.color, currentFillAlg}});
                    } else {
                        MessageBox(hWnd, "This shape does not support filling.", 
                                  "Invalid Shape", MB_OK | MB_ICONWARNING);
                    }
                }, lastLayer.shape);
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
        // Handle dynamic quarter circle filling
        else if (currentShape == SHAPE_EXTRA_QUARTER_CIRCLES && extraQuarterCircleActive) {
            if (extraQuarterStage == 0) {
                extraQuarterCenter = {x, y};
                extraQuarterStage = 1;
            } else if (extraQuarterStage == 1) {
                int dx = x - extraQuarterCenter.x;
                int dy = y - extraQuarterCenter.y;
                extraQuarterRadius = (int)hypot(dx, dy);
                extraQuarterStage = 2;
                InvalidateRect(hWnd, NULL, TRUE);
            } else if (extraQuarterStage == 2) {
                int dx = x - extraQuarterCenter.x;
                int dy = y - extraQuarterCenter.y;
                int dist = (int)hypot(dx, dy);
                if (dist > extraQuarterRadius) {
                    MessageBox(hWnd, "Error: Click is outside the circle boundary. Please click inside the circle to select a quarter.", "Quarter Selection Error", MB_OK | MB_ICONERROR);
                    return 0;
                }
                int dy_inv = extraQuarterCenter.y - y;
                if (dx >= 0 && dy_inv >= 0)
                    extraQuarterQuarter = 1;
                else if (dx < 0 && dy_inv >= 0)
                    extraQuarterQuarter = 2;
                else if (dx < 0 && dy_inv < 0)
                    extraQuarterQuarter = 3;
                else
                    extraQuarterQuarter = 4;
                layers.push_back(Layer{LayerQuarterCircleFilling{extraQuarterCenter, extraQuarterRadius, extraQuarterQuarter, currentColor}});
                extraQuarterCircleActive = false;
                extraQuarterStage = 0;
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        // Handle dynamic rectangle Bezier waves
        else if (currentShape == SHAPE_EXTRA_RECT_BEZIER_WAVES && extraRectBezierActive) {
            if (extraRectBezierStage == 0) {
                extraRectBezierP1 = {x, y};
                extraRectBezierStage = 1;
            } else if (extraRectBezierStage == 1) {
                extraRectBezierP2 = {x, y};
                layers.push_back(Layer{LayerRectangleBezierWaves{extraRectBezierP1, extraRectBezierP2, currentColor}});
                extraRectBezierActive = false;
                extraRectBezierStage = 0;
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        // Extra: Circle Quarter Fill (3 clicks: center, radius, quarter)
        if (currentShape == SHAPE_EXTRA_CIRCLE_QUARTER && extraCircleQuarterActive) {
            if (extraCircleQuarterStage == 0) {
                extraCircleQuarterCenter = {x, y};
                extraCircleQuarterStage = 1;
            } else if (extraCircleQuarterStage == 1) {
                int dx = x - extraCircleQuarterCenter.x;
                int dy = y - extraCircleQuarterCenter.y;
                extraCircleQuarterRadius = (int)hypot(dx, dy);
                extraCircleQuarterStage = 2;
                InvalidateRect(hWnd, NULL, TRUE);
            } else if (extraCircleQuarterStage == 2) {
                int dx = x - extraCircleQuarterCenter.x;
                int dy = y - extraCircleQuarterCenter.y;
                int dist = (int)hypot(dx, dy);
                if (dist > extraCircleQuarterRadius) {
                    MessageBox(hWnd, "Error: Click is outside the circle boundary. Please click inside the circle to select a quarter.", "Quarter Selection Error", MB_OK | MB_ICONERROR);
                    return 0;
                }
                int dy_inv = extraCircleQuarterCenter.y - y;
                if (dx >= 0 && dy_inv >= 0)
                    extraCircleQuarterQuarter = 1;
                else if (dx < 0 && dy_inv >= 0)
                    extraCircleQuarterQuarter = 2;
                else if (dx < 0 && dy_inv < 0)
                    extraCircleQuarterQuarter = 3;
                else
                    extraCircleQuarterQuarter = 4;
                layers.push_back(Layer{LayerCircleQuarter{extraCircleQuarterCenter, extraCircleQuarterRadius, extraCircleQuarterQuarter, currentColor}});
                extraCircleQuarterActive = false;
                extraCircleQuarterStage = 0;
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        // Extra: Square Hermite Waves (2 clicks: top-left, size)
        if (currentShape == SHAPE_EXTRA_SQUARE_HERMITE_WAVES && extraSquareHermiteActive) {
            if (extraSquareHermiteStage == 0) {
                extraSquareHermiteTopLeft = {x, y};
                extraSquareHermiteStage = 1;
            } else if (extraSquareHermiteStage == 1) {
                int dx = x - extraSquareHermiteTopLeft.x;
                int dy = y - extraSquareHermiteTopLeft.y;
                int size = max(abs(dx), abs(dy));
                if (size == 0) {
                    MessageBox(hWnd, "Error: Size must be greater than zero.", "Square Size Error", MB_OK | MB_ICONERROR);
                    return 0;
                }
                layers.push_back(Layer{LayerSquareHermiteWaves{extraSquareHermiteTopLeft, size, currentColor}});
                extraSquareHermiteActive = false;
                extraSquareHermiteStage = 0;
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
    }
    break;
    case WM_MOUSEMOVE:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            // Update preview positions for line and clip window
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
            // Right-click: finish polygon, cancel line/clip preview
            // Handle polygon completion
            if (currentShape == SHAPE_POLYGON && currentPolygon && currentPolygon->pts.size() >= 3) {
                if (currentPolygon->pts.front().x != currentPolygon->pts.back().x ||
                    currentPolygon->pts.front().y != currentPolygon->pts.back().y) {
                    currentPolygon->pts.push_back(currentPolygon->pts.front());
                }
                
                layers.push_back(Layer{LayerPolygon{*currentPolygon}});
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
            // Finish Cardinal Spline input
            else if (currentShape == SHAPE_CARDINAL_SPLINE) {
                // Only allow drawing if at least 4 points (8 values)
                if (cardinalSplinePoints.size() < 8) {
                    MessageBox(hWnd, "Cardinal Spline requires at least 4 points (8 clicks).", "Not enough points", MB_OK | MB_ICONERROR);
                } else {
                    // Store the spline as a persistent layer
                    layers.push_back(Layer{LayerCardinalSpline{cardinalSplinePoints, currentColor}});
                    cardinalSplinePoints.clear();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
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
                    } else if constexpr (std::is_same_v<T, LayerFill>) {
                        // Find the previous layer to fill
                        for (auto it = layers.begin(); it != layers.end(); ++it) {
                            if (&(*it) == &layer) {
                                if (it != layers.begin()) {
                                    --it;  // Move to the previous layer
                                    std::visit([&](auto&& prevShape) {
                                        using P = std::decay_t<decltype(prevShape)>;
                                        if constexpr (std::is_same_v<P, LayerPolygon>) {
                                            if (prevShape.pts.size() >= 3) {
                                                switch (shape.alg) {
                                                    case FILL_RECURSIVE_FLOOD:
                                                        Filling::RecursiveFloodFill(hdc, shape.fillPoint.x, shape.fillPoint.y, shape.color);
                                                        break;
                                                    case FILL_NONRECURSIVE_FLOOD:
                                                        Filling::NonRecursiveFloodFill(hdc, shape.fillPoint.x, shape.fillPoint.y, shape.color);
                                                        break;
                                                    case FILL_CONVEX:
                                                        if (Common::IsConvex(prevShape.pts)) {
                                                            Filling::ConvexFill(hdc, prevShape.pts, shape.color);
                                                        }
                                                        break;
                                                    case FILL_NONCONVEX:
                                                        Filling::NonConvexFill(hdc, prevShape.pts, shape.color);
                                                        break;
                                                }
                                            }
                                        } else if constexpr (std::is_same_v<P, LayerRect>) {
                                            std::vector<POINT> rectPoints = {
                                                prevShape.p1,
                                                {prevShape.p2.x, prevShape.p1.y},
                                                prevShape.p2,
                                                {prevShape.p1.x, prevShape.p2.y},
                                                prevShape.p1
                                            };
                                            switch (shape.alg) {
                                                case FILL_RECURSIVE_FLOOD:
                                                    Filling::RecursiveFloodFill(hdc, shape.fillPoint.x, shape.fillPoint.y, shape.color);
                                                    break;
                                                case FILL_NONRECURSIVE_FLOOD:
                                                    Filling::NonRecursiveFloodFill(hdc, shape.fillPoint.x, shape.fillPoint.y, shape.color);
                                                    break;
                                                case FILL_CONVEX:
                                                    Filling::ConvexFill(hdc, rectPoints, shape.color);
                                                    break;
                                                case FILL_NONCONVEX:
                                                    Filling::NonConvexFill(hdc, rectPoints, shape.color);
                                                    break;
                                            }
                                        } else if constexpr (std::is_same_v<P, LayerCircle> || std::is_same_v<P, LayerEllipse>) {
                                            if (shape.alg == FILL_RECURSIVE_FLOOD) {
                                                Filling::RecursiveFloodFill(hdc, shape.fillPoint.x, shape.fillPoint.y, shape.color);
                                            } else if (shape.alg == FILL_NONRECURSIVE_FLOOD) {
                                                Filling::NonRecursiveFloodFill(hdc, shape.fillPoint.x, shape.fillPoint.y, shape.color);
                                            }
                                        }
                                    }, it->shape);
                                }
                                break;
                            }
                        }
                    } else if constexpr (std::is_same_v<T, LayerQuarterCircleFilling>) {
                        // Draw the circle boundary
                        SecondDegreeCurve::BresenhamCircle(hdc, shape.center.x, shape.center.y, shape.radius, shape.color);
                        // Draw the filled quarter
                        Filling::FillQuarterWithSmallCircles(hdc, shape.center.x, shape.center.y, shape.radius, shape.quarter, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerRectangleBezierWaves>) {
                        // Draw rectangle boundary
                        POINT rectPoints[5] = {
                            shape.p1,
                            {shape.p2.x, shape.p1.y},
                            shape.p2,
                            {shape.p1.x, shape.p2.y},
                            shape.p1
                        };
                        DrawPolygon(hdc, std::vector<POINT>(rectPoints, rectPoints + 5), shape.color);
                        // Fill with Bezier waves
                        Filling::FillRectangleWithBezierWaves(hdc, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerCircleQuarter>) {
                        SecondDegreeCurve::BresenhamCircle(hdc, shape.center.x, shape.center.y, shape.radius, shape.color);
                        Filling::FillCircleQuarter(hdc, shape.center.x, shape.center.y, shape.radius, shape.quarter, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerSquareHermiteWaves>) {
                        // Draw square boundary
                        POINT pts[5] = {
                            shape.topLeft,
                            {shape.topLeft.x + shape.size, shape.topLeft.y},
                            {shape.topLeft.x + shape.size, shape.topLeft.y + shape.size},
                            {shape.topLeft.x, shape.topLeft.y + shape.size},
                            shape.topLeft
                        };
                        DrawPolygon(hdc, std::vector<POINT>(pts, pts + 5), shape.color);
                        Filling::FillSquareWithVerticalHermiteWaves(hdc, shape.topLeft.x, shape.topLeft.y, shape.size, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerBezierCurve>) {
                        ThirdDegreeCurve::BezierCurve(hdc, shape.p0.x, shape.p0.y, shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.p3.x, shape.p3.y, shape.color);
                    } else if constexpr (std::is_same_v<T, LayerCardinalSpline>) {
                        if (shape.points.size() >= 8) {
                            ThirdDegreeCurve::CardinalSplines(hdc, shape.points, 1, shape.color);
                        }
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
            // Draw preview for quarter circle boundary if in stage 2
            if (currentShape == SHAPE_EXTRA_QUARTER_CIRCLES && extraQuarterCircleActive && extraQuarterStage == 2) {
                // Draw the circle boundary using midpoint
                SecondDegreeCurve::BresenhamCircle(hdc, extraQuarterCenter.x, extraQuarterCenter.y, extraQuarterRadius, currentColor);
            }
            // Draw preview for rectangle if in stage 1
            if (currentShape == SHAPE_EXTRA_RECT_BEZIER_WAVES && extraRectBezierActive && extraRectBezierStage == 1) {
                HPEN hPen = CreatePen(PS_DOT, 1, currentColor);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                Rectangle(hdc, extraRectBezierP1.x, extraRectBezierP1.y, extraRectBezierP1.x + 1, extraRectBezierP1.y + 1); // single point if not moved
                POINT mouse;
                GetCursorPos(&mouse);
                ScreenToClient(hWnd, &mouse);
                Rectangle(hdc, extraRectBezierP1.x, extraRectBezierP1.y, mouse.x, mouse.y);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            // Draw preview for circle quarter (after radius, before quarter selection)
            if (currentShape == SHAPE_EXTRA_CIRCLE_QUARTER && extraCircleQuarterActive && extraCircleQuarterStage == 2) {
                // Draw the circle boundary using midpoint
                SecondDegreeCurve::BresenhamCircle(hdc, extraCircleQuarterCenter.x, extraCircleQuarterCenter.y, extraCircleQuarterRadius, currentColor);
            }
            // Draw preview for square (after top-left, before size)
            if (currentShape == SHAPE_EXTRA_SQUARE_HERMITE_WAVES && extraSquareHermiteActive && extraSquareHermiteStage == 1) {
                HPEN hPen = CreatePen(PS_DOT, 1, currentColor);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                POINT mouse;
                GetCursorPos(&mouse);
                ScreenToClient(hWnd, &mouse);
                int size = max(abs(mouse.x - extraSquareHermiteTopLeft.x), abs(mouse.y - extraSquareHermiteTopLeft.y));
                Rectangle(hdc, extraSquareHermiteTopLeft.x, extraSquareHermiteTopLeft.y, extraSquareHermiteTopLeft.x + size, extraSquareHermiteTopLeft.y + size);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            // Draw preview for Cardinal Spline
            if (currentShape == SHAPE_CARDINAL_SPLINE && cardinalSplinePoints.size() >= 2) {
                // Draw small preview circles at each input point
                for (size_t i = 0; i + 1 < cardinalSplinePoints.size(); i += 2) {
                    int px = (int)cardinalSplinePoints[i];
                    int py = (int)cardinalSplinePoints[i + 1];
                    Ellipse(hdc, px - 3, py - 3, px + 3, py + 3);
                }
                // Only preview spline if at least 4 points
                if (cardinalSplinePoints.size() >= 8) {
                    ThirdDegreeCurve::CardinalSplines(hdc, cardinalSplinePoints, 1, currentColor);
                }
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
// Registers the window class, creates the main window, and starts the message loop
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register window class and create main window
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
// ===== End of File =====