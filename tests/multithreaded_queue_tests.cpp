#include <chrono>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "multithreaded_queue.h"

using namespace std::chrono_literals;

namespace {

const std::string THQUEUE = "[MultiThreadedQueue]";

std::mutex mutex;
int tasks_count = 1000;

void produce(bormon::GenerateQueue<int>& queue)  {
    while(true) {
        int current;
        {
            std::lock_guard<std::mutex> lock{mutex};
            if (tasks_count <= 0) { break; }
            current = tasks_count;
            --tasks_count;
        }
        // Imitate task production
        std::this_thread::sleep_for(0.1ms);
        queue.push(current);
    }
    queue.notify_all();
}

void consume(bormon::GenerateQueue<int>& queue) {
    while(true) {
        int current;
        {
            std::lock_guard<std::mutex> lock{mutex};
            if (tasks_count <= 0) { break; }
        }
        // Imitate task execution
        queue.pop(current);
        std::this_thread::sleep_for(0.1ms);
    }
    queue.notify_all();
}

constexpr int buf_size = 10;
}  // namespace

SCENARIO("Queue", THQUEUE) {
    const size_t num_threads = std::thread::hardware_concurrency();
    bormon::GenerateQueue<int> queue{buf_size};
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (size_t thread = 0; thread < num_threads / 2; ++thread) {
        producers.emplace_back(produce, std::ref(queue));
    }

    for (size_t thread = 0; thread < num_threads / 2; ++thread) {
        consumers.emplace_back(consume, std::ref(queue));
    }

    for (auto& it : producers) {
        it.join();
    }

    for (auto& it : consumers) {
        it.join();
    }
}
