#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <memory>

class BitWriter {
public:
    void WriteBit(bool bit);

    void WriteInt(size_t value, size_t size);

    void Close();

    bool Closed() const;

    ~BitWriter();

protected:
    BitWriter();

    /// @brief Получить количество бит, которые можно вывести за раз. Это значение обязано не
    /// превосходить количество бит, которые можно вместить в size_t
    virtual size_t GetBase() const;

    /// @brief Вывести слово, состоящее из GetBase() бит
    /// @param word Слово, которое требуется вывести в поток
    virtual void WriteWord(size_t word);

    /// @brief Вывести слово, состоящее из count бит. Вызывается, при закрытии потока, в случае
    /// когда вывелись не все биты и есть некоторый остаток.
    /// @param word Слово, которое требуется вывести.
    /// @param count Количество бит в слове.
    virtual void WriteLastWord(size_t word, size_t count);

private:
    size_t buffer_;
    size_t buffer_size_;
    bool closed_;
};

class BitWriterString : public BitWriter {
public:
    BitWriterString();

    const std::string& Data() const;

protected:
    size_t GetBase() const override;
    void WriteWord(size_t word) override;
    void WriteLastWord(size_t word, size_t count) override;

private:
    std::string data_;
};

class BitWriterU8 : public BitWriter {
public:
    BitWriterU8();

    const std::vector<uint8_t>& Data() const;

protected:
    size_t GetBase() const override;
    void WriteWord(size_t word) override;
    void WriteLastWord(size_t word, size_t count) override;

private:
    std::vector<uint8_t> data_;
};

class BitWriterStream final : public BitWriter {
public:
    explicit BitWriterStream(std::unique_ptr<std::ostream>&& os);

protected:
    size_t GetBase() const override;
    void WriteWord(size_t word) override;
    void WriteLastWord(size_t word, size_t count) override;

private:
    std::unique_ptr<std::ostream> os_;
};