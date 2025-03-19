cmake_minimum_required(VERSION 3.8)
project(kmf3_ros360_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(ros360_cv_msgs REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(OpenCV REQUIRED)
find_package(image_transport REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)


add_executable(helloworld src/helloworld.cpp)
add_executable(examplePublisher src/examplePublisher.cpp)
add_executable(exampleSubscriber src/exampleSubscriber.cpp)
add_executable(lidarSub src/lidarSub.cpp)
add_executable(randomRobot src/randomRobot.cpp)
add_executable(templatePubSub src/templatePubSub.cpp)
add_executable(huskyPubSub src/huskyPubSub.cpp)
add_executable(visualPerception src/visualPerception.cpp)
add_executable(assignment24 src/assignment24.cpp)
add_executable(colourClient src/colourClient.cpp)



ament_target_dependencies(examplePublisher rclcpp std_msgs)
ament_target_dependencies(exampleSubscriber rclcpp std_msgs)
ament_target_dependencies(lidarSub rclcpp sensor_msgs)
ament_target_dependencies(randomRobot rclcpp geometry_msgs std_msgs)
ament_target_dependencies(templatePubSub rclcpp sensor_msgs std_msgs geometry_msgs)
ament_target_dependencies(huskyPubSub rclcpp sensor_msgs std_msgs geometry_msgs)
ament_target_dependencies(visualPerception rclcpp sensor_msgs std_msgs geometry_msgs ros360_cv_msgs rcl_interfaces)
ament_target_dependencies(assignment24 rclcpp sensor_msgs std_msgs geometry_msgs ros360_cv_msgs rcl_interfaces image_transport cv_bridge OpenCV tf2 tf2_ros)
ament_target_dependencies(colourClient rclcpp sensor_msgs std_msgs geometry_msgs ros360_cv_msgs rcl_interfaces)


install(TARGETS
helloworld
examplePublisher
exampleSubscriber
lidarSub
randomRobot
templatePubSub
huskyPubSub
visualPerception
assignment24
colourClient
DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
launch config
DESTINATION share/${PROJECT_NAME}/
)

target_include_directories(helloworld PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
  
target_compile_features(helloworld PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

install(TARGETS helloworld
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
