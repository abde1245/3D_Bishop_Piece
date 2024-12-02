This project renders a 3D model of a chess bishop using OpenGL and C++.  The bishop's profile is derived from a 2D image and uses cubic spline interpolation for a smooth, accurate representation.

## Project Structure

* **bishop.cpp:**  The main C++ source file containing the OpenGL rendering code, user interaction logic (keyboard, mouse), and integration with the edge function.
* **edge_function.cpp:** C++ implementation of the cubic spline edge function.  This function takes a normalized height (y-coordinate) as input and returns the corresponding normalized edge distance (x-coordinate) to define the bishop's profile.
* **edge_function.h:** Header file for the edge function, declaring its signature.
* **contour_equation.py:** Python script used to extract the contour from an image (`Contours/chess-bishop-illustration.png`), normalize the edge points, perform cubic spline interpolation, and generate the `edge_function.cpp` and `edge_function.h` files. This script automates the process of converting the image contour to a usable C++ function.
* **Contours/:** Folder containing the source images used for contour extraction. Notably, `chess-bishop-illustration.png` is the primary image used in this project.
* **Rendered_Model_Pics/:**  A folder intended to store screenshots or rendered images of the 3D bishop model.
* **ReadMe.txt:** This file.

## Building and Running

1. **Dependencies:** Ensure you have OpenGL and GLUT (Freeglut) libraries installed.
2. **Compilation:** Compile `bishop.cpp` using a suitable C++ compiler (e.g., g++). Make sure to link the necessary OpenGL and GLUT libraries. Example:
```shell
g++ bishop.cpp -o bishop -lglut -lGL -lGLU
```
