#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <cmath>
#include <vector>
#include <cstdlib>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int32.hpp" 
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rcl_interfaces/srv/set_parameters.hpp"
#include "rcl_interfaces/msg/parameter.hpp"
#include "ros360_cv_msgs/msg/colour_points.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "cv_bridge/cv_bridge.h"
#include <opencv2/opencv.hpp>
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/exceptions.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"

using namespace std::chrono_literals;
using std::placeholders::_1;

static const std::string OPENCV_WINDOW = "Depth Perception";;
class Assignment24 : public rclcpp::Node
{
float left_laser;
float right_laser;
float front_laser;
float depth = 0.0;

public:
 Assignment24() : Node("assignment_24")
 {
  // Initializisation of publishers and subscribers 
  tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

  tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

  velocity_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/a200_0000/cmd_vel", 10);

  subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
   "/a200_0000/sensors/lidar2d_0/scan", 10, std::bind(&Assignment24::laser_scanner_callback, this, _1));


  // Subscriber for color detection points
  colour_sub_ = this->create_subscription<ros360_cv_msgs::msg::ColourPoints>(
    "/colour_detection", 10,std::bind(&Assignment24::colourPointsCallback, this, _1));

  // Create subscription to depth image
  img_subscription = this->create_subscription<sensor_msgs::msg::Image>(
    "/a200_0000/sensors/camera_0/depth/image",10, std::bind(&Assignment24::depth_image_callback, this, _1));


  timer_ = this->create_wall_timer(300ms, std::bind(&Assignment24::movement_callback, this));
 }
void send_request(const std::vector<long int> lower_limits, const std::vector<long int> upper_limits)
{
   auto parameter = rcl_interfaces::msg::Parameter();
   auto request = std::make_shared<rcl_interfaces::srv::SetParameters::Request>();

   // Create lower_limits parameter
   parameter.name = "lower_limits";
   parameter.value.type = 7; // Integer array type
   parameter.value.integer_array_value = lower_limits;
   request->parameters.push_back(parameter);
   
   // Create upper_limits parameter
   parameter.name = "upper_limits";
   parameter.value.type = 7; // Integer array type
   parameter.value.integer_array_value = upper_limits;
   request->parameters.push_back(parameter);

   // Create client and wait for service to be available
   auto client = this->create_client<rcl_interfaces::srv::SetParameters>("/colour_detector/set_parameters");

   // Wait for the service to become available
   while (!client->wait_for_service(1s)) {
       if (!rclcpp::ok()) {
           RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
           return;
       }
       RCLCPP_INFO_STREAM(this->get_logger(), "Service not available, waiting again...");
   }

    // Send request and handle future response
    client->async_send_request(request);

//    // Wait for the result
//    if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) == rclcpp::FutureReturnCode::SUCCESS) {
//        auto response = future.get();
//        for (auto result : response->results) {
//            RCLCPP_INFO(this->get_logger(), "Result of colour params update: %d", result.successful);
//        }
//    } else {
//        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to call service /colour_detector/set_parameters");
//    }
}

private:
//Subscriptions and Publishers
   rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
   rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr velocity_pub_;
   rclcpp::Subscription<ros360_cv_msgs::msg::ColourPoints>::SharedPtr colour_sub_;
   rclcpp::TimerBase::SharedPtr timer_;
   rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_laser_;
   rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_subscription;
  
   // tf2 objects for transform broadcasting and listening
   std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
   std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
   std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

   double image_center;
   bool object_detected = false;
   float radius = 0.0;
   int image_width = 640;
   int image_height = 430;
   float x;
   float y;
   float velocity;
   float turning_rate = 0.5;
   bool stop_robot;
   bool spin_robot;
   bool read_depth = false;
   int state = 1;


void laser_scanner_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{

   right_laser = msg->ranges[180];
   front_laser = msg->ranges[360];
   left_laser = msg->ranges[540];

  //RCLCPP_INFO(this->get_logger(), "The object right is %f, in distance %f ", right_laser, msg->ranges[180]);
  // RCLCPP_INFO(this->get_logger(), "The object front is %f, in distance %f ", front_laser,  msg->ranges[360]);
  // RCLCPP_INFO(this->get_logger(), "The object left is %f, in distance %f ", left_laser, msg->ranges[540]);
}

void depth_image_callback(const sensor_msgs::msg::Image & imgMsg)
{

    if(read_depth) {
 // Convert ROS Image to CV Image
 cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(imgMsg,
 sensor_msgs::image_encodings::TYPE_32FC1);
 geometry_msgs::msg::TransformStamped t;
 // Extract the depth at the coordinate from colour detection
 float depth = cv_ptr->image.at<float>(y,x);
 
rclcpp::Time timestamp = rclcpp::Time(imgMsg.header.stamp);

t.header.stamp = timestamp;
t.header.frame_id = "camera_0_depth_frame";
t.child_frame_id = "target_object";
// In this coordinate frame, x is forward, y is right and z is up
float focal_length = 608.6419;

float x_offset = -(((x - image_width / 2) * depth) / focal_length);
float y_offset = -(((y - image_height / 2) * depth) / focal_length);
// hence the x and y offsets being applied to the y and z coordinates.
// Additional offsets could be added here later for grasping
t.transform.translation.x = depth;
t.transform.translation.y = x_offset;
t.transform.translation.z = y_offset;
   RCLCPP_INFO(this->get_logger(), "Depth: %f", depth);
// We need to supply a rotation, but currently don't care so set it to 0 in Euler coordinates.
// A function is then used to generate the quaternion for us
tf2::Quaternion q;
q.setRPY(0, 0, 0);
t.transform.rotation.x = q.x();
t.transform.rotation.y = q.y();
t.transform.rotation.z = q.z();
t.transform.rotation.w = q.w();


// Send the transformation
tf_broadcaster_->sendTransform(t);
 cv::cvtColor(cv_ptr->image, cv_ptr->image, cv::COLOR_GRAY2RGB);
 cv::circle(cv_ptr->image, cv::Point(x, y), 10, CV_RGB(0,0,255), 2);

//Later on
std::string fromFrame = "odom";
std::string toFrame = "target_object";
if (tf_buffer_->canTransform(fromFrame, toFrame, tf2::TimePointZero)) {
    try {
        t = tf_buffer_->lookupTransform(fromFrame, toFrame, tf2::TimePointZero);
        // Process the transform
    } catch (const tf2::TransformException & ex) {
        RCLCPP_ERROR(this->get_logger(), "Error looking up transform: %s", ex.what());
    }
} else {
    RCLCPP_WARN(this->get_logger(), "Transform from %s to %s not available", fromFrame.c_str(), toFrame.c_str());
}

// Look up for the transformation between target frames
try {

    
t = tf_buffer_->lookupTransform(
fromFrame, toFrame,
tf2::TimePointZero);
RCLCPP_INFO_STREAM(this->get_logger(), "Transform obtained transltion:\n\t"
<< t.transform.translation.x << ", "
<< t.transform.translation.y << ", "
<< t.transform.translation.z << ", "
<< "\nRotation:\n\t"
<< t.transform.rotation.x << ", "
<< t.transform.rotation.y << ", "
<< t.transform.rotation.z << ", "
<< t.transform.rotation.w << ", "
);
} catch (const tf2::TransformException & ex) {
RCLCPP_INFO(this->get_logger(), "Could not transform %s to %s: %s",
toFrame.c_str(), fromFrame.c_str(), ex.what());
//return;
}

 // Update GUI Window
 cv::imshow(OPENCV_WINDOW, cv_ptr->image);
 cv::waitKey(3);
    }
 }


void movement_callback()
{
    auto twist_msg = geometry_msgs::msg::Twist();

    switch (state) {
        case 1: {
            // Move robot forward based on laser readings
            float safe_dist = 4.0;
            float max_speed = 1.9;
            float avg_side = 0.4;

            twist_msg.angular.z = ((left_laser - right_laser) / avg_side) * turning_rate;
            twist_msg.linear.x = (front_laser / safe_dist) * max_speed;


            // Transition to state 2 if radius is greater than 0 (assuming object is found)
            if (radius > 0) {
                state = 2;
            }
            break;
        }
        case 2: {
                      // Try to align with red square
           RCLCPP_INFO(this->get_logger(), "Detected object at x: %f, radius: %f.", x, radius);

           // Center the object on the x-axis of the image
           if (x > 310 && x < 325) {
               // Object centered
               RCLCPP_INFO(this->get_logger(), "Object is centered, stopping robot.");
               twist_msg.angular.z = 0.0;
               twist_msg.linear.x = 0.3;
               object_detected = true;  // Flag object as detected
        
             // Transition to stop state
                   
           } else {
               // Object not centered, turn towards it
               velocity = (x - 320) / 320;
                    RCLCPP_INFO(this->get_logger(), "Adjusting");
               twist_msg.angular.z = velocity;
               twist_msg.linear.x = 1.0;  // Move forward slowly
            
           }

           if (object_detected && radius < 50.0) {
                   state = 3;
           }
           break;
             }
    
        case 3: {
            // Object reached, stop robot
            RCLCPP_INFO(this->get_logger(), "Object reached, stopping robot.");
            twist_msg.angular.z = 0.0;
            twist_msg.linear.x = 0.0;
                  state = 4; 
        
             // Transition to spin state
            break;
        }
        case 4: {
            
            // Spin robot in place
            RCLCPP_INFO(this->get_logger(), "Spinning.");
             turning_rate = 0.005;
            twist_msg.angular.z = (x - 320) * turning_rate;
            twist_msg.linear.x = 0.0;
            
                std::vector<long int> lower_limits = {100, 0, 0};  
    std::vector<long int> upper_limits = {255, 0, 0};
            send_request(lower_limits, upper_limits);


             if (x > 310 && x < 325) {
              RCLCPP_INFO(this->get_logger(), "Blue object detected and centered, stopping robot.");
                twist_msg.angular.z = 0.0;
                state = 5;
                
        }
            break;
        }
        case 5: {
            read_depth = true;
                   RCLCPP_INFO(this->get_logger(), "read");

        }
    }
  velocity_pub_->publish(twist_msg);
    }

 //  RCLCPP_INFO(this->get_logger(), "Publishing: Linear Velocity %f, Angular Velocity=%f" ,   message.linear.x, message.angular.z);


void colourPointsCallback(const ros360_cv_msgs::msg::ColourPoints msg)
{
    radius = msg.points[0].radius;
    x = msg.points[0].x;
    y = msg.points[0].y;
}
};

int main(int argc, char * argv[])
{
   // Initialize ROS 2 for this node
   rclcpp::init(argc, argv);
   
   // Create the node and start spinning
   auto node = std::make_shared<Assignment24>();
   
   // Optionally, send the request after node creation if necessary
   std::vector<long int> lower_limits = {0, 0, 255};  
   std::vector<long int> upper_limits = {203, 192, 255};
   node->send_request(lower_limits, upper_limits);
   
   rclcpp::spin(node);  // Spin to process callbacks
   
   // Shutdown ROS 2
   rclcpp::shutdown();
   return 0;
}
