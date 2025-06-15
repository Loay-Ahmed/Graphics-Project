# Graphics Project Wiki

## Overview
This project is a Windows GDI-based interactive graphics application for drawing, filling, and manipulating various geometric shapes and curves. It supports persistent layers, interactive input, and a variety of classic and advanced drawing algorithms.

---

## main.cpp Structure (Developer Guide)

The `main.cpp` file is the core of the application. Here is its high-level structure to help with development and extension:

- **Includes & Enums:**
  - Includes all headers for shapes, curves, filling, and utilities.
  - Defines enums for drawing modes, menu shape types, algorithms, and clipping types.

- **Global State Variables:**
  - Current drawing mode, selected shape, algorithm, color, and user input points.
  - Layer system: all drawn shapes and fills are stored as `Layer` objects in a vector.
  - State for interactive/extra drawing methods (e.g., Bezier, Cardinal Spline, quarter circles, etc.).

- **Layer System:**
  - Each shape type (line, circle, ellipse, polygon, etc.) has a corresponding `Layer` struct.
  - All layers are stored in a vector and redrawn in `WM_PAINT`.

- **Utility Functions:**
  - Logging, polygon drawing, and other helpers.

- **Window Procedure (`WndProc`):**
  - Handles all Windows messages (menu commands, mouse/keyboard events, painting, etc.).
  - **WM_CREATE:** Sets up menus and window properties.
  - **WM_COMMAND:** Handles menu selections (shape/tool selection, color, help, etc.).
  - **WM_LBUTTONDOWN:** Handles left-click for interactive input (adding points, starting shapes, etc.).
  - **WM_RBUTTONDOWN:** Handles right-click for finishing shapes (e.g., polygons, splines) or canceling previews.
  - **WM_MOUSEMOVE:** Handles mouse movement for previews.
  - **WM_PAINT:** Draws all layers and previews.
  - **WM_CLOSE/WM_DESTROY:** Handles window closing and cleanup.

- **Main Entry Point (`WinMain`):**
  - Registers the window class and starts the message loop.

---

## Features
- **Line Drawing:** DDA, Midpoint, Parametric
- **Circle Drawing:** Direct, Polar, Iterative Polar, Midpoint, Modified Midpoint
- **Ellipse Drawing:** Direct, Polar, Midpoint
- **Polygon Drawing:** Interactive, with validation
- **Rectangle Drawing**
- **Point Drawing**
- **Bezier (Cubic) Spline:** 4-point interactive input
- **Cardinal Spline:** Interactive, any number of points (min 4)
- **Clipping:** Rectangle and Square window, supports lines, polygons, and points
- **Filling:** Recursive/Non-Recursive Flood, Convex, Non-Convex
- **Extra Draw Methods:**
  - Quarter Circles Filling
  - Rectangle Bezier Waves
  - Circle Quarter Fill
  - Square Hermite Waves

---

## Usage
### General
- Select a shape/tool from the 'Draw Shape' menu.
- Use the 'Color' menu to change drawing color.
- Use 'Clear' to erase all layers.

### Shape Input
- **Line:** Left-click start, then end point.
- **Circle/Ellipse/Rectangle:** Left-click two points.
- **Polygon:** Left-click to add points, right-click to finish.
- **Point:** Left-click to place.
- **Bezier Spline:** Left-click 4 control points.
- **Cardinal Spline:** Left-click to add points (min 4), right-click to draw. Preview circles show input points. Error if fewer than 4 points.

### Clipping
- Select 'Clipping', left-click two corners. Choose window type from menu.
- Only the last drawn shape is clipped.

### Filling
- Select 'Filling', then left-click inside a shape to fill it using the selected algorithm.

### Extra Draw Methods
- **Quarter Circles Filling:** Click center, radius, then quarter.
- **Rectangle Bezier Waves:** Click two corners.
- **Circle Quarter Fill:** Click center, radius, then quarter.
- **Square Hermite Waves:** Click top-left, then size.

---

## Layers & Persistence
- All shapes and fills are stored as layers and are persistent across redraws.
- Undo/redo is not implemented, but clearing removes all layers.

---

## Code Structure
- `src/main.cpp`: Main application logic, event handling, drawing, and menu.
- `include/` and `src/`: Shape, curve, filling, and utility implementations.
- `test/`: Test files for individual modules.

---

## Contributing
- Please comment your code and update the manual/help text for any new features.
- Use the layer system for all new persistent shapes.

---

## Troubleshooting
- If a shape does not appear, ensure you have completed all required input points.
- For Cardinal Spline, you must provide at least 4 points (8 clicks).
- If you see errors about `<variant>` or `<optional>`, check your compiler and include paths.

---

## License
This project is for educational use. See LICENSE for details.
