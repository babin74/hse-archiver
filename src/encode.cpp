#include "encode.hpp"
#include "core.hpp"
#include "binary_forest.hpp"
#include "priority_queue.hpp"
#include "bitstream_writer.hpp"

#include <algorithm>
#include <vector>
#include <numeric>
#include <fstream>
#include <cassert>

ArchiveEncoder::CharFrequency ArchiveEncoder::CharFrequencyCombiner::operator()(const CharFrequency& lhs,
                                                                                const CharFrequency& rhs) {
    return CharFrequency{.occurrences_count = lhs.occurrences_count + rhs.occurrences_count,
                         .character = std::min(lhs.character, rhs.character)};
}

bool ArchiveEncoder::HuffmanIteratorCompareGreater::operator()(const HuffmanTree::Iterator& lhs,
                                                               const HuffmanTree::Iterator& rhs) const {
    return std::tie(lhs->occurrences_count, lhs->character) > std::tie(rhs->occurrences_count, rhs->character);
}

ArchiveEncoder::ArchiveEncoder(BitWriter& bs)
    : bs_(std::ref(bs)), tree_(), root_(), codes_(), order_(), first_file_(true) {
}

ArchiveEncoder::~ArchiveEncoder() {
    if (!bs_.Closed()) {
        Close();
    }
}

void ArchiveEncoder::Encode(const std::string_view filename, std::unique_ptr<std::istream> is) {
    // Требуется для того, чтобы при вызове функции извне пользователь мог самостоятельно
    // не следить за тем, какой файл будет последним.
    if (!first_file_) {
        WriteCharacter(archive::ONE_MORE_FILE);
        root_.Reset();
    } else {
        first_file_ = false;
    }

    auto char_frequency = ArchiveEncoder::CalculateCharFrequencyArray(filename, is);
    GenerateHuffmanTree(char_frequency);

    for (auto& code : codes_) {
        code.clear();
    }

    tree_.ProvidePaths(root_, [&](const CharFrequency& char_frequency, const HuffmanTree::BinaryString& binary_string) {
        codes_[char_frequency.character] = binary_string;
    });

    СonvertHuffmanCodeToCanonicalForm();
    EncodeHeader();
    EncodeData(filename, is);
}

void ArchiveEncoder::EncodeFile(const std::string& filename) {
    auto stream = std::make_unique<std::ifstream>();
    stream->exceptions(std::ofstream::badbit);
    stream->open(filename, std::ios::binary);
    Encode(filename, std::move(stream));
}

void ArchiveEncoder::Close() {
    assert(!first_file_);
    WriteCharacter(archive::ARCHIVE_END);
    bs_.Close();
}

void ArchiveEncoder::WriteCharacter(Char ch) {
    for (auto bit : codes_[ch]) {
        bs_.WriteBit(bit);
    }
}

void ArchiveEncoder::EncodeHeader() {
    bs_.WriteInt(order_.size(), archive::ALPHABET_BIT_COUNT);
    for (size_t ch : order_) {
        bs_.WriteInt(ch, archive::ALPHABET_BIT_COUNT);
    }

    size_t max_symbol_code_size = 0;
    for (size_t ch : order_) {
        max_symbol_code_size = std::max(max_symbol_code_size, codes_[ch].size());
    }

    std::vector<size_t> symbol_count_with_code_size(max_symbol_code_size);
    for (size_t ch : order_) {
        ++symbol_count_with_code_size[codes_[ch].size() - 1];
    }

    for (size_t count : symbol_count_with_code_size) {
        bs_.WriteInt(count, archive::ALPHABET_BIT_COUNT);
    }
}

void ArchiveEncoder::EncodeData(std::string_view filename, std::unique_ptr<std::istream>& stream) {
    for (uint8_t byte : filename) {
        WriteCharacter(Char{byte});
    }
    WriteCharacter(archive::FILENAME_END);

    stream->clear();
    stream->seekg(0);
    uint8_t byte = 0;
    while (stream->read(reinterpret_cast<char*>(&byte), sizeof(uint8_t))) {
        WriteCharacter(Char{byte});
    }
}

void ArchiveEncoder::GenerateHuffmanTree(const CharFrequencyArray& distribution) {
    PriorityQueue<HuffmanTree::Iterator, HuffmanIteratorCompareGreater> queue;
    for (size_t i = 0; i < archive::CHARS_COUNT; ++i) {
        if (distribution[i] == 0) {
            continue;
        }

        queue.Emplace(tree_.EmplaceLeaf(CharFrequency{.occurrences_count = distribution[i], .character = Char{i}}));
    }

    while (queue.Size() > 1) {
        auto a = queue.Top();
        queue.Pop();
        auto b = queue.Top();
        queue.Pop();
        queue.Emplace(tree_.Unite(a, b));
    }

    root_ = queue.Top();
}

ArchiveEncoder::CharFrequencyArray ArchiveEncoder::CalculateCharFrequencyArray(const std::string_view filename,
                                                                               std::unique_ptr<std::istream>& stream) {
    CharFrequencyArray char_frequency;
    std::ranges::fill(char_frequency, 0);

    char_frequency[archive::FILENAME_END] = 1;
    char_frequency[archive::ONE_MORE_FILE] = 1;
    char_frequency[archive::ARCHIVE_END] = 1;

    for (uint8_t ch : filename) {
        ++char_frequency[ch];
    }

    stream->clear();
    stream->seekg(0);
    uint8_t byte = 0;
    while (stream->read(reinterpret_cast<char*>(&byte), sizeof(uint8_t))) {
        ++char_frequency[byte];
    }

    return char_frequency;
}

void ArchiveEncoder::СonvertHuffmanCodeToCanonicalForm() {
    order_.clear();
    for (size_t i = 0; i < archive::CHARS_COUNT; ++i) {
        if (!codes_[i].empty()) {
            order_.push_back(i);
        }
    }

    std::ranges::sort(order_, [&](size_t lhs, size_t rhs) {
        auto x = codes_[lhs].size() <=> codes_[rhs].size();
        if (x != std::strong_ordering::equal) {
            return x < 0;
        }
        return lhs < rhs;
    });

    std::vector<bool> current;
    for (size_t i : order_) {
        if (!current.empty()) {
            if (current.size() > codes_[i].size()) {
                current.resize(codes_[i].size());
            }

            size_t j = current.size() - 1;
            while (current[j]) {
                current[j] = false;
                --j;
            }
            current[j] = true;
        }

        while (current.size() < codes_[i].size()) {
            current.push_back(false);
        }

        codes_[i] = current;
    }
}