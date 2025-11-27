import pybullet as p
import pybullet_data
import numpy as np
import time
import matplotlib.pyplot as plt

# --- Simulation Setup ---
p.connect(p.GUI)
p.setAdditionalSearchPath(pybullet_data.getDataPath())
p.setGravity(0, 0, 0)  # No gravity

# Load plane
plane_id = p.loadURDF("plane.urdf")

# Load a floating robot
robot_start_pos = [0, 0, 1]
robot_start_ori = p.getQuaternionFromEuler([0, 0, 0])
robot_id = p.loadURDF("r2d2.urdf", robot_start_pos, robot_start_ori, useFixedBase=False)

# Add static obstacles
def add_static_objects():
    objs = []
    visual_shape_id = p.createVisualShape(p.GEOM_BOX, halfExtents=[0.5,0.5,0.5], rgbaColor=[0,0,1,1])
    collision_shape_id = p.createCollisionShape(p.GEOM_BOX, halfExtents=[0.5,0.5,0.5])
    positions = [[3,3,0.5],[-3,3,0.5],[3,-3,0.5],[-3,-3,0.5]]
    for pos in positions:
        obj_id = p.createMultiBody(baseMass=0,
                                   baseCollisionShapeIndex=collision_shape_id,
                                   baseVisualShapeIndex=visual_shape_id,
                                   basePosition=pos)
        objs.append(obj_id)
    return objs

structures = add_static_objects()

# Simulation params
dt = 1./240.
p.setTimeStep(dt)
speed_xy = 10.0  # faster XY movement
speed_z = 10.0   # vertical movement

# --- Robot control ---
def move_robot_linear(robot_id, vx, vy, vz):
    pos, ori = p.getBasePositionAndOrientation(robot_id)
    new_pos = [pos[0] + vx*dt, pos[1] + vy*dt, pos[2] + vz*dt]
    p.resetBasePositionAndOrientation(robot_id, new_pos, ori)

# --- 2D Lidar with 1-degree resolution (no gaps) ---
def simulate_lidar_full_360(robot_id, num_rays=360, max_dist=10.0):
    pos, ori = p.getBasePositionAndOrientation(robot_id)
    yaw = p.getEulerFromQuaternion(ori)[2]
    points_x = []
    points_y = []

    for i in range(num_rays):
        angle = yaw + i * (2*np.pi / num_rays)
        start = list(pos)
        end = [pos[0] + max_dist*np.cos(angle), pos[1] + max_dist*np.sin(angle), pos[2]]
        result = p.rayTest(start, end)[0]
        hit_body = result[0]
        if hit_body == robot_id:
            dist = max_dist  # ignore hits on self
        else:
            dist = result[2] * max_dist

        points_x.append(pos[0] + dist*np.cos(angle))
        points_y.append(pos[1] + dist*np.sin(angle))

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
lidar_plot, = ax.plot([], [], 'r.', markersize=2)
robot_dot, = ax.plot([], [], 'bo', markersize=6)
ax.set_xlim(-10, 10)
ax.set_ylim(-10, 10)
ax.set_aspect('equal')
ax.set_title("2D Lidar Scan Visualization")

# --- Main Loop ---
while True:
    p.stepSimulation()

    # Robot control
    keys = p.getKeyboardEvents()
    vx = vy = vz = 0
    if p.B3G_UP_ARROW in keys and keys[p.B3G_UP_ARROW] & p.KEY_IS_DOWN: vx = speed_xy
    if p.B3G_DOWN_ARROW in keys and keys[p.B3G_DOWN_ARROW] & p.KEY_IS_DOWN: vx = -speed_xy
    if p.B3G_LEFT_ARROW in keys and keys[p.B3G_LEFT_ARROW] & p.KEY_IS_DOWN: vy = speed_xy
    if p.B3G_RIGHT_ARROW in keys and keys[p.B3G_RIGHT_ARROW] & p.KEY_IS_DOWN: vy = -speed_xy
    if p.B3G_SPACE in keys and keys[p.B3G_SPACE] & p.KEY_IS_DOWN: vz = speed_z
    if p.B3G_SHIFT in keys and keys[p.B3G_SHIFT] & p.KEY_IS_DOWN: vz = -speed_z

    move_robot_linear(robot_id, vx, vy, vz)

    # Read sensors
    px, py = simulate_lidar_full_360(robot_id, num_rays=360)
    pos, _ = p.getBasePositionAndOrientation(robot_id)

    # Update Matplotlib
    lidar_plot.set_data(px, py)
    robot_dot.set_data([pos[0]], [pos[1]])
    fig.canvas.draw()
    fig.canvas.flush_events()

    time.sleep(dt)
