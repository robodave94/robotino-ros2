/*
 * RTONode.h
 *
 *  Created on: 09.12.2011
 *      Author: indorewala@servicerobotics.eu
 */

#ifndef ROBOTINOODOMETRYNODE_H_
#define ROBOTINOODOMETRYNODE_H_

#include "ComROS.h"
#include "OdometryROS.h"

#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_msgs/msg/tf_message.hpp"
#include "rto4_msgs/srv/reset_odometry_to_frame.hpp"

#include <string>
#include <set>
#include <memory>

class RTOOdometryNode: public rclcpp::Node
{
public:
	RTOOdometryNode();
	~RTOOdometryNode();


private:
	void spin();

	rclcpp::TimerBase::SharedPtr timer_;
	std::string hostname_;
	std::string frame_prefix_;

	ComROS com_;
	OdometryROS odometry_;

	// TF2 for reset-to-frame service
	std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
	std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

	// Track static frames from /tf_static
	std::set<std::string> static_frames_;
	rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr tf_static_sub_;
	void tfStaticCallback(const tf2_msgs::msg::TFMessage::SharedPtr msg);

	// Reset odometry to world-anchored frame service
	rclcpp::Service<rto4_msgs::srv::ResetOdometryToFrame>::SharedPtr reset_to_frame_server_;
	bool resetOdometryToFrameCallback(
		rto4_msgs::srv::ResetOdometryToFrame::Request::SharedPtr req,
		rto4_msgs::srv::ResetOdometryToFrame::Response::SharedPtr res);

	bool isStaticWorldFrame(const std::string& frame, std::string& error_msg);

	void initModules();
};

#endif /* ROBOTINOODOMETRYNODE_H_ */
