#pragma once

#include <istream>
#include <vector>
#include <istream>
#include <memory>

class BitReader {
public:
    class ReadException : public std::exception {
        const char* what() const noexcept override;
    };

    bool ReadBit(bool& output);
    bool ReadInt(size_t& output, size_t size);

    bool ReadBit();
    size_t ReadInt(size_t size);

protected:
    BitReader();

    /// @brief Получить количество бит, которые можно считать за раз. Это значение обязано не
    /// превосходить количество бит, которые можно вместить в size_t
    virtual size_t GetBase() const;

    /// @brief Считать слово, состоящее из GetBase() бит
    /// @param word Слово, которое требуется считать из потока
    virtual bool ReadWord(size_t& output);

private:
    size_t buffer_;
    size_t current_pos_;
};

class BitReaderU8 : public BitReader {
public:
    BitReaderU8(const std::vector<uint8_t>& data);
    BitReaderU8(std::vector<uint8_t>&& data);

    const std::vector<uint8_t>& Data() const;

protected:
    size_t GetBase() const override;
    bool ReadWord(size_t& output) override;

private:
    std::vector<uint8_t> data_;
    size_t first_not_readed;
};

class BitReaderStream : public BitReader {
public:
    explicit BitReaderStream(std::unique_ptr<std::istream>&& is);

protected:
    size_t GetBase() const override;
    bool ReadWord(size_t& output) override;

private:
    std::unique_ptr<std::istream> is_;
};