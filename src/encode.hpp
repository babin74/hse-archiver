#pragma once

#include "core.hpp"
#include "bitstream_writer.hpp"
#include "binary_forest.hpp"

#include <string>
#include <string_view>
#include <memory>
#include <array>

class ArchiveEncoder {
public:
    explicit ArchiveEncoder(BitWriter& bs);
    ~ArchiveEncoder();

    void Encode(const std::string_view filename, std::unique_ptr<std::istream> istream);
    void EncodeFile(const std::string& filename);
    void Close();

private:
    struct CharFrequency {
        size_t occurrences_count;
        archive::Char character;
    };

    struct CharFrequencyCombiner {
        CharFrequency operator()(const CharFrequency& lhs, const CharFrequency& rhs);
    };

    using CharFrequencyArray = std::array<size_t, archive::CHARS_COUNT>;
    using HuffmanTree = BinaryForest<CharFrequency, CharFrequencyCombiner>;
    using Char = archive::Char;
    using CharCodesArray = std::array<HuffmanTree::BinaryString, archive::CHARS_COUNT>;

    struct HuffmanIteratorCompareGreater {
        bool operator()(const HuffmanTree::Iterator& lhs, const HuffmanTree::Iterator& rhs) const;
    };

    BitWriter& bs_;
    HuffmanTree tree_;
    HuffmanTree::Iterator root_;
    CharCodesArray codes_;
    std::vector<size_t> order_;
    bool first_file_;

    void WriteCharacter(Char ch);
    void EncodeHeader();
    void EncodeData(std::string_view filename, std::unique_ptr<std::istream>& stream);

    static CharFrequencyArray CalculateCharFrequencyArray(const std::string_view filename,
                                                          std::unique_ptr<std::istream>& stream);
    void GenerateHuffmanTree(const CharFrequencyArray& distribution);
    void СonvertHuffmanCodeToCanonicalForm();
};
