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
    void push(Type obj) {}
    void pop() {}

private:
    std::queue<Type> buffer_;

    std::mutex mutex_;
    std::condition_variable cond_var_;
};
}  // namespace bormon