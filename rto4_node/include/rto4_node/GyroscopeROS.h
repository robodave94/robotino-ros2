/*
 * GyroscopeROS.h
 *
 *  Gyroscope sensor wrapper for Festo Robotino4.
 *  Publishes sensor_msgs/Imu on the "imu" topic.
 */

#ifndef GYROSCOPEROS_H_
#define GYROSCOPEROS_H_

#include "rec/robotino/api2/GyroscopeExt.h"

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

#include <string>

class GyroscopeROS : public rec::robotino::api2::GyroscopeExt
{
public:
	GyroscopeROS(rclcpp::Node* parent_node, const std::string& frame_prefix = "");
	~GyroscopeROS();

	void setTimeStamp(rclcpp::Time stamp);
	void setFramePrefix(const std::string& fp) { frame_prefix_ = fp; }

private:
	rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;

	sensor_msgs::msg::Imu imu_msg_;

	std::string frame_prefix_;
	rclcpp::Time stamp_;

	void gyroscopeExtEvent(float angle, float rate) override;
};

#endif /* GYROSCOPEROS_H_ */
