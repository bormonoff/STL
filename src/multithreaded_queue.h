// It was interesting to me to write a safe thread container 
// but due to the API races it was a bad idea to write thread safe vector or map.
// I chose queue due to it's work model.

// Imagine that this class is needed to store some tasks. We have producers which
// generate tasks into this queue and consumers that get and execute tasks from this queue.

#include <condition_variable>
#include <queue>

namespace bormon {

template <typename Type>
Type limiter();

template <>
int limiter<int>() { return -1; }

template<typename Type>
class GenerateQueue {
public:
    GenerateQueue() = default;

    void push(Type obj) {
        std::unique_lock<std::mutex> lock{mutex_};
        buffer_.push(std::move(obj));
        cond_var_cons_.notify_one();
    }

    void pop(Type& obj) {
        std::unique_lock<std::mutex> lock{mutex_};
        cond_var_cons_.wait(lock, [this]() { return !buffer_.empty(); });
        Type data = std::move(buffer_.front());
        if (data == limiter<Type>()) { return; }
        obj = std::move(data);
        buffer_.pop();
    }
    
    void create_limiter_and_notify() {
        std::unique_lock<std::mutex> lock{mutex_};
        buffer_.push(limiter<Type>());
        cond_var_cons_.notify_all();
    }

private:
    std::queue<Type> buffer_;
    int current_{0};

    std::mutex mutex_;
    std::condition_variable cond_var_cons_;
};
}  // namespace bormon