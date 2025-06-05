#include "../include/clipping.h"
#include "../include/import.h" // Adjust the import path as needed
#include"../src/clipping.cpp"
#include <cassert>
#include <vector>
#include <iostream>

void test_set_clip_window() {
    Clipping::SetClipWindow(10, 20, 30, 40);
    assert(Clipping::CLIP_X_MIN == 10);
    assert(Clipping::CLIP_Y_MIN == 20);
    assert(Clipping::CLIP_X_MAX == 30);
    assert(Clipping::CLIP_Y_MAX == 40);
}

void test_inside() {
    Clipping::SetClipWindow(0, 0, 100, 100);
    assert(Clipping::inside(50, 50, 0)); // left
    assert(!Clipping::inside(-1, 50, 0));
    assert(Clipping::inside(50, 50, 1)); // right
    assert(!Clipping::inside(101, 50, 1));
    assert(Clipping::inside(50, 50, 2)); // bottom
    assert(!Clipping::inside(50, -1, 2));
    assert(Clipping::inside(50, 50, 3)); // top
    assert(!Clipping::inside(50, 101, 3));
}

void test_clip_point_square() {
    // This is a visual test, but we can check logic by calling the function
    // and ensuring no crash. (No assert here)
    HDC hdc = nullptr; // Not used in logic
    Clipping::SetClipWindow(0, 0, 100, 100);
    Clipping::ClipPointSquare(hdc, 50, 50, RGB(0,0,0));
    Clipping::ClipPointSquare(hdc, 150, 150, RGB(0,0,0));
}

void test_computeCode() {
    Clipping::SetClipWindow(0, 0, 100, 100);
    // Inside
    assert(Clipping::computeCode(50, 50) == 0);
    // Left
    assert(Clipping::computeCode(-1, 50) & Clipping::LEFT);
    // Right
    assert(Clipping::computeCode(101, 50) & Clipping::RIGHT);
    // Top
    assert(Clipping::computeCode(50, 101) & Clipping::TOP);
    // Bottom
    assert(Clipping::computeCode(50, -1) & Clipping::BOTTOM);
}

void test_SutherlandHodgmanClip() {
    Clipping::SetClipWindow(0, 0, 100, 100);
    std::vector<POINT> poly = { {50, 50}, {150, 50}, {150, 150}, {50, 150} };
    POINT out[10];
    int outCount = 0;
    Clipping::SutherlandHodgmanClip(poly.data(), poly.size(), out, outCount);
    // Output should be a clipped rectangle
    assert(outCount == 4);
    for (int i = 0; i < outCount; ++i) {
        assert(out[i].x >= 0 && out[i].x <= 100);
        assert(out[i].y >= 0 && out[i].y <= 100);
    }
}

int main() {
    test_set_clip_window();
    test_inside();
    test_clip_point_square();
    test_computeCode();
    test_SutherlandHodgmanClip();
    std::cout << "All Clipping unit tests passed!\n";
    return 0;
}
