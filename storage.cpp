#include "import.h"
#include <fstream>
#include <sstream>
#include <utility> // for std::pair

class Storage {
public:
    static bool saveToFile() {
        std::ofstream outFile("drawing.txt");
        if (!outFile.is_open()) {
            std::cerr << "Error opening file: drawing.txt\n";
            return false;
        }
        
        for (const auto& [pos, color] : Common::drawings) {
            outFile << pos.first << " " << pos.second << " " << color << "\n";
        }
        
        outFile.close();
        return true;
    }

    static bool loadFromFile(bool overwrite = true) {
        std::ifstream inFile("drawing.txt");
        if (!inFile.is_open()) {
            std::cerr << "Error reading file: drawing.txt\n";
            return false;
        }
        
        std::string line;
        while (std::getline(inFile, line)) {
            std::istringstream iss(line);
            int x, y;
            COLORREF color;
            
            if (iss >> x >> y >> color) {
                auto pos = std::make_pair(x, y);
                if (overwrite) {
                    // Always overwrite existing pixels
                    Common::drawings[pos] = color;
                } else {
                    // Only add if position doesn't exist
                    Common::drawings.try_emplace(pos, color);
                }
            }
        }
        
        inFile.close();
        return true;
    }
};