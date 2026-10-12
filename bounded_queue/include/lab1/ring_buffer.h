#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
namespace lab1 {
template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity)
        : slots_(capacity)
    {
        if(capacity == 0)
            throw std::invalid_argument("capacity must be positive");
    }

    std::size_t size() const noexcept {
        return size_;
    }

    std::size_t capacity() const noexcept {
        return slots_.size();   //可使用的槽位数量，
    }

    bool empty() const noexcept {
        return size_ == 0;
    }    

    void push(T value) {
        assert(size_ < capacity());
        assert(!slots_[tail_].has_value());
        slots_[tail_].emplace(std::move(value));
        tail_ = (tail_ + 1) % capacity();
        ++size_;
    }

    T pop() {
        assert(size_ > 0);
        assert(slots_[head_].has_value());
        T value = std::move(*slots_[head_]);    //*是对optional解引用，取出T
        slots_[head_].reset(); // 销毁槽位中被移动的T
        head_ =(head_ + 1) % capacity();
        --size_;
        return value;
    }
private:
    std::vector<std::optional<T>> slots_;
    std::size_t head_ = 0;  //下一个取出位置
    std::size_t tail_ = 0; //下一个写入位置
    std::size_t size_ = 0;  //当前缓冲区有多少个T
};

}
