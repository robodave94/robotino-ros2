/*
 * GyroscopeROS.cpp
 *
 *  Gyroscope sensor wrapper for Festo Robotino4.
 *  Publishes sensor_msgs/Imu with angular velocity (rate) and
 *  orientation derived from the gyroscope angle.
 */

#include "GyroscopeROS.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

GyroscopeROS::GyroscopeROS(rclcpp::Node* parent_node)
{
	imu_pub_ = parent_node->create_publisher<sensor_msgs::msg::Imu>("imu", 10);

	// Pre-fill constant fields
	imu_msg_.header.frame_id = "base_link";

	// We only have yaw from gyroscope — mark linear accel as unknown
	imu_msg_.linear_acceleration_covariance[0] = -1.0;

	// Angular velocity: only z-axis is known
	imu_msg_.angular_velocity_covariance[0] = 0.0;
	imu_msg_.angular_velocity_covariance[4] = 0.0;
	imu_msg_.angular_velocity_covariance[8] = 0.01;

	// Orientation covariance: only yaw is known
	imu_msg_.orientation_covariance[0] = 0.0;
	imu_msg_.orientation_covariance[4] = 0.0;
	imu_msg_.orientation_covariance[8] = 0.01;
}

GyroscopeROS::~GyroscopeROS()
{
}

void GyroscopeROS::setTimeStamp(rclcpp::Time stamp)
{
	stamp_ = stamp;
}

void GyroscopeROS::gyroscopeExtEvent(float angle, float rate)
{
	imu_msg_.header.stamp = stamp_;

	// Convert yaw angle to quaternion
	tf2::Quaternion q;
	q.setRPY(0.0, 0.0, static_cast<double>(angle));
	imu_msg_.orientation = tf2::toMsg(q);

	// Angular velocity around z-axis
	imu_msg_.angular_velocity.x = 0.0;
	imu_msg_.angular_velocity.y = 0.0;
	imu_msg_.angular_velocity.z = static_cast<double>(rate);

	imu_pub_->publish(imu_msg_);
}
