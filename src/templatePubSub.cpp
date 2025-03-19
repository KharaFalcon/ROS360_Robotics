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

class PubSubTemplate : public rclcpp::Node
{
unsigned int value;
float count;
float average;
float sum;
public:
  //TODO: Update node name to reflect purpose of node
  PubSubTemplate() : Node("pub_sub_template")
  {
  value = 0;
    // TODO: Update message type and topic name as appropriate for the publisher. Remember to update private field too.
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/a200_0000/cmd_vel", 10);

    // TODO: Update message type and topic name as appropriate for the subscriber. Remember to update private field too.
     subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
       "/a200_0000/sensors/lidar2d_0/scan", 10, std::bind(&PubSubTemplate::topic_callback, this, _1));
    
    // Create a timer that will trigger calls to the method timer_callback
    // Multiple timers with different durations can be used as necessary.
    // TODO: Update timer duration as required
    timer_ = this->create_wall_timer(
      1300ms, std::bind(&PubSubTemplate::timer_callback, this));
  }

private:

  //TODO rename method, especially if having multiple timers.
  void timer_callback()
  {
    //TODO add behaviour here to be repeated at regular intervals

    //Create an instance of the appropriate message type for publishing
    auto message = geometry_msgs::msg::Twist();
    float x;
    float z;
    message.linear.x = 1; //forwards
    message.angular.z = 0.5; //left
    
    //Set the values for necessary fields in message type
    // On the commandline use
    //   $ ros2 interface show std_msgs/msg/UInt32
    // To check fields of message type
    
        //Log output for message to be published
    RCLCPP_INFO(this->get_logger(), "Publishing: Linear Velocity %f, Angular Velocity=%f" ,  	message.linear.x, message.angular.z);

    //publish the message to topic
    publisher_->publish(message);
  }

  //TODO rename method to better reflect topic it is subscribed to
  //TODO update message type as appropriate
  void topic_callback(const sensor_msgs::msg::LaserScan & msg)
  {
    //Log output of message received
    // On the commandline use
    //   $ ros2 interface show std_msgs/msg/UInt32
    // To check fields of message type
    //TODO update fields used from pre-defined message type
       count = 0;
      sum = 0;
    while (value < msg.ranges.size() / 2) {
    
    if (!std::isinf(msg.ranges[value])) {
    RCLCPP_INFO(this->get_logger(), "I heard %f from scanner=%u", msg.ranges[value],value); //prints every beam
    // Update class variables (based on message data)
    //count_++;
      count += 1;
      sum += msg.ranges[value];
    }
    value++;
   }
      if( count != 0) { 
        average = sum/count;
        RCLCPP_INFO(this->get_logger(), "The average =%f", average);
    }
   
  }

  //Declaration of private fields used for timer, publisher and subscriber
  //TODO: Update message types as appropriate for publishers and subscribers
  //Add other class variables here.
  rclcpp::TimerBase::SharedPtr timer_;
   rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
  size_t count_;
};

//Main method defining entry point for program
int main(int argc, char * argv[])
{
  //Initialise ROS 2 for this node
  rclcpp::init(argc, argv);

  //Create the instance of the Node subclass and 
  // start the spinner with a pointer to the instance
  rclcpp::spin(std::make_shared<PubSubTemplate>());

  //When the node is terminated, shut down ROS 2 for this node
  rclcpp::shutdown();
  return 0;
}
