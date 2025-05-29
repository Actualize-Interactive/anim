#ifndef ANIM_HANDLE_TYPE_HPP
#define ANIM_HANDLE_TYPE_HPP



namespace anim {

enum class HandleMode : uint8_t {
    flat = 0,
    smooth = 1,
    aligned = 2,
    free = 3,
    alignStrict = 4,
    alignFlex = 5,
    alignAdjustable = 6,
    count = 7
};





} // namespace anim

#endif // ANIM_HANDLE_TYPE_HPP