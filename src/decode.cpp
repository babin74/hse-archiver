#include "decode.hpp"
#include "core.hpp"

#include <fstream>

ArchiveDecoder::ArchiveDecoder(BitReader& bs) : bs_(std::ref(bs)), tree_(), root_(), done_(false) {
}

bool ArchiveDecoder::Done() const {
    return done_;
}

std::string ArchiveDecoder::DecodeFile() {
    DecodeHeader();
    auto name = DecodeName();
    std::ofstream stream(name, std::ios::binary);
    DecodeData(stream);
    return name;
}

std::string ArchiveDecoder::Decode(std::ostream& stream) {
    DecodeHeader();
    auto name = DecodeName();
    DecodeData(stream);
    return name;
}

void ArchiveDecoder::DecodeHeader() {
    try {
        size_t alphabet_size = bs_.ReadInt(archive::ALPHABET_BIT_COUNT);
        std::vector<Char> order(alphabet_size);
        for (Char& ch : order) {
            ch = Char{bs_.ReadInt(archive::ALPHABET_BIT_COUNT)};
        }

        root_.Reset();

        DecodingTreeBuilder builder(tree_);
        for (size_t len = 1, i = 0; i < order.size(); ++len) {
            size_t count = bs_.ReadInt(archive::ALPHABET_BIT_COUNT);
            if (i + count > order.size()) {
                throw ProcessError("Inconsistency in header.");
            }

            for (size_t i_first = i; i < i_first + count; ++i) {
                builder.Push(tree_.EmplaceLeaf(order[i]), len);
            }
        }

        root_ = builder.Get();
    } catch (const BitReader::ReadException& exception) {
        throw ProcessError("Error while reading file-header.");
    }
}

ArchiveDecoder::DecodingTreeBuilder::DecodingTreeBuilder(DecodingTree& tree) : stack_(), tree_(std::ref(tree)) {
}

void ArchiveDecoder::DecodingTreeBuilder::Push(DecodingTree::Iterator vertex, size_t length) {
    if (!stack_.empty() && stack_.back().second == length) {
        if (length == 0) {
            throw ProcessError("Incorrectly defined huffman tree.");
        }

        auto previous = stack_.back().first;
        stack_.pop_back();
        Push(tree_.Unite(previous, vertex), length - 1);
    } else {
        stack_.emplace_back(vertex, length);
    }
}

ArchiveDecoder::DecodingTree::Iterator ArchiveDecoder::DecodingTreeBuilder::Get() {
    if (stack_.size() != 1 || stack_[0].second != 0) {
        throw ProcessError("Incorrectly defined huffman tree.");
    }

    return stack_[0].first;
}

archive::Char ArchiveDecoder::ReadCharacter() {
    auto curret_node = root_;
    while (!curret_node.IsLeaf()) {
        const bool bit = bs_.ReadBit();
        if (bit) {
            curret_node = curret_node.Right();
        } else {
            curret_node = curret_node.Left();
        }
    }
    return *curret_node;
}

std::string ArchiveDecoder::DecodeName() {
    try {
        std::string name;

        while (true) {
            Char ch = ReadCharacter();
            if (ch == archive::FILENAME_END) {
                break;
            }

            if (ch == archive::ARCHIVE_END || ch == archive::ONE_MORE_FILE) {
                throw ProcessError("An incorrect character was found in the file name.");
            }

            name.push_back(static_cast<char>(ch));
        }

        return name;
    } catch (const BitReader::ReadException& exception) {
        throw ProcessError("Error while reading file-name.");
    }
}

void ArchiveDecoder::DecodeData(std::ostream& os) {
    try {
        while (true) {
            Char ch = ReadCharacter();
            if (ch == archive::FILENAME_END) {
                throw ProcessError("An incorrect character was found in the file content.");
            }

            if (ch == archive::ARCHIVE_END) {
                done_ = true;
                break;
            } else if (ch == archive::ONE_MORE_FILE) {
                break;
            } else {
                os << static_cast<char>(ch);
            }
        }
    } catch (const BitReader::ReadException& exception) {
        throw ProcessError("Error while reading file-content.");
    }
}