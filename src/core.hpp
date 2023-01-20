#pragma once

#include <cstddef>

namespace archive {

class Char {
public:
    inline constexpr explicit Char(size_t value) : value_(value) {
    }

    inline constexpr explicit Char() : value_(0) {
    }

    inline operator size_t() const {
        return value_;
    }

private:
    size_t value_;
};

constexpr Char FILENAME_END{256};
constexpr Char ONE_MORE_FILE{257};
constexpr Char ARCHIVE_END{258};
constexpr size_t CHARS_COUNT{259};
constexpr size_t ALPHABET_BIT_COUNT{9};

}  // namespace archive
