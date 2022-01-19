#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using DataT = std_msgs::msg::String;

void process(const DataT::SharedPtr& msg) {
  // define internal state
  static int data_count = 0;

  // protect shared state - data_count
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

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

  // create a callback group and add both subscriptions to it
  auto callback_group = node->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  rclcpp::SubscriptionOptions subopt;
  subopt.callback_group = callback_group;

  auto sub1 = node->create_subscription<DataT>("sensor1", /* history_depth */ 0, process_callback, subopt);
  auto sub2 = node->create_subscription<DataT>("sensor2", /* history_depth */ 0, process_callback, subopt);

  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), /* num_threads */ 4);
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();

  RCLCPP_INFO(rclcpp::get_logger("subscriber"), "subscriber process finished");
}