//Based on minimal client example from ROS 2 tutorials


#include "rcl_interfaces/srv/set_parameters.hpp"
#include "rcl_interfaces/msg/parameter.hpp"
#include "rclcpp/rclcpp.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <vector>

using namespace std::chrono_literals;



class ColourClient : public rclcpp::Node
{
  public:
    ColourClient() : Node("colourClient")
    {
      //
      
    }


    void send_request()
    {
      auto parameter = rcl_interfaces::msg::Parameter();
      auto request = std::make_shared<rcl_interfaces::srv::SetParameters::Request>();

      auto client = this->create_client<rcl_interfaces::srv::SetParameters>("/colour_detector/set_parameters");

      parameter.name = "lower_limits";
      parameter.value.type = 7; // specified integer array value 
      parameter.value.integer_array_value = std::vector<long int>{0,0,100};

      request->parameters.push_back(parameter);

      parameter.name = "upper_limits";
      parameter.value.type = 7; // specified integer array value 
      parameter.value.integer_array_value = std::vector<long int>{50,50,255};

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

};



int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);

  //auto colour_client = std::make_shared<ColourClient>();

  ColourClient colour_client = ColourClient();
  colour_client.send_request();

  rclcpp::shutdown();
  return 0;
}
