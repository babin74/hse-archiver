#include "bitstream_writer.hpp"

#include <stdexcept>

BitWriter::BitWriter() : buffer_(0), buffer_size_(0), closed_(false) {
}

BitWriter::~BitWriter() {
    Close();
}

void BitWriter::Close() {
    if (closed_) {
        return;
    }

    if (buffer_size_ != 0) {
        WriteLastWord(buffer_, buffer_size_);
        buffer_ = 0;
        buffer_size_ = 0;
    }
    closed_ = true;
}

bool BitWriter::Closed() const {
    return closed_;
}

void BitWriter::WriteBit(bool bit) {
    if (closed_) {
        throw std::invalid_argument("Tried write bit to closed stream.");
    }

    buffer_ <<= 1;
    if (bit) {
        buffer_ |= 1;
    }
    ++buffer_size_;

    if (buffer_size_ == GetBase()) {
        WriteWord(buffer_);
        buffer_size_ = 0;
        buffer_ = 0;
    }
}

void BitWriter::WriteInt(size_t value, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        WriteBit((value >> (size - i - 1)) & 1);
    }
}

size_t BitWriter::GetBase() const {
    return 0;  // чтобы было неповадно
}

void BitWriter::WriteWord(size_t word) {
}

void BitWriter::WriteLastWord(size_t word, size_t count) {
}

BitWriterString::BitWriterString() {
}

const std::string& BitWriterString::Data() const {
    return data_;
}

size_t BitWriterString::GetBase() const {
    return 1;
}

void BitWriterString::WriteWord(size_t word) {
    data_.push_back('0' + word);
}

void BitWriterString::WriteLastWord(size_t word, size_t count) {
}

BitWriterU8::BitWriterU8() {
}

const std::vector<uint8_t>& BitWriterU8::Data() const {
    return data_;
}

size_t BitWriterU8::GetBase() const {
    return 8;
}

void BitWriterU8::WriteWord(size_t word) {
    data_.push_back(word);
}

void BitWriterU8::WriteLastWord(size_t word, size_t count) {
    data_.push_back(word << (GetBase() - count));
}

BitWriterStream::BitWriterStream(std::unique_ptr<std::ostream>&& os) : os_(std::move(os)) {
}

size_t BitWriterStream::GetBase() const {
    return 8;
}

void BitWriterStream::WriteWord(size_t word) {
    os_->write(reinterpret_cast<const char*>(&word), sizeof(uint8_t));
}

void BitWriterStream::WriteLastWord(size_t word, size_t count) {
    word <<= GetBase() - count;
    os_->write(reinterpret_cast<const char*>(&word), sizeof(uint8_t));
}