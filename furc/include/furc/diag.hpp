#ifndef FURC_DIAG_HPP
#define FURC_DIAG_HPP

#include <ostream>
#include <string_view>

namespace furc {

struct location {
    std::string_view filename;
    std::size_t      line;
    std::size_t      column;
};

static inline std::ostream& operator<<(std::ostream& os, const location& location) {
    return os << location.filename << ':' << location.line + 1 << ':' << location.column + 1;
}

} // namespace furc

#endif // FURC_DIAG_HPP