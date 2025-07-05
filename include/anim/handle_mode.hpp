#ifndef ANIM_HANDLE_TYPE_HPP
#define ANIM_HANDLE_TYPE_HPP

#include <cstdint>

namespace anim {

enum class HandleMode : uint8_t {
    Flat = 0,
    Smooth = 1,
    Aligned = 2,
    Free = 3,
    AlignStrict = 4,
    AlignFlex = 5,
    AlignAdjustable = 6,
    Count = 7
};





} // namespace anim

#endif // ANIM_HANDLE_TYPE_HPP