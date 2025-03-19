#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <cmath>
#include <vector>
#include <cstdlib>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "ros360_cv_msgs/msg/colour_points.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "rcl_interfaces/srv/set_parameters.hpp"
#include "rcl_interfaces/msg/parameter.hpp"


using namespace std::chrono_literals;
using std::placeholders::_1;

class VisualPerception : public rclcpp::Node
{
public:
    VisualPerception() : Node("visual_perception"), image_center(0), object_detected(false)
    {
        // Publisher for robot velocity commands
        velocity_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/a200_0000/cmd_vel", 10);

         subscription_laser_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
       "/a200_0000/sensors/lidar2d_0/scan", 10, std::bind(&VisualPerception::laser_scanner_callback, this, _1));

        // Subscriber for color detection points
        colour_sub_ = this->create_subscription<ros360_cv_msgs::msg::ColourPoints>(
            "/colour_detection", 10,
            std::bind(&VisualPerception::colourPointsCallback, this, _1));

        // Subscriber for camera information
        camera_info_sub_ = this->create_subscription<sensor_msgs::msg::CameraInfo>(
            "/a200_0000/sensors/camera_0/color/camera_info", 10,
            std::bind(&VisualPerception::cameraInfoCallback, this, _1));

        // Timer for periodic updates
        timer_ = this->create_wall_timer(
            100ms, std::bind(&VisualPerception::timerCallback, this));


    }

 void send_request()
    {
      auto parameter = rcl_interfaces::msg::Parameter();
      auto request = std::make_shared<rcl_interfaces::srv::SetParameters::Request>();

      auto client = this->create_client<rcl_interfaces::srv::SetParameters>("/colour_detector/set_parameters");

      parameter.name = "lower_limits";
      parameter.value.type = 7; // specified integer array value 
      parameter.value.integer_array_value = std::vector<long int>{100,0,0};

      request->parameters.push_back(parameter);

      parameter.name = "upper_limits";
      parameter.value.type = 7; // specified integer array value 
      parameter.value.integer_array_value = std::vector<long int>{255,0,0};

      request->parameters.push_back(parameter);

      while (!client->wait_for_service(1s)) {
          if (!rclcpp::ok()) {
          RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
          return;
          }
          RCLCPP_INFO_STREAM(this->get_logger(), "service not available, waiting again..."); 
      }
      auto future = client->async_send_request(request);
      
      usleep(500);
       // Wait for the result.
      // if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) ==
      //   rclcpp::FutureReturnCode::SUCCESS)
      // {
      //   auto response = future.get();
      //   for(auto result : response->results)
      //   {
      //     RCLCPP_INFO(this->get_logger(), "Result of colour params update: %d", result.successful);
      //   }
        
      // } else {
      //   RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to call service /colour_detector/set_parameters");
      // }
    }
    

private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr velocity_pub_;
    rclcpp::Subscription<ros360_cv_msgs::msg::ColourPoints>::SharedPtr colour_sub_;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_laser_;

    double image_center;
    bool object_detected;
    float safe_dist = 0.2;
    float radius;
    int image_width = 640;
    int image_height = 430;
    float x;
    float y;
    float left_laser;
    float right_laser;
    float front_laser;
    float velocity;
    ros360_cv_msgs::msg::ColourPoints current_object;

     // Threshold for detecting red color (simple approximation)
    const int red_threshold = 100;  // Red component threshold
    const int green_threshold = 0;  // Green component threshold
    const int blue_threshold = 0;  // Blue component threshold



    void laser_scanner_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
right_laser = msg->ranges[180];
front_laser = msg->ranges[360];
left_laser = msg->ranges[540];
}

    void cameraInfoCallback(const sensor_msgs::msg::CameraInfo::SharedPtr msg)
    {
        // Compute the center of the image
        image_center = msg->width / 2.0;
    }

   
    void colourPointsCallback(const ros360_cv_msgs::msg::ColourPoints &msg)
    {
     radius = msg.points[0].radius;
     x = msg.points[0].x;
     y = msg.points[0].y;
        
    }

void timerCallback()
{
    auto twist_msg = geometry_msgs::msg::Twist();
        velocity = (x - 320) / 320;
int state = 1;
switch(state) {
case 1: 
       if(x > 0) {
            twist_msg.angular.z = velocity;
            RCLCPP_INFO(this->get_logger(), "Object reached, stopping.");
            state = 2;
       }
case 2: 
        //Rotate to search for an object
        twist_msg.angular.z = 0.3;
       RCLCPP_INFO(this->get_logger(), "Searching for object, Angular Z: %f", twist_msg.angular.z);
       
  

    // Publish the velocity command
    velocity_pub_->publish(twist_msg);
}
}

};

// Main entry point
int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
  //auto colour_client = std::make_shared<ColourClient>();
    VisualPerception visual_perception = VisualPerception();
    visual_perception.send_request();
    rclcpp::spin(std::make_shared<VisualPerception>());
    rclcpp::shutdown();
    return 0;
}