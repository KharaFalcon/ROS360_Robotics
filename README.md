
# ROS2 Application - Husky Robot Simulation

Clearpath husky robot that navigates through a corridor autonomously until a red object is detected, to which it drives onto. It also detects a blue object using the odom frame. To implement this, we use the likes of Robot Operating System(ROS) where we divide operations into nodes to handle. To simulate the robot in the environment we use a physics engine called Ignition Gazebo Simulator.


## Getting Started
Follow the default instructions for installing [ROS 2 Humble desktop](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html).

## How to Build and Run
### 1. Build the Package
Ensure the code is placed in a ROS 2 workspace. Then, build the workspace:
```bash
colcon build
```

### 2. Source the Workspace
Source the ROS 2 workspace:

```bash
source install/setup.bash
```

### 3. Run the Node
Launch the node:
```bash
ros2 run kmf3_ros360_cpp assignment24 --ros-args -r __ns:=/a200_0000 -r /tf:=tf \ -r /tf_static:=tf_static
```

### 3. Launch
Launch the node:
```bash
ros2 launch kmf3_ros360_cpp assignment.launch.py
```


## Features

- Laser Scanner: Reads data from a laser scanner to detect obstacles and adjust the robots movement.

- Color Detection: Subscribes to a color-detection topic for identifying objects based on color points.

- Dynamic Parameter Update: Based on RGB values for colour detection

- State Machine Navigation: Implements state-based control for forward movement, object alignment, stopping, and spinning.


## State Machine 

#### 1. State 1 - Forward Movement:
Robot moves forward, adjusts speed based on laser scanner data.
It then transitions to State 2 when an object is detected.

#### 2. State 2 - Object Alignment:
Robot aligns itself with the detected object using color points.
It then transitions to State 3 when the object is close.

#### 3. State 3 - Object Reached:
Stops the robot upon reaching the object.
It then transitions to State 4.

#### 4. State 4 - Spin in Place:
Spins the robot while updating the color detection limits dynamically.
## Packages

- geometry_msgs
- sensor_msgs
- ros360_cv_msgs
- rclcpp
- rcl_interfaces
- tf2
- tf2_ros
- tf_static
## Authors

- Khara Falcon


## Acknowledgements

Aberystywth University - Module CS36010


Quigley, Morgan, Brian Gerkey, and William D Smart. Programming Robots with ROS. First edition. Sebastopol: O’Reilly & Associates Incorporated, 2015. Print.


# ROS360_Robotics
