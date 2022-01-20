#include <chrono>
#include <queue>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using DataT = std_msgs::msg::String;

///
class BaseDataProcessor {
 protected:
  void start() {
    //
    process_thread = std::thread(&BaseDataProcessor::process, this);
  }

  void terminate() {
    // send the stop signal to the worker thread
    std::unique_lock<std::mutex> lock(mutex);
    stop = true;
    cv_nonempty_or_stop.notify_one();

    // wait for the worker thread to finish
    cv_thread_finished.wait(lock, [&] { return thread_count == 0; });
    if (process_thread.joinable()) process_thread.join();
  }

 public:
  void process_callback(const DataT::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(mutex);

    // add data to the queue
    queue.push(msg);

    // notify the process thread that there is new data
    cv_nonempty_or_stop.notify_one();
  };

 private:
  void process() {
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
      process_(msg);
    };
  }

 private:
  virtual void process_(const DataT::SharedPtr& msg) = 0;

 private:
  // cpp queue to store incoming data
  std::queue<DataT::SharedPtr> queue;
  bool stop = false;     // flag to indicate that the thread should stop and return
  int thread_count = 1;  // number of additional threads this process forks
  // lock to protect access to queue
  std::mutex mutex;
  // condition variables for inter-thread communication
  std::condition_variable cv_nonempty_or_stop;
  std::condition_variable cv_thread_finished;
  std::thread process_thread;
};

class DataProcessor : public BaseDataProcessor {
 public:
  DataProcessor() { start(); }
  ~DataProcessor() { terminate(); }

 private:
  void process_(const DataT::SharedPtr& msg) override {
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

  // define internal state
  int data_count = 0;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("subscriber");

  DataProcessor data_processor;

  auto process_callback = [&](const DataT::SharedPtr msg) { data_processor.process_callback(msg); };

  auto sub1 = node->create_subscription<DataT>("sensor1", /* history_depth */ 0, process_callback);
  auto sub2 = node->create_subscription<DataT>("sensor2", /* history_depth */ 0, process_callback);

  rclcpp::spin(node);
  rclcpp::shutdown();

  RCLCPP_INFO(rclcpp::get_logger("subscriber"), "subscriber process finished");
}