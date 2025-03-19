// PHS: Template combining together publisher and subscriber examples

//General C++ includes as needed
#include <chrono>
#include <functional>
#include <memory>
#include <string>
 #include <random>

//ROS 2 Specific includes
//Important: these includes should be reflected in the package.xml and CMakeLists.txt
#include "rclcpp/rclcpp.hpp"

// TODO: Add or replace with message type needed
#include "geometry_msgs/msg/twist.hpp" 
#include "std_msgs/msg/u_int32.hpp" 

using namespace std::chrono_literals;
using std::placeholders::_1;

/* Template giving outline for creating a program
 * that contains both a publisher and subscriber.
 */

class PubSubTemplate : public rclcpp::Node
{
public:
  //TODO: Update node name to reflect purpose of node
  PubSubTemplate() : Node("pub_sub_template")
  {
    // TODO: Update message type and topic name as appropriate for the publisher. Remember to update private field too.
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/a200_0000/cmd_vel", 10);
    
    
    // Create a timer that will trigger calls to the method timer_callback
    // Multiple timers with different durations can be used as necessary.
    // TODO: Update timer duration as required
    timer_ = this->create_wall_timer(
      1s, std::bind(&PubSubTemplate::timer_callback, this));
  }

private:

  //TODO rename method, especially if having multiple timers.
  void timer_callback()
  {
    //TODO add behaviour here to be repeated at regular intervals

    //Create an instance of the appropriate message type for publishing
    auto message = geometry_msgs::msg::Twist();
    
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    float x = distribution(generator);
    float z = distribution(generator) - 0.5;
     
    message.linear.x = x;
    message.angular.z = z;
    
    
    //Set the values for necessary fields in message type
    // On the commandline use
    //   $ ros2 interface show std_msgs/msg/UInt32
    // To check fields of message type

    //Log output for message to be published
    RCLCPP_INFO(this->get_logger(), "Publishing: Linear Velocity %f, Angular Velocity=%f" ,  	message.linear.x, message.angular.z);

    //publish the message to topic
    publisher_->publish(message);
  }

 

  //Declaration of private fields used for timer, publisher and subscriber
  //TODO: Update message types as appropriate for publishers and subscribers
  //Add other class variables here.
  rclcpp::TimerBase::SharedPtr timer_;
  std::default_random_engine generator;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;

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
