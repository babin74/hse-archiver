#pragma once

#include <memory>
#include <vector>
#include <algorithm>

template <typename NodeInformation, typename Combiner>
class BinaryForest;

template <typename NodeInformation, typename Combiner>
class BinaryForestIterator {
public:
    using ForestType = BinaryForest<NodeInformation, Combiner>;
    using Node = typename ForestType::Node;
    friend ForestType;

    BinaryForestIterator() {
    }

    BinaryForestIterator(const BinaryForestIterator& iterator) : BinaryForestIterator(iterator.node_) {
    }

    BinaryForestIterator(BinaryForestIterator&& itearator) : BinaryForestIterator(std::move(itearator.node_)) {
    }

    BinaryForestIterator& operator=(const BinaryForestIterator& iterator) {
        node_ = iterator.node_;
        return *this;
    }

    BinaryForestIterator& operator=(BinaryForestIterator&& iterator) {
        node_.reset();
        node_.swap(iterator.node_);
        return *this;
    }

    const NodeInformation& operator*() const {
        return node_->information;
    }

    const NodeInformation* operator->() const {
        return &node_->information;
    }

    bool operator==(const BinaryForestIterator& another) const {
        return node_ == another.node_;
    }

    bool IsLeaf() const {
        return !node_->left && !node_->right;
    }

    operator bool() const {
        return node_.use_count() != 0;
    }

    BinaryForestIterator Left() const {
        return BinaryForestIterator(node_->left);
    }

    BinaryForestIterator Right() const {
        return BinaryForestIterator(node_->right);
    }

    void Swap(BinaryForestIterator& another) {
        node_.swap(another.node_);
    }

    void Reset() {
        node_.reset();
    }

private:
    explicit BinaryForestIterator(const std::shared_ptr<Node>& node) : node_(node) {
    }

    explicit BinaryForestIterator(std::shared_ptr<Node>&& node) : node_(node) {
    }

    std::shared_ptr<Node> node_;
};

/**
 * @brief Этот класс предназначен для работы с двоичными префиксными деревьями, с его помощью
 * также можно легко поддерживать информацию на поддеревьях бора.
 * @tparam NodeInformation информация, которая содержится в вершине.
 * @tparam Combiner предикат, позволяющий объединять информацию на поддеревьях
 */
template <typename NodeInformation, typename Combiner>
class BinaryForest {
private:
    struct Node;

public:
    using BinaryString = std::vector<bool>;
    using Iterator = BinaryForestIterator<NodeInformation, Combiner>;
    friend Iterator;

    BinaryForest() : combiner_() {
    }

    template <typename... Args>
    Iterator EmplaceLeaf(Args&&... args) {
        return Iterator(std::make_shared<Node>(Node{
            .information = NodeInformation{std::forward<Args>(args)...},
            .left = nullptr,
            .right = nullptr,
        }));
    }

    Iterator Unite(Iterator left_child, Iterator right_child) {
        auto vertex = std::make_shared<Node>(Node{
            .information = combiner_(*left_child, *right_child), .left = left_child.node_, .right = right_child.node_});
        return Iterator(vertex);
    }

    template <typename Callback>
    void ProvidePaths(Iterator root, Callback callback) const {
        BinaryString binary_string;
        ProvidePaths(root, callback, binary_string);
    }

private:
    struct Node {
        using ValueType = NodeInformation;

        NodeInformation information;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
    };

    template <typename Callback>
    void ProvidePaths(Iterator vertex, Callback& callback, BinaryString& binary_string) const {
        if (!vertex) {
            return;
        }

        if (vertex.IsLeaf()) {
            callback(*vertex, binary_string);
        } else {
            binary_string.push_back(false);
            ProvidePaths(vertex.Left(), callback, binary_string);
            binary_string.back() = true;
            ProvidePaths(vertex.Right(), callback, binary_string);
            binary_string.pop_back();
        }
    }

    Combiner combiner_;
};

template <typename NodeInformation, typename Combiner>
void swap(BinaryForestIterator<NodeInformation, Combiner>& lhs, BinaryForestIterator<NodeInformation, Combiner>& rhs) {
    lhs.Swap(rhs);
}
