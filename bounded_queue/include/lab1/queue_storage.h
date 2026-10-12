#pragma once

#include <cassert>
#include <stdexcept>
#include <cstddef>
#include <queue>
#include <utility>

namespace lab1 {
template <typename T>
class QueueStorage {
public:
    explicit QueueStorage(std::size_t capacity)
        :capacity_(capacity)
    {
        if(capacity_ == 0) {
            throw std::invalid_argument("queue capacity must be greater than zero");
        }
    }

    std::size_t size() const noexcept{
        return queue_.size();
    }

    std::size_t capacity() const noexcept{
        return capacity_;
    }

    bool empty() const noexcept{
        return queue_.empty();
    }

    void push(T value) {
        assert(queue_.size() < capacity_);
        queue_.push(std::move(value));
    }

    T pop() {
        assert(!queue_.empty());
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

private:
    std::queue<T> queue_;
    std::size_t capacity_;
};
}
