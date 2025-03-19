// PHS: Template combining together publisher and subscriber examples

//General C++ includes as needed
#include <chrono>
#include <functional>
#include <memory>
#include <string>

//ROS 2 Specific includes
//Important: these includes should be reflected in the package.xml and CMakeLists.txt
#include "rclcpp/rclcpp.hpp"

// TODO: Add or replace with message type needed
#include "std_msgs/msg/u_int32.hpp" 
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;

/* Template giving outline for creating a program
 * that contains both a publisher and subscriber.
 */

class HuskyPubSub : public rclcpp::Node
{
unsigned int value;
float count;
float average;
float sum;
float left_laser;
float right_laser;
float front_laser;

public:
  //TODO: Update node name to reflect purpose of node
  HuskyPubSub() : Node("huksy_pub_sub")
  {
    // TODO: Update message type and topic name as appropriate for the publisher. Remember to update private field too.
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/a200_0000/cmd_vel", 10);

    // TODO: Update message type and topic name as appropriate for the subscriber. Remember to update private field too.
     subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
       "/a200_0000/sensors/lidar2d_0/scan", 10, std::bind(&HuskyPubSub::laser_scanner_callback, this, _1));

    // Create a timer that will trigger calls to the method timer_callback
    // Multiple timers with different durations can be used as necessary.
    // TODO: Update timer duration as required
    timer_ = this->create_wall_timer(
      300ms, std::bind(&HuskyPubSub::timer_callback, this));
  
  }
private:

void laser_scanner_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
right_laser = msg->ranges[180];
    RCLCPP_INFO(this->get_logger(), "The object right is %f, in distance %f ", right_laser, msg->ranges[180]);
    
front_laser = msg->ranges[360];
   RCLCPP_INFO(this->get_logger(), "The object front is %f, in distance %f ", front_laser,  msg->ranges[360]);
   
left_laser = msg->ranges[540];
  RCLCPP_INFO(this->get_logger(), "The object left is %f, in distance %f ", left_laser, msg->ranges[540]);
}

void movement_callback()
{
    auto message = geometry_msgs::msg::Twist();
    float turning_rate = 0.8;
    
    float safe_dist = 2.0;
    float max_speed = 0.5;
    float min_speed = 0.3;
    float avg_side = 0.5;



  // if(front_laser > safe_dist) {
  //   message.linear.x = min_speed;
  //   message.angular.z = 0.0;
  //  } else {
//    message.linear.x = max_speed;
//    if(left_laser < right_laser) {
//      message.angular.z = -turning_rate;
//    }
//     else {
//      message.angular.z = turning_rate;
//     }
//  }



      message.angular.z = ((left_laser - right_laser) / avg_side) * turning_rate;
      message.linear.x = (front_laser / safe_dist) * max_speed;

    RCLCPP_INFO(this->get_logger(), "Publishing: Linear Velocity %f, Angular Velocity=%f" ,  	message.linear.x, message.angular.z);

     publisher_->publish(message);
}

  //TODO rename method, especially if having multiple timers.
  void timer_callback()
  {
    //TODO add behaviour here to be repeated at regular intervals

    //Create an instance of the appropriate message type for publishing
  
   // message.linear.x = 1; //forwards
   // message.angular.z = 0.5; //left
  

    movement_callback();
  }


  //Declaration of private fields used for timer, publisher and subscriber
  //TODO: Update message types as appropriate for publishers and subscribers
  //Add other class variables here.
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;

};
//Main method defining entry point for program
int main(int argc, char * argv[])
{
  //Initialise ROS 2 for this node
  rclcpp::init(argc, argv);

  //Create the instance of the Node subclass and 
  // start the spinner with a pointer to the instance
  rclcpp::spin(std::make_shared<HuskyPubSub>());

  //When the node is terminated, shut down ROS 2 for this node
  rclcpp::shutdown();
  return 0;
}
