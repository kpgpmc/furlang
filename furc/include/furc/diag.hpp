#ifndef FURC_DIAG_HPP
#define FURC_DIAG_HPP

#include <ostream>
#include <string_view>

namespace furc {

/**
 * @brief A location in file.
 */
struct location {
    std::string_view filename;   /**< File's name */
    std::size_t      line   = 0; /**< Line */
    std::size_t      column = 0; /**< Column */

    /**
     * @brief Compare two locations for equality.
     *
     * @param rhs Location to compare against.
     * @return true if the locations are equal.
     */
    bool operator==(const location& rhs) const {
        return filename == rhs.filename && line == rhs.line && column == rhs.column;
    }

    /**
     * @brief Compare two locations for inequality.
     *
     * @param rhs Location to compare against.
     * @return true if the locations are not equal.
     */
    bool operator!=(const location& rhs) const { return !this->operator==(rhs); }
};

static inline std::ostream& operator<<(std::ostream& os, const location& location) {
    return os << location.filename << ':' << location.line + 1 << ':' << location.column + 1;
}

} // namespace furc

#endif // FURC_DIAG_HPP