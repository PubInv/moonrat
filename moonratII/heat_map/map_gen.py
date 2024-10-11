import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import griddata
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm

# Temperature measurement data
# Replace these with your actual measurements
x_ = np.cos(np.pi/4)
y_ = np.sin(np.pi/4)
data = np.array([
    # x, y, z, temperature
    [0, 0, 0, 37.7],   #! Center
    [0, 0, 2.5, 34.8],   #! Center
    [0, 0, 5.0, 36.1],   #! Center
    [-1, 0, 0, 40.9],     #! Base
    [1, 0, 0, 40.8],     #! Base
    [0, -1, 0, 39.1],     #! Base
    [0, 1, 0, 41.6],     #! Base
    [-1, 0, 2.5, 41.7],     #! Mid
    [1, 0, 2.5, 41.2],     #! Mid
    [0, -1, 2.5, 39.9],     #! Mid
    [0, 1, 2.5, 43.2],     #! Mid
    [-1, 0, 5.0, 38.2],     #! Top
    [1, 0, 5.0, 37.8],     #! Top
    [0, -1, 5.0, 37.1],     #! Top
    [0, 1, 5.0, 38.7],     #! Top
    [-x_, -y_, 0, 40.7],    #! Base
    [x_, y_, 0, 40.5],     #! Base
    [y_, x_, 0, 40.1],     #! Base
    [-y_, x_, 0, 40.5],     #! Base
    [-x_, -y_, 2.5, 40.8],     #! Mid
    [x_, y_, 2.5, 40.3],     #! Mid
    [y_, -x_, 2.5, 42.8],     #! Mid
    [-y_, x_, 2.5, 42.5],     #! Mid
    [-x_, -y_, 5.0, 37.5],     #! Top
    [x_, y_, 5.0, 37.3],     #! Top
    [y_, -x_, 5.0, 38.7],     #! Top
    [-y_, x_, 5.0, 38.5],     #! Top
])

# Extract coordinates and temperatures
x_samples = data[:, 0]
y_samples = data[:, 1]
z_samples = data[:, 2]
temperatures = data[:, 3]

radius = 1       # Cylinder radius
height = 5       # Cylinder height

# --- Lateral View ---

# Create a mesh in the XZ plane for the lateral view
num_x = 100    # Number of points in the X direction
num_z = 100    # Number of points in the Z direction

x = np.linspace(-radius, radius, num_x)
z = np.linspace(0, height, num_z)
x_grid, z_grid = np.meshgrid(x, z)

# For each point in the XZ mesh, calculate the maximum Y value within the cylinder
# This gives us the Y range for each X
num_y = 50  # Number of points in the Y direction for better precision
y = np.linspace(-radius, radius, num_y)

# Create a 3D mesh of X, Y, Z inside the cylinder
x_interp, y_interp, z_interp = np.meshgrid(x, y, z, indexing='ij')

# Filter points that are inside the cylinder
inside_cylinder = x_interp**2 + y_interp**2 <= radius**2

# Flatten the arrays and filter points inside the cylinder
x_points = x_interp[inside_cylinder]
y_points = y_interp[inside_cylinder]
z_points = z_interp[inside_cylinder]

# Interpolate temperatures at these points
grid_points = np.column_stack((x_points, y_points, z_points))

temperatures_grid = griddata(
    points=(x_samples, y_samples, z_samples),
    values=temperatures,
    xi=grid_points,
    method='linear'
)

# Handle NaN values in the interpolation
temperatures_grid = np.nan_to_num(temperatures_grid, nan=np.nanmean(temperatures))

# Create a 2D mesh for the lateral view
# Average temperatures along Y for each X and Z
temperatures_lateral = np.zeros((num_z, num_x))

for i in range(num_z):
    for j in range(num_x):
        # Select points with fixed X and Z coordinates
        mask = (x_points == x[j]) & (z_points == z[i])
        if np.any(mask):
            temperatures_lateral[i, j] = np.mean(temperatures_grid[mask])
        else:
            temperatures_lateral[i, j] = np.nan

# Replace NaN with the average temperature
temperatures_lateral = np.nan_to_num(temperatures_lateral, nan=np.nanmean(temperatures))

# Plot the heat map
plt.figure(figsize=(8, 6))
c = plt.contourf(x_grid, z_grid, temperatures_lateral, levels=100, cmap='YlOrRd')
plt.colorbar(c)
plt.title('Heat Map - Lateral View')
plt.xlabel('X (diameter)')
plt.ylabel('Height Z')
plt.show()

# --- Top View ---

# Create a mesh in the XY plane at mid-height
num_theta = 100
theta = np.linspace(0, 2 * np.pi, num_theta)
r = np.linspace(0, radius, num_x)
r_grid, theta_grid = np.meshgrid(r, theta)

# Convert to Cartesian coordinates
x_grid_top = r_grid * np.cos(theta_grid)
y_grid_top = r_grid * np.sin(theta_grid)
z_mid = height / 2  # Mid-height

# Flatten the arrays for interpolation
grid_points_top = np.column_stack((x_grid_top.ravel(), y_grid_top.ravel(), np.full(x_grid_top.size, z_mid)))

# Interpolate temperatures on the mesh
temperatures_top = griddata(
    points=(x_samples, y_samples, z_samples),
    values=temperatures,
    xi=grid_points_top,
    method='linear'
)

# Reshape to get the original mesh shape
temperatures_top = temperatures_top.reshape(r_grid.shape)

# Handle NaN values in the interpolation
temperatures_top = np.nan_to_num(temperatures_top, nan=np.nanmean(temperatures))

# Plot the heat map
plt.figure(figsize=(6, 6))
c = plt.contourf(x_grid_top, y_grid_top, temperatures_top, levels=100, cmap='YlOrRd')
plt.colorbar(c)
plt.title('Heat Map - Top View')
plt.xlabel('X')
plt.ylabel('Y')
plt.gca().set_aspect('equal')
plt.show()

# --- 3D View ---

fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Plot the cylinder contour
z_cylinder = np.linspace(0, height, 100)
theta_cylinder = np.linspace(0, 2 * np.pi, 100)
theta_cylinder, z_cylinder = np.meshgrid(theta_cylinder, z_cylinder)
x_cylinder = radius * np.cos(theta_cylinder)
y_cylinder = radius * np.sin(theta_cylinder)

ax.plot_surface(x_cylinder, y_cylinder, z_cylinder, color='grey', alpha=0.2, edgecolor='none')

# Add the measurement points with heat irradiation effect clipped at the cylinder boundaries
for xi, yi, zi, ti in zip(x_samples, y_samples, z_samples, temperatures):
    color = cm.YlOrRd((ti - temperatures.min()) / (temperatures.max() - temperatures.min()))
    u, v = np.mgrid[0:2*np.pi:50j, 0:np.pi:25j]
    r_point = 0.5  # Radius of the sphere
    x_sphere = r_point * np.cos(u) * np.sin(v) + xi
    y_sphere = r_point * np.sin(u) * np.sin(v) + yi
    z_sphere = 2*r_point * np.cos(v) + zi # NOTE The multiplication by 2 is only to adjust the perspective by the scale of the axes in the image

    # Create a mask for points inside the cylinder
    inside_cylinder = (x_sphere**2 + y_sphere**2 <= radius**2) & (z_sphere >= 0) & (z_sphere <= height)

    # Apply the mask to the sphere coordinates
    x_sphere_clipped = np.ma.masked_where(~inside_cylinder, x_sphere)
    y_sphere_clipped = np.ma.masked_where(~inside_cylinder, y_sphere)
    z_sphere_clipped = np.ma.masked_where(~inside_cylinder, z_sphere)

    # Plot the clipped sphere
    ax.plot_surface(x_sphere_clipped, y_sphere_clipped, z_sphere_clipped, color=color, alpha=0.5, edgecolor='none')

# Add the original measurement points
ax.scatter(x_samples, y_samples, z_samples, c=temperatures, cmap='YlOrRd', s=100, edgecolors='k')

# Adjust axis limits
ax.set_xlim(-radius * 1.5, radius * 1.5)
ax.set_ylim(-radius * 1.5, radius * 1.5)
ax.set_zlim(0, height)

ax.set_title('Heat Map - 3D View with Clipped Irradiation')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
plt.show()
