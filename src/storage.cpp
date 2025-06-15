#include "import.h"
#include <fstream>
#include <sstream>
#include <utility> // for std::pair
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Storage
{
public:
    static bool saveToFile()
    {
        std::ofstream outFile("drawing.txt");
        if (!outFile.is_open())
        {
            std::cerr << "Error opening file: drawing.txt\n";
            return false;
        }

        for (const auto &pair : Common::drawings)
        {
            outFile << pair.first.first << " " << pair.first.second << " " << pair.second << "\n";
        }

        outFile.close();
        return true;
    }
    static void clearCanvas(HWND hwnd)
    {
        InvalidateRect(hwnd, NULL, true);
        UpdateWindow(hwnd);
        Common::drawings.clear();
    }
    static void setCanvas(HDC hdc)
    {
        for (const auto &pair : Common::drawings)
        {
            int x = pair.first.first;
            int y = pair.first.second;

            // Set the pixel on the device context
            SetPixel(hdc, x, y, pair.second);
        }
    }
    static bool loadFromFile(HDC hdc)
    {
        std::ifstream inFile("drawing.txt");
        if (!inFile.is_open())
        {
            std::cerr << "Error reading file: drawing.txt\n";
            return false;
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

    // Helper to serialize a POINT
    json point_to_json(const POINT &pt)
    {
        return {{"x", pt.x}, {"y", pt.y}};
    }
    POINT point_from_json(const json &j)
    {
        return POINT{j.at("x").get<int>(), j.at("y").get<int>()};
    }

    // Serialize a Layer to JSON
    json layer_to_json(const Layer &layer)
    {
        json j;
        std::visit(
            [&](auto &&shape)
            {
                using T = std::decay_t<decltype(shape)>;
                if constexpr (std::is_same_v<T, LayerLine>)
                {
                    j["type"] = "line";
                    j["p1"] = point_to_json(shape.p1);
                    j["p2"] = point_to_json(shape.p2);
                    j["color"] = shape.color;
                    j["alg"] = shape.alg;
                }
                else if constexpr (std::is_same_v<T, LayerCircle>)
                {
                    j["type"] = "circle";
                    j["center"] = point_to_json(shape.center);
                    j["r"] = shape.r;
                    j["color"] = shape.color;
                    j["alg"] = shape.alg;
                }
                else if constexpr (std::is_same_v<T, LayerEllipse>)
                {
                    j["type"] = "ellipse";
                    j["center"] = point_to_json(shape.center);
                    j["a"] = shape.a;
                    j["b"] = shape.b;
                    j["color"] = shape.color;
                    j["alg"] = shape.alg;
                }
                else if constexpr (std::is_same_v<T, LayerRect>)
                {
                    j["type"] = "rect";
                    j["p1"] = point_to_json(shape.p1);
                    j["p2"] = point_to_json(shape.p2);
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerPolygon>)
                {
                    j["type"] = "polygon";
                    j["pts"] = json::array();
                    for (const auto &pt : shape.pts)
                        j["pts"].push_back(point_to_json(pt));
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerPoint>)
                {
                    j["type"] = "point";
                    j["pt"] = point_to_json(shape.pt);
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerFill>)
                {
                    j["type"] = "fill";
                    j["fillPoint"] = point_to_json(shape.fillPoint);
                    j["color"] = shape.color;
                    j["alg"] = shape.alg;
                }
                else if constexpr (std::is_same_v<T, LayerQuarterCircleFilling>)
                {
                    j["type"] = "quarter_circle";
                    j["center"] = point_to_json(shape.center);
                    j["radius"] = shape.radius;
                    j["quarter"] = shape.quarter;
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerRectangleBezierWaves>)
                {
                    j["type"] = "rect_bezier";
                    j["p1"] = point_to_json(shape.p1);
                    j["p2"] = point_to_json(shape.p2);
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerCircleQuarter>)
                {
                    j["type"] = "circle_quarter";
                    j["center"] = point_to_json(shape.center);
                    j["radius"] = shape.radius;
                    j["quarter"] = shape.quarter;
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerSquareHermiteWaves>)
                {
                    j["type"] = "square_hermite";
                    j["topLeft"] = point_to_json(shape.topLeft);
                    j["size"] = shape.size;
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerBezierCurve>)
                {
                    j["type"] = "bezier";
                    j["p0"] = point_to_json(shape.p0);
                    j["p1"] = point_to_json(shape.p1);
                    j["p2"] = point_to_json(shape.p2);
                    j["p3"] = point_to_json(shape.p3);
                    j["color"] = shape.color;
                }
                else if constexpr (std::is_same_v<T, LayerCardinalSpline>)
                {
                    j["type"] = "cardinal_spline";
                    j["points"] = shape.points;
                    j["color"] = shape.color;
                }
            },
            layer.shape);
        return j;
    }

    // Deserialize a Layer from JSON
    Layer layer_from_json(const json &j)
    {
        std::string type = j.at("type").get<std::string>();
        if (type == "line")
        {
            return Layer{LayerLine{point_from_json(j.at("p1")), point_from_json(j.at("p2")), j.at("color"), j.at("alg")}};
        }
        else if (type == "circle")
        {
            return Layer{LayerCircle{point_from_json(j.at("center")), j.at("r"), j.at("color"), j.at("alg")}};
        }
        else if (type == "ellipse")
        {
            return Layer{LayerEllipse{point_from_json(j.at("center")), j.at("a"), j.at("b"), j.at("color"), j.at("alg")}};
        }
        else if (type == "rect")
        {
            return Layer{LayerRect{point_from_json(j.at("p1")), point_from_json(j.at("p2")), j.at("color")}};
        }
        else if (type == "polygon")
        {
            std::vector<POINT> pts;
            for (const auto &ptj : j.at("pts"))
                pts.push_back(point_from_json(ptj));
            return Layer{LayerPolygon{pts, j.at("color")}};
        }
        else if (type == "point")
        {
            return Layer{LayerPoint{point_from_json(j.at("pt")), j.at("color")}};
        }
        else if (type == "fill")
        {
            return Layer{LayerFill{point_from_json(j.at("fillPoint")), j.at("color"), j.at("alg")}};
        }
        else if (type == "quarter_circle")
        {
            return Layer{LayerQuarterCircleFilling{point_from_json(j.at("center")), j.at("radius"), j.at("quarter"), j.at("color")}};
        }
        else if (type == "rect_bezier")
        {
            return Layer{LayerRectangleBezierWaves{point_from_json(j.at("p1")), point_from_json(j.at("p2")), j.at("color")}};
        }
        else if (type == "circle_quarter")
        {
            return Layer{LayerCircleQuarter{point_from_json(j.at("center")), j.at("radius"), j.at("quarter"), j.at("color")}};
        }
        else if (type == "square_hermite")
        {
            return Layer{LayerSquareHermiteWaves{point_from_json(j.at("topLeft")), j.at("size"), j.at("color")}};
        }
        else if (type == "bezier")
        {
            return Layer{LayerBezierCurve{point_from_json(j.at("p0")), point_from_json(j.at("p1")), point_from_json(j.at("p2")), point_from_json(j.at("p3")), j.at("color")}};
        }
        else if (type == "cardinal_spline")
        {
            return Layer{LayerCardinalSpline{j.at("points").get<std::vector<double>>(), j.at("color")}};
        }
        throw std::runtime_error("Unknown layer type in JSON");
    }
    static bool saveToFile(const std::vector<Layer> &layers)
    {
        json j;
        j["layers"] = json::array();
        for (const auto &layer : layers)
        {
            j["layers"].push_back(layer_to_json(layer));
        }
        std::ofstream outFile("drawing.json");
        if (!outFile.is_open())
        {
            std::cerr << "Error opening file: drawing.json\n";
            return false;
        }
        outFile << j.dump(4);
        outFile.close();
        return true;
    }

    static bool loadFromFile(std::vector<Layer> &layers)
    {
        std::ifstream inFile("drawing.json");
        if (!inFile.is_open())
        {
            std::cerr << "Error reading file: drawing.json\n";
            return false;
        }
        json j;
        inFile >> j;
        layers.clear();
        for (const auto &lj : j["layers"])
        {
            layers.push_back(layer_from_json(lj));
        }
        inFile.close();
        return true;
    }
};