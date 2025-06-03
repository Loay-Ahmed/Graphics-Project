# Compile all .cpp files in src/
all:
	g++ src/*.cpp -I. -I./include -o GraphicsProject.exe -lgdi32 -luser32 -mwindows

clean:
	del GraphicsProject.exe
