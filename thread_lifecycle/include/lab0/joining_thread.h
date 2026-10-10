#pragma once

#include <cstdio>
#include <system_error>
#include <thread>
#include <utility>

namespace lab0 {

/**
 * @brief JoiningThread is a pre-implemented std::jthread in C++
 *
 */
class JoiningThread {
  public:
    JoiningThread() noexcept = default;
    template <class Callable, class... Args> 
    explicit JoiningThread(Callable&& f, Args&&... args)
        : thread_(std::forward<Callable>(f), std::forward<Args>(args)...) {}

    explicit JoiningThread(std::thread t) noexcept
      : thread_(std::move(t))
    {}

    JoiningThread(JoiningThread&& other) noexcept
      : thread_(std::move(other.thread_))
    {}

    JoiningThread& operator=(JoiningThread&& other) noexcept {
      if(this != &other) {
        if(joinable()) {
          join();
        }
        thread_ = std::move(other.thread_);
      }
      return *this;
    }

    JoiningThread& operator=(std::thread other) noexcept {
      if(joinable()) {
        join();
      }
      thread_ = std::move(other);
      return *this;
    }

    JoiningThread(const JoiningThread&) = delete;
    JoiningThread& operator=(const JoiningThread&) = delete;

    ~JoiningThread() {
      if(joinable()) {
        try {
          join();
        } catch (const std::system_error& e) {
          std::fprintf(stderr, "JoiningThread: join() failed: %s\n", e.what());
        }
      }
    };

    void join() {
      thread_.join();
    }

    bool joinable() const noexcept {
      return thread_.joinable();
    }

  private:
    std::thread thread_;
};

} // namespace lab0
