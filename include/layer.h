#pragma once
#include <windows.h>
#include <vector>
#include <variant>

// Layer type definitions
struct LayerLine { POINT p1, p2; COLORREF color; int alg; };
struct LayerCircle { POINT center; int r; COLORREF color; int alg; };
struct LayerEllipse { POINT center; int a, b; COLORREF color; int alg; };
struct LayerRect { POINT p1, p2; COLORREF color; };
struct LayerPolygon { std::vector<POINT> pts; COLORREF color; };
struct LayerPoint { POINT pt; COLORREF color; };
struct LayerFill { POINT fillPoint; COLORREF color; int alg; };
struct LayerQuarterCircleFilling { POINT center; int radius; int quarter; COLORREF color; };
struct LayerRectangleBezierWaves { POINT p1, p2; COLORREF color; };
struct LayerCircleQuarter { POINT center; int radius; int quarter; COLORREF color; };
struct LayerSquareHermiteWaves { POINT topLeft; int size; COLORREF color; };
struct LayerBezierCurve { POINT p0, p1, p2, p3; COLORREF color; };
typedef std::vector<double> CardinalSplinePoints;
struct LayerCardinalSpline { CardinalSplinePoints points; COLORREF color; };
using LayerShape = std::variant<LayerLine, LayerCircle, LayerEllipse, LayerRect, LayerPolygon, LayerPoint, LayerFill, LayerQuarterCircleFilling, LayerRectangleBezierWaves, LayerCircleQuarter, LayerSquareHermiteWaves, LayerBezierCurve, LayerCardinalSpline>;
struct Layer { LayerShape shape; }; 