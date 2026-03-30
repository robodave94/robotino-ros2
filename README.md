# RTO-ROS 2

This repository contains code for packages that enable ROS 2 support for the **Festo Robotino 3**.

This repo is verified to work on Ubuntu 20.04 LTS with ROS2 Foxy and Ubuntu 24.04 LTS with ROS2 Jazzy.

The original code was taken from [dietriro's robotino repository](https://github.com/dietriro/rto_core) and adapted for ROS2

**Note:** Not all functionality has been enabled. North Star support, BHA support, and gripper support have been disabled.

## Installation instructions for Host PC
- Go to [[https://wiki.openrobotino.org/index.php?title=Robotino_OS]] and download the .deb files provided under Ubuntu 20.04 for robotino-dev, rec-rpc, and robotino-api2
- Install the .deb files
- Clone this repo into a ros2 workspace
- Install `teleop_twist_keyboard`: `sudo apt-get install ros-${ROS_DISTRO}-teleop-twist-keyboard`
- Do `colcon build`
- Source the workspace

## Running the robot

### 1. Bring up the robot

```bash
ros2 launch rto_bringup rto_bringup.launch hostname:=<robotino_ip> [ns:=rto3]
```

| Argument | Default | Description |
|---|---|---|
| `hostname` | `172.26.1.1` | Robotino IP address |
| `ns` | `rto3` | ROS namespace for the robot nodes |

### 2. Launch keyboard teleop

In a separate terminal:

```bash
ros2 launch rto_bringup rto_teleop.launch [ns:=rto3]
```

The `ns` argument must match the namespace used in the bringup launch. `teleop_twist_keyboard` will open in the same terminal — use the standard keys to drive:

| Key | Action |
|---|---|
| `i` / `,` | Forward / Backward |
| `j` / `l` | Rotate left / right |
| `u` / `o` | Strafe diagonal |
| `k` | Stop |
| `q` / `z` | Increase / decrease max speed |
| `w` / `x` | Increase / decrease linear speed only |
| `e` / `c` | Increase / decrease angular speed only |

### 3. Launch RViz visualisation

```bash
ros2 launch rto_bringup rto_rviz.launch
```

## Velocity limits

Default velocity limits configured in `rto_node`:

| Parameter | Default | Description |
|---|---|---|
| `max_linear_vel` | `2.3` m/s | Maximum linear speed (x and y axes) |
| `min_linear_vel` | `0.02` m/s | Minimum non-zero linear speed (deadband) |
| `max_angular_vel` | `1.0` rad/s | Maximum angular (rotation) speed |
| `min_angular_vel` | `0.1` rad/s | Minimum non-zero angular speed (deadband) |

These can be overridden at launch:

```bash
ros2 launch rto_bringup rto_bringup.launch max_linear_vel:=1.0 max_angular_vel:=0.5
```

## TODO::
<!-- (ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args --remap cmd_vel:=/rto3/cmd_vel) -->

currently needed to operate teleop twist properly, also need to parse/find speed limits to match the limits of the RTO defined in the cpp driver