from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d

# Load the image
image_path = r"Contours\chess-bishop-illustration.png"
image = Image.open(image_path).convert("L")  # Convert to grayscale

# Convert the image to a NumPy array
image_array = np.array(image)  # Convert to a 2D NumPy array (grayscale)

# Binary thresholding
threshold_value = 128 
binary_image = (image_array < threshold_value).astype(int)  # Convert to binary (1 = black, 0 = white)

# Visualize the binary image
plt.imshow(binary_image, cmap='gray')
plt.title("Binary Image (Black = 1, White = 0)")
plt.show()

# Find the last black pixel for each row
height, width = binary_image.shape
edge_points = []  # To store the (x, y) coordinates of the edge

for y in range(height):
    row = binary_image[y, :]  # Get the row (1D array of 0s and 1s)
    black_pixels = np.where(row == 1)[0]  # Find indices of all black pixels
    if black_pixels.size > 0:
        x = black_pixels[-1]  # Get the last black pixel in the row
        edge_points.append((x, y))  # Store the (x, y) coordinate
    else:
        edge_points.append((0, y))  # Default to 0 if no black pixel is found

# Normalize the coordinates
edge_points = np.array(edge_points)
x_coords = edge_points[:, 0]  # Extract x-coordinates
y_coords = edge_points[:, 1]  # Extract y-coordinates

# Normalize y-coordinates (height) between 0 and 1
step = 1

y_normalized = (y_coords / height)[::step]

# Normalize x-coordinates (edge distance) between 0 and 1
x_normalized = (x_coords / width)[::step]

# Interpolation
# Remove duplicates and ensure distinct y values
unique_points = np.unique(np.array(list(zip(y_normalized, x_normalized))), axis=0)
y_cleaned = unique_points[:, 0]
x_cleaned = unique_points[:, 1]

if len(y_cleaned) < 4:
    raise ValueError("Not enough unique points for cubic interpolation. Ensure the image is processed correctly.")

edge_function = interp1d(y_cleaned, x_cleaned, kind="cubic", fill_value="extrapolate")

# Test the function and visualize
test_heights = np.linspace(0, 1, 100)  # Generate test heights between 0 and 1
predicted_edges = edge_function(test_heights)  # Get edge distances for these heights

# Plot the results
plt.figure(figsize=(8, 6))
plt.imshow(binary_image, cmap='gray', extent=[0, 1, 0, 1])  # Show the normalized image
plt.scatter(x_normalized, y_normalized, color='red', s=5, label='Extracted Edge Points')
plt.plot(predicted_edges, test_heights, color='blue', label='Interpolated Edge')
plt.xlabel('Normalized Edge Distance (X)')
plt.ylabel('Normalized Height (Y)')
plt.title('Normalized Contour of Bishop and Interpolated Edge')
plt.legend()
plt.show()

# Example: Use the function
height_input = 0.5  # Enter a normalized height between 0 and 1
edge_distance = edge_function(height_input)
print(f"Edge distance at height {height_input}: {edge_distance}")


# ==========================================================================================================================


from scipy.interpolate import CubicSpline
import numpy as np

# Extract edge points and normalized data
x_cleaned = np.linspace(0, 1, 1000) 
y_cleaned = edge_function(x_cleaned)  

# Use CubicSpline to generate the spline
cubic_spline = CubicSpline(x_cleaned, y_cleaned, bc_type='natural')

# Extract coefficients and knots
coeffs = cubic_spline.c  # Shape: (4, n_intervals), highest to lowest degree for each interval
knots = cubic_spline.x   # The knot points (y-values)

# Generate the C++ source file content
cpp_code = """
#include <vector>
#include <cmath>

// Spline coefficients and knots
std::vector<double> knots = {""" + ", ".join(map(str, knots)) + """};
std::vector<std::vector<double>> coeffs = {
"""

# Add coefficients for each interval
for i in range(coeffs.shape[1]):  # Iterate over intervals
    cpp_code += "    {"
    cpp_code += ", ".join(map(str, coeffs[:, i]))  # Add coefficients for this interval
    cpp_code += "},\n"

cpp_code += """};

// Edge function
double edge_function(double y) {
    // Find the interval containing y
    size_t interval = 0;
    while (interval < knots.size() - 1 && y > knots[interval + 1]) {
        interval++;
    }

    // Ensure y is within valid range
    if (interval >= coeffs.size()) {
        return 0.0;  // Handle out-of-range values
    }

    // Compute the cubic polynomial for the interval
    double a = coeffs[interval][0];
    double b = coeffs[interval][1];
    double c = coeffs[interval][2];
    double d = coeffs[interval][3];
    double dy = y - knots[interval];  // Offset from the start of the interval
    return a * std::pow(dy, 3) + b * std::pow(dy, 2) + c * dy + d;
}
"""

# Generate the header file content
header_code = """
#ifndef EDGE_FUNCTION_H
#define EDGE_FUNCTION_H

double edge_function(double y);

#endif // EDGE_FUNCTION_H
"""

# Save the C++ source and header files
with open(r"edge_function.cpp", "w") as cpp_file:
    cpp_file.write(cpp_code)

with open(r"edge_function.h", "w") as header_file:
    header_file.write(header_code)

print("C++ source and header files generated: edge_function.cpp and edge_function.h")
