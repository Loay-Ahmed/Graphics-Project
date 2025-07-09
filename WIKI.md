# Graphics Project Wiki

## Overview
This project is a Windows GDI-based interactive graphics application for drawing, filling, and manipulating various geometric shapes and curves. It supports persistent layers, interactive input, and a variety of classic and advanced drawing algorithms.

---

## Code Commenting & Documentation (2024 Update)

- All major source files, especially `main.cpp` and `storage.cpp`, have been updated with improved comments for clarity and maintainability.
- **Section headers** now clearly mark major parts of the code (e.g., Drawing Modes, Global State, Utility Functions, Main Entry Point).
- **Function headers** and **inline comments** explain the purpose and logic of complex or non-obvious code blocks.
- This makes it easier for new contributors and maintainers to understand, extend, or debug the codebase.

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
    - **File Save/Load:** Uses Windows file dialogs (GetOpenFileName/GetSaveFileName) to let the user choose files for saving/loading layers.
  - **WM_LBUTTONDOWN:** Handles left-click for interactive input (adding points, starting shapes, etc.).
  - **WM_RBUTTONDOWN:** Handles right-click for finishing shapes (e.g., polygons, splines) or canceling previews.
  - **WM_MOUSEMOVE:** Handles mouse movement for previews.
  - **WM_PAINT:** Draws all layers and previews.
  - **WM_CLOSE/WM_DESTROY:** Handles window closing and cleanup.

- **Main Entry Point (`WinMain`):**
  - Registers the window class and starts the message loop.

---

## Deep Dive: main.cpp (Architecture & Event Flow)

### 1. High-Level Architecture
- **main.cpp** is the entry point and central controller for the application. It manages the Windows message loop, user interaction, menu system, and all drawing operations.
- The file is organized into logical sections: includes, enums, global state, utility functions, the main window procedure (`WndProc`), and the WinMain entry point.

### 2. Menu System & User Interaction
- **Menus:** All drawing tools, algorithms, colors, and extra features are exposed via Windows menus. Menus are created in the `WM_CREATE` handler using `CreateMenu` and `AppendMenu`.
- **Menu IDs:** Each menu item is assigned a unique ID. These IDs are used in the `WM_COMMAND` handler to determine which action to take (e.g., change drawing mode, select algorithm, open file dialog).
- **Shape/Algorithm Selection:** Selecting a shape or algorithm from the menu updates the global state (e.g., `currentShape`, `currentCircleAlg`). This state determines how subsequent mouse events are interpreted.

### 3. Global State & Layer System
- **Global Variables:** Track the current drawing mode, selected shape, algorithm, color, and user input points. These variables are critical for managing interactive drawing and previews.
- **Layer System:** All drawn objects (lines, circles, polygons, fills, etc.) are stored as `Layer` objects in a vector. Each `Layer` is a variant holding the parameters and color for a specific shape. This enables persistent, multi-shape drawing and easy redrawing on window refresh.
- **Interactive State:** Additional variables track the state of ongoing interactive operations (e.g., polygon point collection, preview lines, extra features like Bezier waves).

### 4. Window Procedure (`WndProc`)
- **Central Event Handler:** `WndProc` is the heart of the application, handling all Windows messages:
  - **WM_CREATE:** Sets up menus, window background, and cursor.
  - **WM_COMMAND:** Handles all menu actions (shape/tool selection, algorithm changes, color picker, file dialogs, help/manual, etc.). Updates global state and triggers UI updates.
  - **WM_LBUTTONDOWN:** Handles left mouse clicks for interactive drawing. Depending on the current shape/tool, it collects points, starts or completes shapes, or triggers special actions (e.g., filling, extra draw methods).
  - **WM_RBUTTONDOWN:** Handles right mouse clicks, typically to finish polygons/splines or cancel previews.
  - **WM_MOUSEMOVE:** Updates preview positions for lines, polygons, and other interactive shapes, providing real-time visual feedback.
  - **WM_PAINT:** Redraws all layers and previews. Uses the current state to determine what to draw and how (including which algorithm to use for each shape).
  - **WM_CLOSE/WM_DESTROY:** Handles window closure and cleanup.

### 5. Drawing Logic & Algorithms
- **Layer Redraw:** In `WM_PAINT`, the application iterates over all layers and uses `std::visit` to dispatch to the correct drawing function for each shape type and algorithm.
- **Algorithm Selection:** The selected algorithm for lines, circles, ellipses, and filling is stored in global variables and used to determine which drawing function to call.
- **Previews:** While the user is interacting (e.g., dragging to set a line endpoint), preview shapes are drawn using dotted lines or temporary graphics.
- **Extensibility:** New shapes or algorithms can be added by extending the `Layer` variant, updating the menu, and adding the appropriate drawing logic in `WM_PAINT`.

### 6. File Operations & Persistence
- **Save/Load:** The File menu allows saving and loading of all layers using standard Windows file dialogs. The `Storage` module handles serialization and deserialization of layers.
- **Clear:** The 'Clear' menu item resets all layers and state, providing a fresh canvas.

### 7. Error Handling & User Feedback
- **Validation:** The code checks for valid input (e.g., minimum points for polygons/splines, valid fill points) and provides user feedback via message boxes.
- **Help/Manual:** The Help menu displays a detailed manual for users, describing all features and input methods.

### 8. Main Entry Point (`WinMain`)
- **Window Registration:** Registers the window class and creates the main application window.
- **Message Loop:** Runs the standard Windows message loop, dispatching messages to `WndProc`.

### 9. Best Practices & Design Notes
- **Separation of Concerns:** Drawing logic, state management, and UI handling are clearly separated, making the codebase maintainable and extensible.
- **Commenting:** The code is heavily commented, with section headers and inline explanations for complex logic.
- **Layer System:** The use of a layer system enables persistent, undoable drawing and easy extension for new features.

---

## storage.cpp Structure (Developer Guide)

- **Purpose:** Handles saving/loading of drawings and layers, and canvas management.
- **Key Methods:**
  - `saveToFile()`: Save the current drawing (all pixels) to a file.
  - `clearCanvas(HWND hwnd)`: Clear the canvas and remove all drawings.
  - `setCanvas(HDC hdc)`: Redraw the canvas from the stored drawings.
  - `loadFromFile(HDC hdc)`: Load a drawing from file and update the canvas.
  - `saveLayersToFile(const std::vector<Layer>&, const std::string&)`: Save all layers to a file for persistence.
  - `loadLayersFromFile(std::vector<Layer>&, const std::string&)`: Load all layers from a file for persistence.
- **Helpers:**
  - `point_to_str`, `str_to_point`: Serialize/deserialize POINT structures for file I/O.

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
- **File Dialogs for Save/Load:**
  - When saving or loading layers, a standard Windows file dialog allows you to choose the file name and location.

---

## Usage
### General
- Select a shape/tool from the 'Draw Shape' menu.
- Use the 'Color' menu to change drawing color.
- Use 'Clear' to erase all layers.
- **To Save/Load Layers:**
  - Choose 'Save' or 'Load' from the File menu. A file dialog will appear for you to select or name the file.
  - Only layer-based save/load uses the file dialog (not pixel-based drawing save/load).

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
