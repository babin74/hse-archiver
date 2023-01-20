#include <catch.hpp>

#include "../priority_queue.hpp"
#include <queue>
#include <random>
#include <algorithm>
#include <vector>
#include <numeric>

namespace {

template <typename T>
void CheckQueues(PriorityQueue<T>& lhs, std::priority_queue<T>& rhs) {
    REQUIRE(lhs.Empty() == rhs.empty());
    REQUIRE(lhs.Size() == rhs.size());
    if (!lhs.Empty()) {
        REQUIRE(lhs.Top() == rhs.top());
    }
}

template <typename T>
void Push(PriorityQueue<T>& lhs, std::priority_queue<T>& rhs, const T& value) {
    lhs.Push(value);
    rhs.push(value);
    CheckQueues(lhs, rhs);
}

template <typename T>
void Pop(PriorityQueue<T>& lhs, std::priority_queue<T>& rhs) {
    lhs.Pop();
    rhs.pop();
    CheckQueues(lhs, rhs);
}

}  // namespace

TEST_CASE("PriorityQueue") {
    PriorityQueue<int> my_queue;
    std::priority_queue<int> queue;
    CheckQueues(my_queue, queue);
    Push(my_queue, queue, 3);
    Push(my_queue, queue, 1);
    Push(my_queue, queue, 7);
    Push(my_queue, queue, 4);
    Pop(my_queue, queue);
    Pop(my_queue, queue);
    Pop(my_queue, queue);
    Pop(my_queue, queue);
}

TEST_CASE("PriorityQueue stress 1") {
    const size_t size = 1e5;
    std::mt19937 rng(1337228);

    std::vector<int> permutation(size);
    std::iota(permutation.begin(), permutation.end(), 1);
    std::ranges::shuffle(permutation, rng);

    PriorityQueue<int> my_queue;
    std::priority_queue<int> queue;

    for (auto x : permutation) {
        Push(my_queue, queue, x);
    }

    for (size_t i = 0; i < size; ++i) {
        Pop(my_queue, queue);
    }
}

TEST_CASE("PriorityQueue stress 2") {
    const size_t size = 1e5;
    std::mt19937 rng(1337228);

    std::vector<int> permutation(size);
    std::iota(permutation.begin(), permutation.end(), 1);

    PriorityQueue<int> my_queue;
    std::priority_queue<int> queue;

    std::ranges::shuffle(permutation, rng);
    for (auto x : permutation) {
        Push(my_queue, queue, x);
    }

    std::ranges::shuffle(permutation, rng);
    for (auto x : permutation) {
        Pop(my_queue, queue);
        Push(my_queue, queue, x);
    }
}

TEST_CASE("PriorityQueue constructor") {
    const size_t size = 2;
    PriorityQueue<std::string> queue(size, "aba");
    queue.Push("bab");

    REQUIRE(queue.Top() == "bab");
    queue.Pop();
    REQUIRE(queue.Top() == "aba");
    queue.Pop();
    REQUIRE(queue.Top() == "aba");
    queue.Pop();
    REQUIRE(queue.Empty());
}