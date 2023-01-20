#pragma once

#include <algorithm>
#include <vector>

template <typename Value, typename Comparator = std::less<Value>>
class PriorityQueue {
public:
    using SizeType = typename std::vector<Value>::size_type;

    PriorityQueue() {
    }

    template <typename... Args>
    PriorityQueue(Args... args) : data_(args...) {
        std::ranges::sort(data_, comparator_);
        std::ranges::reverse(data_);
    }

    bool Empty() const {
        return data_.empty();
    }

    SizeType Size() const {
        return data_.size();
    }

    const Value& Top() const {
        return data_[0];
    }

    void Push(const Value& v) {
        data_.push_back(v);
        LiftVertex(Size() - 1);
    }

    void Push(Value&& v) {
        data_.push_back(std::move(v));
        LiftVertex(Size() - 1);
    }

    template <typename... Args>
    void Emplace(Args... args) {
        data_.emplace_back(args...);
        LiftVertex(Size() - 1);
    }

    void Pop() {
        using std::swap;
        swap(data_.front(), data_.back());
        data_.pop_back();
        LowerVertex(0);
    }

private:
    Comparator comparator_;
    std::vector<Value> data_;

    void LiftVertex(SizeType vertex) {
        using std::swap;
        while (vertex != 0) {
            const SizeType parent = (vertex - 1) / 2;
            if (comparator_(data_[parent], data_[vertex])) {
                swap(data_[parent], data_[vertex]);
                vertex = parent;
            } else {
                break;
            }
        }
    }

    void LowerVertex(SizeType vertex) {
        using std::swap;
        while (true) {
            const SizeType left_child = 2 * vertex + 1;
            const SizeType right_child = 2 * vertex + 2;
            if (left_child >= Size()) {
                break;
            }

            SizeType push_child = left_child;
            if (right_child >= Size()) {
                if (!comparator_(data_[vertex], data_[left_child])) {
                    break;
                }
            } else {
                if (comparator_(data_[vertex], data_[left_child])) {
                    if (comparator_(data_[left_child], data_[right_child])) {
                        push_child = right_child;
                    }
                } else {
                    if (comparator_(data_[vertex], data_[right_child])) {
                        push_child = right_child;
                    } else {
                        break;
                    }
                }
            }

            swap(data_[vertex], data_[push_child]);
            vertex = push_child;
        }
    }
};