# -*- coding: utf-8 -*-
"""
Created on Sat Dec  6 15:09:54 2025

@author: guans
"""

import pybullet as p
import pybullet_data
import numpy as np
import time
import matplotlib.pyplot as plt

# --- Simulation Setup ---
p.connect(p.GUI)
p.setAdditionalSearchPath(pybullet_data.getDataPath())
p.setGravity(0, 0, -9.8)

# Load plane
plane_id = p.loadURDF("plane.urdf")

# Load a floating robot
robot_start_pos = [0, 0, 0.5]
robot_start_ori = p.getQuaternionFromEuler([0, 0, 0])
robot_id = p.loadURDF("r2d2.urdf", robot_start_pos, robot_start_ori, useFixedBase=False)
p.changeDynamics(robot_id, -1, angularDamping=100)
# Add static obstacles
# --- Modified static objects ---
def add_static_objects():
    objs = []
    
    # Walls - asymmetric design
    wall_height = 1.0
    wall_thickness = 0.2
    
    # Wall coordinates and dimensions (position, size, color)
    maze_walls = [
        # Outer boundary
        [[8, 0, wall_height/2], [wall_thickness, 8, wall_height/2], [0.7, 0.5, 0.3, 1]],   # Right wall
        [[-8, 0, wall_height/2], [wall_thickness, 8, wall_height/2], [0.7, 0.5, 0.3, 1]],  # Left wall
        [[0, 8, wall_height/2], [8, wall_thickness, wall_height/2], [0.7, 0.5, 0.3, 1]],   # Top wall
        [[0, -8, wall_height/2], [8, wall_thickness, wall_height/2], [0.7, 0.5, 0.3, 1]],  # Bottom wall
        
        # Internal maze structures - intentionally asymmetric
        # Region 1 obstacles
        [[0, 4, wall_height/2], [6, wall_thickness, wall_height/2], [0.5, 0.7, 0.3, 1]],    # Horizontal wall 1
        [[-3, 2, wall_height/2], [wall_thickness, 2, wall_height/2], [0.5, 0.7, 0.3, 1]],   # Vertical wall 1
        [[3, 0, wall_height/2], [wall_thickness, 4, wall_height/2], [0.5, 0.7, 0.3, 1]],    # Vertical wall 2
        
        # Region 2 obstacles
        [[-5, -2, wall_height/2], [3, wall_thickness, wall_height/2], [0.3, 0.5, 0.7, 1]],  # Horizontal wall 2
        [[-2, -4, wall_height/2], [2, wall_thickness, wall_height/2], [0.3, 0.5, 0.7, 1]],  # Horizontal wall 3
        [[-6, -5, wall_height/2], [wall_thickness, 1, wall_height/2], [0.3, 0.5, 0.7, 1]],  # Vertical wall 3
        
        # Region 3 obstacles
        [[5, -3, wall_height/2], [1, wall_thickness, wall_height/2], [0.7, 0.3, 0.5, 1]],   # Short horizontal wall
        [[6, -1, wall_height/2], [wall_thickness, 3, wall_height/2], [0.7, 0.3, 0.5, 1]],   # Long vertical wall
        
        # Region 4 - zigzag corridor
        [[1, -6, wall_height/2], [5, wall_thickness, wall_height/2], [0.8, 0.6, 0.2, 1]],   # Bottom wall
        [[-1, -2, wall_height/2], [1, wall_thickness, wall_height/2], [0.8, 0.6, 0.2, 1]],  # Small obstacle
    ]
    
    for pos, half_extents, color in maze_walls:
        visual_shape = p.createVisualShape(p.GEOM_BOX, halfExtents=half_extents, rgbaColor=color)
        collision_shape = p.createCollisionShape(p.GEOM_BOX, halfExtents=half_extents)
        obj_id = p.createMultiBody(baseMass=0,
                                   baseCollisionShapeIndex=collision_shape,
                                   baseVisualShapeIndex=visual_shape,
                                   basePosition=pos)
        objs.append(obj_id)
    

    import random
    random.seed(0)
    # Random obstacle blocks placed in corridors
    for i in range(12):  # More random obstacles
        while True:
            x = random.uniform(-7, 7)
            y = random.uniform(-7, 7)
            
            # Avoid robot spawn position
            if abs(x) < 1.5 and abs(y) < 1.5:
                continue
                
            # Avoid placing directly on walls
            too_close_to_wall = False
            for wall_pos, wall_size, _ in maze_walls:
                if (abs(x - wall_pos[0]) < wall_size[0] + 0.5 and 
                    abs(y - wall_pos[1]) < wall_size[1] + 0.5):
                    too_close_to_wall = True
                    break
            
            if not too_close_to_wall:
                break
        
        # Random block size
        size_x = random.uniform(0.1, 0.4)
        size_y = random.uniform(0.1, 0.4)
        size_z = random.uniform(0.1, 0.6)
        
        color = [random.uniform(0.1, 0.9), 
                 random.uniform(0.1, 0.9), 
                 random.uniform(0.1, 0.9), 1]
        
        visual_shape = p.createVisualShape(p.GEOM_BOX, halfExtents=[size_x, size_y, size_z], rgbaColor=color)
        collision_shape = p.createCollisionShape(p.GEOM_BOX, halfExtents=[size_x, size_y, size_z])
        obj_id = p.createMultiBody(baseMass=0,
                                   baseCollisionShapeIndex=collision_shape,
                                   baseVisualShapeIndex=visual_shape,
                                   basePosition=[x, y, size_z])
        objs.append(obj_id)
    
    # Add several special-shape obstacles
    special_obstacles = [
        # position, rotation, type, size, color
        [[-7, 0, 0.5], [0, 0, 0], "box", [0.3, 0.6, 0.3], [0.8, 0.2, 0.2, 1]],    # Long bar
        [[7, 0, 0.5], [0, 0, 0], "box", [0.6, 0.3, 0.3], [0.2, 0.8, 0.2, 1]],     # Flat block
        [[0, 7, 0.75], [0, 0, np.deg2rad(45)], "box", [0.4, 0.4, 0.4], [0.2, 0.2, 0.8, 1]],   # Rotated cube
    ]
    
    for pos, euler, shape_type, size, color in special_obstacles:
        if shape_type == "box":
            visual_shape = p.createVisualShape(p.GEOM_BOX, halfExtents=size, rgbaColor=color)
            collision_shape = p.createCollisionShape(p.GEOM_BOX, halfExtents=size)
        obj_id = p.createMultiBody(baseMass=0,
                                   baseCollisionShapeIndex=collision_shape,
                                   baseVisualShapeIndex=visual_shape,
                                   basePosition=pos,
                                   baseOrientation=p.getQuaternionFromEuler(euler))
        objs.append(obj_id)
    
    print(f"Maze environment created, total {len(objs)} objects")
    
    return objs

structures = add_static_objects()

# Simulation params
dt = 1./240.
p.setTimeStep(dt)
speed_xy = 10.0
speed_z = 10.0

# --- Robot control ---
def move_robot_linear(robot_id, vx, vy, vz):
    # pos, ori = p.getBasePositionAndOrientation(robot_id)
    # new_pos = [pos[0] + vx*dt, pos[1] + vy*dt, pos[2] + vz*dt]
    # p.resetBasePositionAndOrientation(robot_id, new_pos, ori)
    p.resetBaseVelocity(robot_id, linearVelocity=[vx, vy, vz], angularVelocity=[0, 0, 0])

# --- Reset robot angular ---
def upright_robot(robot_id):
    pos, _ = p.getBasePositionAndOrientation(robot_id)
    upright_ori = p.getQuaternionFromEuler([0, 0, 0])
    p.resetBasePositionAndOrientation(robot_id, pos, upright_ori)
    

# --- 2D Lidar with 1-degree resolution (no gaps) ---
def simulate_lidar_full_360(robot_id, num_rays=360, max_dist=10.0):
    pos, ori = p.getBasePositionAndOrientation(robot_id)
    yaw = p.getEulerFromQuaternion(ori)[2]
    points_x = []
    points_y = []
    
    aabb_min, aabb_max = p.getAABB(robot_id)
    
    robot_radius = max(aabb_max[0]-aabb_min[0], aabb_max[1]-aabb_min[1]) / 2.0
    offset = robot_radius * 0.1 
    
    for i in range(num_rays):
        angle = yaw + i * (2*np.pi / num_rays)
        
        start_x = pos[0] + (robot_radius + offset) * np.cos(angle)
        start_y = pos[1] + (robot_radius + offset) * np.sin(angle)
        start_z = pos[2]  
        
        start = [start_x, start_y, start_z]
        end = [start_x + max_dist*np.cos(angle), 
               start_y + max_dist*np.sin(angle), 
               start_z]  
        
        result = p.rayTest(start, end)[0]
        hit_fraction = result[2]
        
        if hit_fraction < 1.0:
            hit_x = start_x + hit_fraction * max_dist * np.cos(angle)
            hit_y = start_y + hit_fraction * max_dist * np.sin(angle)
            points_x.append(hit_x)
            points_y.append(hit_y)
        else:
            points_x.append(end[0])
            points_y.append(end[1])
    
    return np.array(points_x), np.array(points_y)

# --- IMU ---
class IMU:
    def __init__(self, robot_id):
        self.robot_id = robot_id
        self.prev_linear_vel = np.zeros(3)
    def read(self):
        lin_vel, ang_vel = p.getBaseVelocity(self.robot_id)
        lin_acc = (np.array(lin_vel) - self.prev_linear_vel)/dt
        self.prev_linear_vel = np.array(lin_vel)
        return np.array(lin_acc), np.array(ang_vel)

imu = IMU(robot_id)

# --- Matplotlib Setup ---
plt.ion()
fig, ax = plt.subplots()
lidar_plot, = ax.plot([], [], 'r.', markersize=3)
robot_dot, = ax.plot([], [], 'bo', markersize=8)
ax.set_xlim(-10, 10)
ax.set_ylim(-10, 10)
ax.set_aspect('equal')
ax.set_title("2D Lidar Scan Visualization")
ax.grid(True, alpha=0.3)

# --- Main Loop ---
counter = 0

while True:
    p.stepSimulation()
    
    counter += 1

    # Robot control via keyboard
    keys = p.getKeyboardEvents()
    vx = vy = vz = 0
    if p.B3G_UP_ARROW in keys and keys[p.B3G_UP_ARROW] & p.KEY_IS_DOWN: vx = speed_xy
    if p.B3G_DOWN_ARROW in keys and keys[p.B3G_DOWN_ARROW] & p.KEY_IS_DOWN: vx = -speed_xy
    if p.B3G_LEFT_ARROW in keys and keys[p.B3G_LEFT_ARROW] & p.KEY_IS_DOWN: vy = speed_xy
    if p.B3G_RIGHT_ARROW in keys and keys[p.B3G_RIGHT_ARROW] & p.KEY_IS_DOWN: vy = -speed_xy
    if p.B3G_SPACE in keys and keys[p.B3G_SPACE] & p.KEY_IS_DOWN: vz = speed_z
    if p.B3G_SHIFT in keys and keys[p.B3G_SHIFT] & p.KEY_IS_DOWN: vz = -speed_z
    if ord('u') in keys and keys[ord('u')] & p.KEY_IS_DOWN:upright_robot(robot_id)
    
    move_robot_linear(robot_id, vx, vy, vz)

    # Read sensors
    px, py = simulate_lidar_full_360(robot_id, num_rays=360)
    pos, _ = p.getBasePositionAndOrientation(robot_id)
    
    lin_acc, ang_vel = imu.read()

    # Update Matplotlib
    lidar_plot.set_data(px, py)
    robot_dot.set_data([pos[0]], [pos[1]])
    fig.canvas.draw()
    fig.canvas.flush_events()

    time.sleep(dt)
