#pragma once

#include "core.hpp"
#include "bitstream_reader.hpp"
#include "binary_forest.hpp"

#include <exception>

class ArchiveDecoder {
public:
    class ProcessError : public std::runtime_error {
    public:
        inline ProcessError(const char* message) : std::runtime_error(message) {
        }
    };

    ArchiveDecoder(BitReader& bs);

    bool Done() const;

    std::string Decode(std::ostream& ostream);
    std::string DecodeFile();

private:
    using Char = archive::Char;

    struct CharUnite {
        inline Char operator()(Char lhs, Char rhs) {
            return Char{0};
        }
    };

    using DecodingTree = BinaryForest<Char, CharUnite>;

    BitReader& bs_;
    DecodingTree tree_;
    DecodingTree::Iterator root_;
    bool done_;

    void DecodeHeader();
    std::string DecodeName();
    void DecodeData(std::ostream& ostream);
    Char ReadCharacter();

    class DecodingTreeBuilder {
    public:
        DecodingTreeBuilder(DecodingTree& tree);
        void Push(DecodingTree::Iterator vertex, size_t length);
        DecodingTree::Iterator Get();

    private:
        std::vector<std::pair<DecodingTree::Iterator, size_t>> stack_;
        DecodingTree& tree_;
    };
};
