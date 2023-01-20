#include <catch.hpp>

#include "../binary_forest.hpp"
#include <iostream>
#include <vector>

namespace {

struct CombinerSum {
    int operator()(int a, int b) {
        return a + b;
    }
};

struct Catalan {
    std::string operator()(const std::string& lhs, const std::string& rhs) {
        return "(" + lhs + ")" + rhs;
    }
};

std::vector<bool> operator"" _bin(const char* string) {
    std::string_view view(string);
    std::vector<bool> res;
    for (auto ch : view) {
        res.push_back(ch == '1');
    }
    return res;
}

}  // namespace

TEST_CASE("BinaryForest") {
    BinaryForest<int, CombinerSum> forest_sum;

    auto a = forest_sum.EmplaceLeaf(2);
    auto b = forest_sum.EmplaceLeaf(3);
    auto c = forest_sum.EmplaceLeaf(5);

    auto p = forest_sum.Unite(a, b);
    auto q = forest_sum.Unite(p, c);

    REQUIRE(*q == 10);
    REQUIRE(q.Left() == p);
    REQUIRE(p.Right() == b);

    BinaryForest<std::string, Catalan> forest_catalan;
    auto empty = forest_catalan.EmplaceLeaf();
    auto word = forest_catalan.Unite(empty, empty);  // Abuse like this is allowed
    auto sequence = forest_catalan.Unite(empty, forest_catalan.Unite(word, empty));
    REQUIRE(*sequence == "()(())");

    using PathType = std::vector<std::pair<int, std::vector<bool>>>;
    PathType path;
    forest_sum.ProvidePaths(q, [&](int x, const std::vector<bool>& a) { path.emplace_back(x, a); });

    REQUIRE(path == PathType{{2, 00_bin}, {3, 01_bin}, {5, 1_bin}});
}