#ifndef FURC_DIAG_HPP
#define FURC_DIAG_HPP

#include <ostream>
#include <string_view>

namespace furc {

struct location {
    std::string_view filename;
    std::size_t      line   = 0;
    std::size_t      column = 0;

    bool operator==(const location& rhs) const {
        return filename == rhs.filename && line == rhs.line && column == rhs.column;
    }

    bool operator!=(const location& rhs) const { return !this->operator==(rhs); }
};

static inline std::ostream& operator<<(std::ostream& os, const location& location) {
    return os << location.filename << ':' << location.line + 1 << ':' << location.column + 1;
}

} // namespace furc

#endif // FURC_DIAG_HPP