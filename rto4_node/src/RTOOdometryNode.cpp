/*
 * RTONode.cpp
 *
 *  Created on: 09.12.2011
 *      Author: indorewala@servicerobotics.eu
 */

#include "RTOOdometryNode.h"
#include "tf2/utils.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;
using std::placeholders::_2;

RTOOdometryNode::RTOOdometryNode():
Node( "rto4_odometry_node"),
	com_(this),
	odometry_(this)
{
	this->declare_parameter("hostname", "172.26.1.1");
	this->declare_parameter("frame_prefix", "");
	hostname_ = this->get_parameter("hostname").as_string();

	std::string fp = this->get_parameter("frame_prefix").as_string();
	frame_prefix_ = fp.empty() ? "" : fp + "/";

	// Pass frame prefix to odometry publisher
	odometry_.setFramePrefix(frame_prefix_);

	// TF2 buffer and listener for reset-to-frame lookups
	tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
	tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

	// Subscribe to /tf_static to track known static frames
	tf_static_sub_ = this->create_subscription<tf2_msgs::msg::TFMessage>(
		"/tf_static", rclcpp::QoS(100).transient_local(),
		std::bind(&RTOOdometryNode::tfStaticCallback, this, _1));

	// Reset odometry to world-anchored frame service
	reset_to_frame_server_ = this->create_service<rto4_msgs::srv::ResetOdometryToFrame>(
		"reset_odometry_to_frame",
		std::bind(&RTOOdometryNode::resetOdometryToFrameCallback, this, _1, _2));

	com_.setName( "Odometry" );

	initModules();
	timer_ = this->create_wall_timer(200ms,std::bind(&RTOOdometryNode::spin, this));
}

RTOOdometryNode::~RTOOdometryNode()
{
}

void RTOOdometryNode::initModules()
{
	com_.setAddress( hostname_.c_str() );

	// Set the ComIds
	odometry_.setComId( com_.id() );

	com_.connectToServer( false );
}

void RTOOdometryNode::spin()
{
	rclcpp::Time curr_time = rclcpp::Clock().now();
	odometry_.setTimeStamp(curr_time);

	com_.processEvents();
}

void RTOOdometryNode::tfStaticCallback(const tf2_msgs::msg::TFMessage::SharedPtr msg)
{
	for (const auto& transform : msg->transforms) {
		static_frames_.insert(transform.child_frame_id);
		// Also record the parent — it's a known frame in the static tree
		static_frames_.insert(transform.header.frame_id);
	}
}

bool RTOOdometryNode::isStaticWorldFrame(const std::string& frame, std::string& error_msg)
{
	// Check the frame is known in the static TF tree
	if (static_frames_.find(frame) == static_frames_.end()) {
		// Could be a dynamic frame or unknown entirely
		if (tf_buffer_->canTransform(frame_prefix_ + "odom", frame,
			tf2::TimePointZero, tf2::durationFromSec(0.1))) {
			error_msg = "Frame '" + frame + "' is a dynamic frame; "
				"only static world-anchored frames are allowed";
		} else {
			error_msg = "Frame '" + frame + "' not found in TF tree";
		}
		return false;
	}

	// Walk up the TF static tree to verify it's anchored to world or map
	// (not to an odom or robot-specific frame)
	std::string current = frame;
	std::set<std::string> visited;
	while (true) {
		if (current == "world" || current == "map") {
			return true;  // Found a valid world anchor
		}
		if (visited.count(current)) {
			break;  // Cycle detected
		}
		visited.insert(current);

		// Find the parent of current in the static tree
		bool found_parent = false;
		std::string parent;
		try {
			found_parent = tf_buffer_->_getParent(current, tf2::TimePointZero, parent);
		} catch (...) {
			// No parent found
		}

		if (!found_parent) {
			break;  // Reached a root that's not world/map
		}
		current = parent;
	}

	error_msg = "Frame '" + frame + "' is not a child of 'world' or 'map'";
	return false;
}

bool RTOOdometryNode::resetOdometryToFrameCallback(
	rto4_msgs::srv::ResetOdometryToFrame::Request::SharedPtr req,
	rto4_msgs::srv::ResetOdometryToFrame::Response::SharedPtr res)
{
	std::string error_msg;
	if (!isStaticWorldFrame(req->reference_frame, error_msg)) {
		res->success = false;
		res->message = error_msg;
		RCLCPP_WARN(this->get_logger(), "ResetOdometryToFrame rejected: %s", error_msg.c_str());
		return true;
	}

	// Look up transform from reference_frame to our odom frame
	std::string odom_frame = frame_prefix_ + "odom";
	geometry_msgs::msg::TransformStamped transform;
	try {
		transform = tf_buffer_->lookupTransform(
			odom_frame, req->reference_frame,
			tf2::TimePointZero, tf2::durationFromSec(1.0));
	} catch (const tf2::TransformException& ex) {
		res->success = false;
		res->message = std::string("Transform lookup failed: ") + ex.what();
		RCLCPP_WARN(this->get_logger(), "ResetOdometryToFrame transform failed: %s", ex.what());
		return true;
	}

	// Transform the requested pose into the odom frame
	geometry_msgs::msg::PoseStamped pose_in;
	pose_in.header.frame_id = req->reference_frame;
	pose_in.header.stamp = this->now();
	pose_in.pose = req->pose;

	geometry_msgs::msg::PoseStamped pose_odom;
	try {
		tf2::doTransform(pose_in, pose_odom, transform);
	} catch (const tf2::TransformException& ex) {
		res->success = false;
		res->message = std::string("Pose transform failed: ") + ex.what();
		return true;
	}

	// Extract x, y, yaw from the transformed pose
	double x = pose_odom.pose.position.x;
	double y = pose_odom.pose.position.y;
	double yaw = tf2::getYaw(pose_odom.pose.orientation);

	// Reset hardware odometry
	odometry_.set(x, y, yaw, true);

	res->success = true;
	res->message = "Odometry reset to (" + std::to_string(x) + ", " +
		std::to_string(y) + ", " + std::to_string(yaw) +
		") in " + odom_frame + " frame (from " + req->reference_frame + ")";
	RCLCPP_INFO(this->get_logger(), "%s", res->message.c_str());
	return true;
}
