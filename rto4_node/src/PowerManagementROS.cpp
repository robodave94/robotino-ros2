/*
 * PowerManagementROS.cpp
 *
 *  Created on: 07.12.2011
 *      Author: indorewala@servicerobotics.eu
 */

#include "PowerManagementROS.h"

PowerManagementROS::PowerManagementROS(rclcpp::Node* parent_node)
{
	power_pub_ = parent_node->create_publisher<rto4_msgs::msg::PowerReadings>("power_readings", 10);
}

PowerManagementROS::~PowerManagementROS()
{
}

void PowerManagementROS::setTimeStamp(rclcpp::Time stamp)
{
	stamp_ = stamp;
}

void PowerManagementROS::readingsEvent(float battery_voltage, float system_current,
	bool ext_power, int num_chargers, const char* batteryType,
	bool batteryLow, int batteryLowShutdownCounter)
{
	(void)ext_power;
	(void)num_chargers;
	(void)batteryType;
	(void)batteryLow;
	(void)batteryLowShutdownCounter;

	// Build the PowerReadings msg
	power_msg_.stamp = rclcpp::Clock().now();
	power_msg_.current = system_current;
	power_msg_.voltage = battery_voltage;

	// Publish the msg
	power_pub_->publish( power_msg_ );
}
