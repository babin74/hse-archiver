#include "bitstream_reader.hpp"

const char* BitReader::ReadException::what() const noexcept {
    return "Cannot read another byte";
}

BitReader::BitReader() : buffer_(0), current_pos_(0) {
}

size_t BitReader::GetBase() const {
    return 0;
}

bool BitReader::ReadWord(size_t& output) {
    return false;
}

bool BitReader::ReadInt(size_t& output, size_t size) {
    output = 0;
    for (size_t i = 0; i < size; ++i) {
        output <<= 1;
        bool bit = false;
        if (!ReadBit(bit)) {
            return false;
        }
        if (bit) {
            output |= 1;
        }
    }
    return true;
}

bool BitReader::ReadBit(bool& output) {
    if (current_pos_ == 0) {
        if (!ReadWord(buffer_)) {
            return false;
        }
        current_pos_ = GetBase();
    }

    --current_pos_;
    output = (buffer_ >> current_pos_) & 1;
    return true;
}

bool BitReader::ReadBit() {
    bool value = false;
    if (!ReadBit(value)) {
        throw ReadException();
    }
    return value;
}

size_t BitReader::ReadInt(size_t size) {
    size_t value = 0;
    if (!ReadInt(value, size)) {
        throw ReadException();
    }
    return value;
}

BitReaderU8::BitReaderU8(const std::vector<uint8_t>& data) : BitReader(), data_(data), first_not_readed(0) {
}

BitReaderU8::BitReaderU8(std::vector<uint8_t>&& data) : BitReader(), data_(std::move(data)), first_not_readed(0) {
}

const std::vector<uint8_t>& BitReaderU8::Data() const {
    return data_;
}

size_t BitReaderU8::GetBase() const {
    return 8;
}

bool BitReaderU8::ReadWord(size_t& output) {
    if (first_not_readed == data_.size()) {
        return false;
    }

    output = data_[first_not_readed++];
    return true;
}

BitReaderStream::BitReaderStream(std::unique_ptr<std::istream>&& is) : BitReader(), is_(std::move(is)) {
}

size_t BitReaderStream::GetBase() const {
    return 8;
}

bool BitReaderStream::ReadWord(size_t& output) {
    uint8_t byte = 0;
    is_->read(reinterpret_cast<char*>(&byte), sizeof(uint8_t));
    output = byte;
    return is_->good();
}