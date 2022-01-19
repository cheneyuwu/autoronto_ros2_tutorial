#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using DataT = std_msgs::msg::String;

void process(const DataT::SharedPtr& msg) {
  // define internal state
  static int data_count = 0;

  // log where the data comes from
  if (msg->data == "sensor1") {
    RCLCPP_WARN(rclcpp::get_logger("subscriber"), "processing sensor1 data, count: %d", data_count);
  } else if (msg->data == "sensor2") {
    RCLCPP_ERROR(rclcpp::get_logger("subscriber"), "processing sensor2 data, count: %d", data_count);
  }

  // modify internal state
  ++data_count;

  // simulate processing time
  std::this_thread::sleep_for(msg->data == "sensor1" ? 500ms : 50ms);
}

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("subscriber");

  auto process_callback = [&](const DataT::SharedPtr msg) { process(msg); };

  auto sub1 = node->create_subscription<DataT>("sensor1", /* history_depth */ 0, process_callback);
  auto sub2 = node->create_subscription<DataT>("sensor2", /* history_depth */ 0, process_callback);

  rclcpp::spin(node);
  rclcpp::shutdown();

  RCLCPP_INFO(rclcpp::get_logger("subscriber"), "subscriber process finished");
}