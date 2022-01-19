#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using DataT = std_msgs::msg::String;

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("publisher");

  // fake data publisher from sensor 1 at 1Hz
  auto pub1 = node->create_publisher<DataT>("sensor1", 1);
  DataT msg1;
  msg1.data = "sensor1";
  auto sensor1_timer = node->create_wall_timer(1s, [&] {
    pub1->publish(msg1);
    RCLCPP_WARN(rclcpp::get_logger("publisher"), "publish sensor1 data");
  });

  // fake data publisher from sensor 2 at 4Hz
  auto pub2 = node->create_publisher<DataT>("sensor2", 1);
  DataT msg2;
  msg2.data = "sensor2";
  auto sensor2_timer = node->create_wall_timer(250ms, [&] {
    pub2->publish(msg2);
    RCLCPP_ERROR(rclcpp::get_logger("publisher"), "publish sensor2 data");
  });

  rclcpp::spin(node);
  rclcpp::shutdown();
}