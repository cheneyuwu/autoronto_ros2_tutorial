#include <chrono>
#include <queue>

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

  // cpp queue to store incoming data
  std::queue<DataT::SharedPtr> queue;
  bool stop = false;     // flag to indicate that the thread should stop and return
  int thread_count = 1;  // number of additional threads this process forks
  // lock to protect access to queue
  std::mutex mutex;
  // condition variables for inter-thread communication
  std::condition_variable cv_nonempty_or_stop;  // notify when queue is non-empty or stop is set
  std::condition_variable cv_thread_finished;   // notify when a thread has finished

  // fork an additional thread to process data
  std::thread process_thread([&] {
    while (true) {
      std::unique_lock lock(mutex);

      // wait for data to be available or for the thread to be stopped
      cv_nonempty_or_stop.wait(lock, [&] { return stop || (!queue.empty()); });

      // if the thread should be stopped, return
      if (stop) {
        --thread_count;
        cv_thread_finished.notify_all();
        return;
      }

      // get the front in queue
      const auto msg = queue.front();
      queue.pop();

      // unlock the queue so that new data can be added
      lock.unlock();

      // process the data
      process(msg);
    };
  });

  auto process_callback = [&](const DataT::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(mutex);

    // add data to the queue
    queue.push(msg);

    // notify the process thread that there is new data
    cv_nonempty_or_stop.notify_one();
  };

  auto sub1 = node->create_subscription<DataT>("sensor1", /* history_depth */ 0, process_callback);
  auto sub2 = node->create_subscription<DataT>("sensor2", /* history_depth */ 0, process_callback);

  rclcpp::spin(node);
  rclcpp::shutdown();

  // send the stop signal to the worker thread
  std::unique_lock<std::mutex> lock(mutex);
  stop = true;
  cv_nonempty_or_stop.notify_one();

  // wait for the worker thread to finish
  cv_thread_finished.wait(lock, [&] { return thread_count == 0; });
  if (process_thread.joinable()) process_thread.join();

  RCLCPP_INFO(rclcpp::get_logger("subscriber"), "subscriber process finished");
}