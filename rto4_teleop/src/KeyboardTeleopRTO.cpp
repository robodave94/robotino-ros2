/*
 * KeyboardTeleopRTO.cpp
 *
 * ROS2 keyboard teleop node for Robotino3.
 * Key bindings match teleop_twist_keyboard exactly.
 * Default namespace "rto3" makes this publish to /rto3/cmd_vel without remapping.
 * Speed is clamped to the same bounds used in RTONode / OmniDriveROS.
 */

#include "KeyboardTeleopRTO.h"

#include <signal.h>
#include <stdio.h>
#include <cmath>
#include <stdexcept>
#include <algorithm>

// ---------------------------------------------------------------------------
// Signal handler — restores terminal on Ctrl+C
// ---------------------------------------------------------------------------
static struct termios g_cooked;
static int g_kfd = 0;

static void sigintHandler(int /*sig*/)
{
    tcsetattr(g_kfd, TCSANOW, &g_cooked);
    rclcpp::shutdown();
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
KeyboardTeleopRTO::KeyboardTeleopRTO()
: rclcpp::Node("keyboard"),
  speed_(1.0),
  turn_(0.5),
  kfd_(0)
{
    // Namespace is set externally via --ros-args -r __ns:=/robot1 or launch namespace=
    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 1);

    // Declare the same speed-limit parameters as RTONode so they can be
    // overridden at runtime via --ros-args -p max_linear_vel:=X etc.
    this->declare_parameter("max_linear_vel",  2.3);
    this->declare_parameter("min_linear_vel",  0.02);
    this->declare_parameter("max_angular_vel", 1.0);
    this->declare_parameter("min_angular_vel", 0.1);
    readParams();

    // ------------------------------------------------------------------
    // Move bindings — exactly matching teleop_twist_keyboard
    // tuple fields: { x, y, z, th }
    // ------------------------------------------------------------------
    move_bindings_ = {
        // Non-holonomic directions
        { 'i', { 1.0,  0.0,  0.0,  0.0} },
        { 'o', { 1.0,  0.0,  0.0, -1.0} },
        { 'u', { 1.0,  0.0,  0.0,  1.0} },
        { 'j', { 0.0,  0.0,  0.0,  1.0} },
        { 'l', { 0.0,  0.0,  0.0, -1.0} },
        { ',', {-1.0,  0.0,  0.0,  0.0} },
        { '.', {-1.0,  0.0,  0.0,  1.0} },
        { 'm', {-1.0,  0.0,  0.0, -1.0} },
        // Vertical (linear.z)
        { 't', { 0.0,  0.0,  1.0,  0.0} },
        { 'b', { 0.0,  0.0, -1.0,  0.0} },
        // Holonomic strafing (Shift variants) — linear.y
        { 'I', { 1.0,  0.0,  0.0,  0.0} },
        { 'O', { 1.0, -1.0,  0.0,  0.0} },
        { 'U', { 1.0,  1.0,  0.0,  0.0} },
        { 'J', { 0.0,  1.0,  0.0,  0.0} },
        { 'L', { 0.0, -1.0,  0.0,  0.0} },
        { 'M', {-1.0,  1.0,  0.0,  0.0} },
        { '<', {-1.0,  0.0,  0.0,  0.0} },
        { '>', {-1.0, -1.0,  0.0,  0.0} },
    };

    // ------------------------------------------------------------------
    // Speed bindings — exactly matching teleop_twist_keyboard
    // tuple fields: { linear_factor, angular_factor }
    // ------------------------------------------------------------------
    speed_bindings_ = {
        { 'q', {1.1, 1.1} },
        { 'z', {0.9, 0.9} },
        { 'w', {1.1, 1.0} },
        { 'x', {0.9, 1.0} },
        { 'e', {1.0, 1.1} },
        { 'c', {1.0, 0.9} },
    };
}

KeyboardTeleopRTO::~KeyboardTeleopRTO()
{
    restoreTerminal();
}

// ---------------------------------------------------------------------------
// readParams — load velocity bounds from node parameters
// ---------------------------------------------------------------------------
void KeyboardTeleopRTO::readParams()
{
    max_linear_vel_  = this->get_parameter("max_linear_vel").as_double();
    min_linear_vel_  = this->get_parameter("min_linear_vel").as_double();
    max_angular_vel_ = this->get_parameter("max_angular_vel").as_double();
    min_angular_vel_ = this->get_parameter("min_angular_vel").as_double();
}

// ---------------------------------------------------------------------------
// clampSpeeds — enforce RTONode bounds on current speed / turn values
// ---------------------------------------------------------------------------
void KeyboardTeleopRTO::clampSpeeds()
{
    speed_ = std::max(min_linear_vel_,  std::min(max_linear_vel_,  speed_));
    turn_  = std::max(min_angular_vel_, std::min(max_angular_vel_, turn_));
}

// ---------------------------------------------------------------------------
// publishVelocity — build and publish a Twist from direction + current speeds
// ---------------------------------------------------------------------------
void KeyboardTeleopRTO::publishVelocity(double x, double y, double z, double th)
{
    geometry_msgs::msg::Twist msg;
    msg.linear.x  = x  * speed_;
    msg.linear.y  = y  * speed_;
    msg.linear.z  = z  * speed_;
    msg.angular.z = th * turn_;
    cmd_vel_pub_->publish(msg);
}

// ---------------------------------------------------------------------------
// restoreTerminal — restore cooked terminal settings
// ---------------------------------------------------------------------------
void KeyboardTeleopRTO::restoreTerminal()
{
    if (kfd_ != 0) {
        tcsetattr(kfd_, TCSANOW, &g_cooked);
    }
}

// ---------------------------------------------------------------------------
// printUsage — matches teleop_twist_keyboard banner format
// ---------------------------------------------------------------------------
void KeyboardTeleopRTO::printUsage() const
{
    printf("\nReading from the keyboard and publishing to %s/cmd_vel\n",
           this->get_namespace());
    printf("---------------------------\n");
    printf("Moving around:\n");
    printf("   u    i    o\n");
    printf("   j    k    l\n");
    printf("   m    ,    .\n");
    printf("\nFor Holonomic mode (strafing), hold down the shift key:\n");
    printf("---------------------------\n");
    printf("   U    I    O\n");
    printf("   J    K    L\n");
    printf("   M    <    >\n");
    printf("\nt : up (+z)\nb : down (-z)\n");
    printf("\nAnything else : stop\n");
    printf("\nq/z : increase/decrease max speeds by 10%%\n");
    printf("w/x : increase/decrease only linear speed by 10%%\n");
    printf("e/c : increase/decrease only angular speed by 10%%\n");
    printf("\nCTRL-C to quit\n");
    printf("\ncurrently:\tspeed %.4f\tturn %.4f\n\n", speed_, turn_);
}

// ---------------------------------------------------------------------------
// run — blocking keyboard loop (call from main)
// ---------------------------------------------------------------------------
void KeyboardTeleopRTO::run()
{
    char c;

    // Switch terminal to raw mode
    kfd_ = fileno(stdin);
    tcgetattr(kfd_, &cooked_);
    g_cooked = cooked_;
    g_kfd    = kfd_;

    memcpy(&raw_, &cooked_, sizeof(struct termios));
    raw_.c_lflag &= ~(static_cast<tcflag_t>(ICANON) | static_cast<tcflag_t>(ECHO));
    raw_.c_cc[VEOL] = 1;
    raw_.c_cc[VEOF] = 2;
    tcsetattr(kfd_, TCSANOW, &raw_);

    signal(SIGINT, sigintHandler);

    printUsage();

    while (rclcpp::ok()) {
        // Get the next character from the keyboard
        if (read(kfd_, &c, 1) < 0) {
            perror("read()");
            break;
        }

        auto move_it  = move_bindings_.find(c);
        auto speed_it = speed_bindings_.find(c);

        if (move_it != move_bindings_.end()) {
            auto [x, y, z, th] = move_it->second;
            publishVelocity(x, y, z, th);
        } else if (speed_it != speed_bindings_.end()) {
            auto [lf, af] = speed_it->second;
            speed_ *= lf;
            turn_  *= af;
            clampSpeeds();
            printf("\rcurrently:\tspeed %.4f\tturn %.4f   ", speed_, turn_);
            fflush(stdout);
            publishVelocity(0.0, 0.0, 0.0, 0.0);
        } else {
            // k or any unknown key — stop
            publishVelocity(0.0, 0.0, 0.0, 0.0);
        }

        rclcpp::spin_some(this->shared_from_this());
    }

    restoreTerminal();
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyboardTeleopRTO>();
    node->run();
    rclcpp::shutdown();
    return 0;
}
