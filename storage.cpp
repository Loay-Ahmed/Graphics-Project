#include "import.h"
#include <fstream>
#include <sstream>
#include <utility> // for std::pair

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
};