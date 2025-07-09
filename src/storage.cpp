#include "../include/storage.h"
#include "../include/import.h"
#include "../include/common.h"
#include "../include/layer.h"
#include <fstream>
#include <sstream>
#include <utility> // for std::pair
#include <variant>
// #include <nlohmann/json.hpp>  // Commented out - library not available
// using json = nlohmann::json;

// Implementation of Storage class methods
// Handles saving/loading of drawings and layers, and canvas management
// Save the current drawing (all pixels) to a file (with file path)
// Used by GUI file dialog to specify the file name
bool Storage::saveToFile(const std::string& path)
{
    std::ofstream outFile(path);
    if (!outFile.is_open())
    {
        std::cerr << "Error opening file: " << path << "\n";
        return false;
    }
    for (const auto &pair : Common::drawings)
    {
        outFile << pair.first.first << " " << pair.first.second << " " << pair.second << "\n";
    }
    outFile.close();
    return true;
}
// Overload for backward compatibility (default file name)
bool Storage::saveToFile() { return saveToFile("drawing.txt"); }

// Clear the canvas and remove all drawings
void Storage::clearCanvas(HWND hwnd)
{
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
    Common::drawings.clear();
}

// Redraw the canvas from the stored drawings
void Storage::setCanvas(HDC hdc)
{
    for (const auto &pair : Common::drawings)
    {
        int x = pair.first.first;
        int y = pair.first.second;

        // Set the pixel on the device context
        SetPixel(hdc, x, y, pair.second);
    }
}

// Load a drawing from file and update the canvas (with file path)
// Used by GUI file dialog to specify the file name
bool Storage::loadFromFile(HDC hdc, const std::string& path)
{
    std::ifstream inFile(path);
    if (!inFile.is_open())
    {
        std::cerr << "Error reading file: " << path << "\n";        return false;
    }
    std::string line;
    while (std::getline(inFile, line))
    {
        std::istringstream iss(line);
        int x, y;
        COLORREF color;
        if (iss >> x >> y >> color)
        {
            auto pos = std::make_pair(x, y);
            Common::drawings[pos] = color;
        }
    }
    inFile.close();
    setCanvas(hdc);
    return true;
}
// Overload for backward compatibility (default file name)
bool Storage::loadFromFile(HDC hdc) { return loadFromFile(hdc, "drawing.txt"); }

   

// Helper: Serialize a POINT to string
static std::string point_to_str(const POINT& pt) {
    return std::to_string(pt.x) + " " + std::to_string(pt.y);
}
// Helper: Deserialize a POINT from string
static POINT str_to_point(std::istringstream& iss) {
    int x, y;
    iss >> x >> y;
    return POINT{x, y};
}

// Save all layers to a file for persistence
bool Storage::saveLayersToFile(const std::vector<Layer>& layers, const std::string& path) {
    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << path << "\n";
        return false;
    }
    for (const auto& layer : layers) {
        std::visit([&outFile](auto&& shape) {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, LayerLine>) {
                outFile << "line " << shape.p1.x << " " << shape.p1.y << " " << shape.p2.x << " " << shape.p2.y << " " << shape.color << " " << shape.alg << "\n";
            } else if constexpr (std::is_same_v<T, LayerCircle>) {
                outFile << "circle " << shape.center.x << " " << shape.center.y << " " << shape.r << " " << shape.color << " " << shape.alg << "\n";
            } else if constexpr (std::is_same_v<T, LayerEllipse>) {
                outFile << "ellipse " << shape.center.x << " " << shape.center.y << " " << shape.a << " " << shape.b << " " << shape.color << " " << shape.alg << "\n";
            } else if constexpr (std::is_same_v<T, LayerRect>) {
                outFile << "rect " << shape.p1.x << " " << shape.p1.y << " " << shape.p2.x << " " << shape.p2.y << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerPolygon>) {
                outFile << "polygon " << shape.pts.size();
                for (const auto& pt : shape.pts) outFile << " " << pt.x << " " << pt.y;
                outFile << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerPoint>) {
                outFile << "point " << shape.pt.x << " " << shape.pt.y << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerFill>) {
                outFile << "fill " << shape.fillPoint.x << " " << shape.fillPoint.y << " " << shape.color << " " << shape.alg << "\n";
            } else if constexpr (std::is_same_v<T, LayerQuarterCircleFilling>) {
                outFile << "quarter_circle " << shape.center.x << " " << shape.center.y << " " << shape.radius << " " << shape.quarter << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerRectangleBezierWaves>) {
                outFile << "rect_bezier " << shape.p1.x << " " << shape.p1.y << " " << shape.p2.x << " " << shape.p2.y << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerCircleQuarter>) {
                outFile << "circle_quarter " << shape.center.x << " " << shape.center.y << " " << shape.radius << " " << shape.quarter << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerSquareHermiteWaves>) {
                outFile << "square_hermite " << shape.topLeft.x << " " << shape.topLeft.y << " " << shape.size << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerBezierCurve>) {
                outFile << "bezier " << shape.p0.x << " " << shape.p0.y << " " << shape.p1.x << " " << shape.p1.y << " " << shape.p2.x << " " << shape.p2.y << " " << shape.p3.x << " " << shape.p3.y << " " << shape.color << "\n";
            } else if constexpr (std::is_same_v<T, LayerCardinalSpline>) {
                outFile << "cardinal_spline " << shape.points.size();
                for (const auto& v : shape.points) outFile << " " << v;
                outFile << " " << shape.color << "\n";
            }
        }, layer.shape);
    }
    outFile.close();
    return true;
}

// Load all layers from a file for persistence
bool Storage::loadLayersFromFile(std::vector<Layer>& layers, const std::string& path) {
    std::ifstream inFile(path);
    if (!inFile.is_open()) {
        std::cerr << "Error reading file: " << path << "\n";
        return false;
    }
    layers.clear();
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "line") {
            int x1, y1, x2, y2; COLORREF color; int alg;
            iss >> x1 >> y1 >> x2 >> y2 >> color >> alg;
            layers.push_back(Layer{LayerLine{POINT{x1, y1}, POINT{x2, y2}, color, (LineAlgorithm)alg}});
        } else if (type == "circle") {
            int cx, cy, r; COLORREF color; int alg;
            iss >> cx >> cy >> r >> color >> alg;
            layers.push_back(Layer{LayerCircle{POINT{cx, cy}, r, color, (CircleAlgorithm)alg}});
        } else if (type == "ellipse") {
            int cx, cy, a, b; COLORREF color; int alg;
            iss >> cx >> cy >> a >> b >> color >> alg;
            layers.push_back(Layer{LayerEllipse{POINT{cx, cy}, a, b, color, (EllipseAlgorithm)alg}});
        } else if (type == "rect") {
            int x1, y1, x2, y2; COLORREF color;
            iss >> x1 >> y1 >> x2 >> y2 >> color;
            layers.push_back(Layer{LayerRect{POINT{x1, y1}, POINT{x2, y2}, color}});
        } else if (type == "polygon") {
            size_t n; iss >> n;
            std::vector<POINT> pts(n);
            for (size_t i = 0; i < n; ++i) {
                iss >> pts[i].x >> pts[i].y;
            }
            COLORREF color; iss >> color;
            layers.push_back(Layer{LayerPolygon{pts, color}});
        } else if (type == "point") {
            int x, y; COLORREF color;
            iss >> x >> y >> color;
            layers.push_back(Layer{LayerPoint{POINT{x, y}, color}});
        } else if (type == "fill") {
            int x, y; COLORREF color; int alg;
            iss >> x >> y >> color >> alg;
            layers.push_back(Layer{LayerFill{POINT{x, y}, color, (FillAlgorithm)alg}});
        } else if (type == "quarter_circle") {
            int cx, cy, radius, quarter; COLORREF color;
            iss >> cx >> cy >> radius >> quarter >> color;
            layers.push_back(Layer{LayerQuarterCircleFilling{POINT{cx, cy}, radius, quarter, color}});
        } else if (type == "rect_bezier") {
            int x1, y1, x2, y2; COLORREF color;
            iss >> x1 >> y1 >> x2 >> y2 >> color;
            layers.push_back(Layer{LayerRectangleBezierWaves{POINT{x1, y1}, POINT{x2, y2}, color}});
        } else if (type == "circle_quarter") {
            int cx, cy, radius, quarter; COLORREF color;
            iss >> cx >> cy >> radius >> quarter >> color;
            layers.push_back(Layer{LayerCircleQuarter{POINT{cx, cy}, radius, quarter, color}});
        } else if (type == "square_hermite") {
            int x, y, size; COLORREF color;
            iss >> x >> y >> size >> color;
            layers.push_back(Layer{LayerSquareHermiteWaves{POINT{x, y}, size, color}});
        } else if (type == "bezier") {
            int x0, y0, x1, y1, x2, y2, x3, y3; COLORREF color;
            iss >> x0 >> y0 >> x1 >> y1 >> x2 >> y2 >> x3 >> y3 >> color;
            layers.push_back(Layer{LayerBezierCurve{POINT{x0, y0}, POINT{x1, y1}, POINT{x2, y2}, POINT{x3, y3}, color}});
        } else if (type == "cardinal_spline") {
            size_t n; iss >> n;
            std::vector<double> points(n);
            for (size_t i = 0; i < n; ++i) iss >> points[i];
            COLORREF color; iss >> color;
            layers.push_back(Layer{LayerCardinalSpline{points, color}});
        }
    }
    inFile.close();
    return true;
}
