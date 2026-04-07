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

class GyroscopeROS : public rec::robotino::api2::GyroscopeExt
{
public:
	GyroscopeROS(rclcpp::Node* parent_node);
	~GyroscopeROS();

	void setTimeStamp(rclcpp::Time stamp);

private:
	rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;

	sensor_msgs::msg::Imu imu_msg_;

	rclcpp::Time stamp_;

	void gyroscopeExtEvent(float angle, float rate) override;
};

#endif /* GYROSCOPEROS_H_ */
