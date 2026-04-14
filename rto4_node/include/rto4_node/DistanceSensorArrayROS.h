/*
 * DistanceSensorArrayROS.h
 *
 *  Created on: 07.12.2011
 *      Author: indorewala@servicerobotics.eu
 */

#ifndef DISTANCESENSORARRAYROS_H_
#define DISTANCESENSORARRAYROS_H_

#include "rec/robotino/api2/DistanceSensorArray.h"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud.hpp"

#include <string>

class DistanceSensorArrayROS: public rec::robotino::api2::DistanceSensorArray
{
public:
	DistanceSensorArrayROS(rclcpp::Node* parent_node, const std::string& frame_prefix = "");
	~DistanceSensorArrayROS();

	void setTimeStamp(rclcpp::Time stamp);
	void setFramePrefix(const std::string& fp) { frame_prefix_ = fp; }

private:

	rclcpp::Publisher<sensor_msgs::msg::PointCloud>::SharedPtr distances_pub_;

	sensor_msgs::msg::PointCloud distances_msg_;

	std::string frame_prefix_;
	rclcpp::Time stamp_;

	void distancesChangedEvent(const float* distances, unsigned int size);

};


#endif /* DISTANCESENSORARRAYROS_H_ */
