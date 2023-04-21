// It was interesting to me to write a safe thread container 
// but due to the API races it was a bad idea to write thread safe vector or map.
// I chose queue due to it's work model.

// Imagine that this class is needed to store some tasks. We have producers which
// generate tasks into this queue and consumers that get and execute tasks from this queue.

#include <condition_variable>
#include <queue>

namespace bormon {

template<typename Type>
class GenerateQueue {
public:
    explicit GenerateQueue(int size) 
        : buffer_(size) {
    }

    void push(Type obj) {
        std::unique_lock<std::mutex> lock{mutex_};
        cond_var_prod_.wait(lock, [this]() { return !full(); });
        ++current_;
        buffer_.push_back(std::move(obj));
        cond_var_cons_.notify_one();
    }

    void pop(Type& obj) {
        std::unique_lock<std::mutex> lock{mutex_};
        cond_var_cons_.wait(lock, [this]() { return !empty(); });
        obj = buffer_[current_];
        --current_;
        cond_var_prod_.notify_one();
    }

private:
    bool full() { return current_ >= buffer_.size() - 1; }
    bool empty() { return current_ <= 0; }

    std::vector<Type> buffer_;
    int current_{0};

    std::mutex mutex_;
    std::condition_variable cond_var_cons_;
    std::condition_variable cond_var_prod_;
};
}  // namespace bormon