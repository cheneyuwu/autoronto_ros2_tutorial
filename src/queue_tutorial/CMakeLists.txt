cmake_minimum_required(VERSION 3.8)
project(queue_tutorial)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-march=native -O3 -pthread -Wall -Wextra)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(publisher src/publisher.cpp)
ament_target_dependencies(publisher rclcpp std_msgs)

add_executable(subscriber1_naive src/subscriber1_naive.cpp)
ament_target_dependencies(subscriber1_naive rclcpp std_msgs)
add_executable(subscriber2_ros_queue src/subscriber2_ros_queue.cpp)
ament_target_dependencies(subscriber2_ros_queue rclcpp std_msgs)
add_executable(subscriber3_ros_multithread_executor src/subscriber3_ros_multithread_executor.cpp)
ament_target_dependencies(subscriber3_ros_multithread_executor rclcpp std_msgs)
add_executable(subscriber4_cpp_queue_function src/subscriber4_cpp_queue_function.cpp)
ament_target_dependencies(subscriber4_cpp_queue_function rclcpp std_msgs)
add_executable(subscriber5_cpp_queue_class src/subscriber5_cpp_queue_class.cpp)
ament_target_dependencies(subscriber5_cpp_queue_class rclcpp std_msgs)


install(TARGETS
  publisher
  subscriber1_naive
  subscriber2_ros_queue
  subscriber3_ros_multithread_executor
  subscriber4_cpp_queue_function
  subscriber5_cpp_queue_class
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
