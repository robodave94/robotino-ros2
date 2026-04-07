/*
 * KeyboardTeleopRTO.h
 *
 * ROS2 keyboard teleop node for Robotino3.
 * Mirrors teleop_twist_keyboard key bindings exactly, targeting /rto3/cmd_vel
 * by default (no remapping required) and enforcing the speed bounds defined
 * in RTONode (max_linear_vel, min_linear_vel, max_angular_vel, min_angular_vel).
 */

#ifndef KEYBOARDTELEOPrto4_H_
#define KEYBOARDTELEOPrto4_H_

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

#include <termios.h>
#include <map>
#include <tuple>
#include <string>

class KeyboardTeleopRTO : public rclcpp::Node
{
public:
    KeyboardTeleopRTO();
    ~KeyboardTeleopRTO();

    // Blocking keyboard loop — call from main()
    void run();

private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;

    // Current speed magnitudes (adjusted by q/z/w/x/e/c)
    double speed_;
    double turn_;

    // Velocity bounds from RTONode parameters
    double max_linear_vel_;
    double min_linear_vel_;
    double max_angular_vel_;
    double min_angular_vel_;

    // Terminal state
    struct termios cooked_;
    struct termios raw_;
    int kfd_;

    void readParams();
    void clampSpeeds();
    void publishVelocity(double x, double y, double z, double th);
    void restoreTerminal();
    void printUsage() const;

    // move bindings: key -> {x, y, z, th}
    using MoveBinding = std::tuple<double, double, double, double>;
    std::map<char, MoveBinding> move_bindings_;

    // speed bindings: key -> {linear_factor, angular_factor}
    using SpeedBinding = std::tuple<double, double>;
    std::map<char, SpeedBinding> speed_bindings_;
};

#endif /* KEYBOARDTELEOPrto4_H_ */
