#ifndef ANIM_ID_HPP
#define ANIM_ID_HPP


#include <cstdint>
#include <functional> // For std::hash

namespace anim {

/**
 * @brief A unique, immutable identifier for a Channel.
 *
 * Each Channel is assigned an Id on creation that never changes for its
 * lifetime (notably, it survives copies and is distinct from the channel's
 * name). Ids are cheap value types and can be used as keys in both ordered
 * (std::map / std::set, via operator<) and unordered (std::unordered_map /
 * std::unordered_set, via the std::hash specialization below) containers.
 */
struct Id {
    const uint64_t id; ///< The underlying identifier value (immutable).

    /// @brief Explicit conversion back to the underlying integer value.
    explicit operator uint64_t() const { return id; }

    /// @brief True if the two Ids share the same underlying value.
    bool operator==(const Id& other) const {
        return id == other.id;
    }
    /// @brief Negation of operator==.
    bool operator!=(const Id& other) const {
        return !(*this == other);
    }
    /// @brief Orders Ids by their underlying value (for use as a std::map / std::set key).
    bool operator<(const Id& other) const {
        return id < other.id;
    }

    /**
     * @brief Returns the sentinel "invalid" Id (the maximum uint64_t value).
     * @return An Id for which is_valid() is false.
     */
    static Id invalid() {
        return Id(static_cast<uint64_t>(-1));
    }

    /// @brief True unless this Id equals the invalid() sentinel.
    bool is_valid() const {
        return id != static_cast<uint64_t>(-1);
    }

private:
    /**
     * @brief Wraps a raw identifier value.
     *
     * Private so that ids can only originate from the library. An Id fabricated
     * by a caller would either fail to resolve or, because ids are handed out
     * from one counter shared by every Animation, resolve to some unrelated
     * channel. Obtain ids from Channel::id(); use invalid() for a sentinel.
     */
    explicit Id(uint64_t value) : id(value) {}

    friend class Animation; ///< Mints ids for the channels it creates.
    friend class Channel;   ///< Stores the id it was created with.
};

} // namespace anim

/// @brief std::hash specialization so Id can key unordered containers.
namespace std {
    template <>
    struct hash<anim::Id> {
        size_t operator()(const anim::Id& handle) const {
            return hash<uint64_t>()(handle.id);
        }
    };
}


#endif // ANIM_ID_HPP
