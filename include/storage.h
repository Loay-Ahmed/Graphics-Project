#ifndef STORAGE_H
#define STORAGE_H

#include <windows.h>
#include <vector>
#include <string>

// Forward declarations
struct Layer;

/**
 * Storage class for saving and loading graphics project data
 * Provides functionality to save/load both simple drawings and complex layer structures
 */
class Storage
{
public:
    // Simple drawing storage (for basic pixel-based drawings)
    static bool saveToFile();
    static bool saveToFile(const std::string& path);
    static bool loadFromFile(HDC hdc);
    static bool loadFromFile(HDC hdc, const std::string& path);
    
    // Canvas management
    static void clearCanvas(HWND hwnd);
    static void setCanvas(HDC hdc);
    
    // Layer-based storage (for complex shapes and objects)
    // Note: JSON-based layer storage is disabled due to missing nlohmann/json library
    // static bool saveToFile(const std::vector<Layer> &layers);
    // static bool loadFromFile(std::vector<Layer> &layers);

    // New methods for saving/loading layers
    static bool saveLayersToFile(const std::vector<Layer>& layers, const std::string& path);
    static bool loadLayersFromFile(std::vector<Layer>& layers, const std::string& path);
};

#endif // STORAGE_H
