#ifndef ANIM_ID_HPP
#define ANIM_ID_HPP


#include <cstdint>
#include <functional> // For std::hash

namespace anim {

// 1. Define Id as a struct (or class)
struct Id {
    // Make the 'id' member const
    const uint64_t id;

    // Constructor: 'id' MUST be initialized in the member initializer list
    explicit Id(uint64_t value) : id(value) {}

    // Default constructor could initialize to a specific "invalid" or "null" ID
    // For example, if 0 is a valid ID, you might need a different sentinel.
    // Or, ensure users always provide an ID.
    // Let's assume for now 0 could be valid, and we'll use a static factory for invalid.
    // Id() : id(0) {} // Or some other default if 0 isn't special

    // Allow explicit conversion back to uint64_t if needed
    explicit operator uint64_t() const { return id; }

    // For comparisons
    bool operator==(const Id& other) const {
        return id == other.id;
    }
    bool operator!=(const Id& other) const {
        return !(*this == other);
    }
    bool operator<(const Id& other) const {
        return id < other.id;
    }

    // Define a sentinel value for an invalid handle
    static Id invalid() {
        // Using static_cast<uint64_t>(-1) which is max uint64_t
        return Id(static_cast<uint64_t>(-1));
    }

    bool isValid() const {
        return id != static_cast<uint64_t>(-1);
    }
};

} // namespace anim

// 2. Provide a hash function specialization if you plan to use Id as a key in std::unordered_map
namespace std {
    template <>
    struct hash<anim::Id> {
        size_t operator()(const anim::Id& handle) const {
            return hash<uint64_t>()(handle.id);
        }
    };
}


#endif // ANIM_ID_HPP