#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <utility>
#include "queue_storage.h"
#include "ring_buffer.h"

namespace lab1 {
template <typename T, typename Storage = RingBuffer<T>> 
class BoundedBlockingQueue {
  public:
    explicit BoundedBlockingQueue(std::size_t capacity)
      : storage_(capacity), capacity_(capacity)
    { }

    void push(T value) {
      std::unique_lock<std::mutex> lock(mutex_);
      not_full_.wait(lock, [this] {return storage_.size() < capacity_;} );
      storage_.push(std::move(value));
      not_empty_.notify_one();
    }

    std::optional<T> pop() {
      std::unique_lock<std::mutex> lock(mutex_);
      not_empty_.wait(lock, [this] { return !storage_.empty(); });
      T value = storage_.pop();
      not_full_.notify_one();
      return value;
    }

    void close();

    bool is_closed() const noexcept;

    bool try_push_for(T value, std::chrono::nanoseconds timeout);

    std::optional<T> try_pop_for(std::chrono::nanoseconds timeout);


    std::size_t size() const noexcept;
  private:
    Storage storage_;
    std::size_t capacity_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
};

} // namespace lab1
