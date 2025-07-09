# Graphics-Project
Computer graphics course project

## How To run the project
-- Step 1 compile the src files
g++ src/*.cpp -I. -I./include -o GraphicsProject.exe -lgdi32 -luser32 -lcomdlg32 -mwindows

-- Step 2 Run
.\GraphicsProject.exe

## Documentation Update (2024)
- All major source files, especially `main.cpp` and `storage.cpp`, now feature improved comments and section headers.
- Comments clarify the purpose of each section, function, and complex logic for easier maintenance and onboarding.
- See `WIKI.md` for a detailed developer guide and code structure overview.
- **New:** When saving or loading layers, a standard Windows file dialog will appear, allowing you to choose the file name and location.


