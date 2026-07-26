#ifndef ANIM_ANIMATION_HPP
#define ANIM_ANIMATION_HPP

#include "anim/channel.hpp"
#include "anim/id.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace anim {

/**
 * @brief A named collection of animation channels.
 *
 * An Animation owns its Channel objects and is the only way to create
 * them, assigning each a unique Id. Channels can be looked up by index,
 * by name, or by id, reordered, and removed. The animation also carries an
 * overall time range (start_time() / end_time()) used for sampling.
 */
class Animation {
public:
    /// @brief Constructs an unnamed animation.
    Animation() = default;
    /// @brief Constructs an animation with the given name.
    explicit Animation(const std::string& name) : m_name(name) {}

    /// @brief The animation's name.
    inline const std::string& name() const { return m_name; }
    /// @brief Renames the animation.
    inline void set_name(const std::string& name) { m_name = name; }

    /// @brief Creates a new channel named @p channel_name and returns it.
    Channel& create_channel(const std::string& channel_name);
    /**
     * @brief Creates a new channel and inserts it at @p index.
     * @throws std::out_of_range if @p index is greater than the channel count.
     */
    Channel& create_channel(const std::string& channel_name, size_t index);
    /**
     * @brief Creates a deep copy of @p source_channel within this animation.
     * @param source_channel The channel to copy keyframes from.
     * @param new_name Name for the copy; if empty, the source name plus "_copy".
     * @return The newly created channel (with its own unique id).
     */
    Channel& copy_channel(const Channel& source_channel, const std::string& new_name = "");

    /// @brief Returns a deep copy of this animation (channels get fresh ids).
    Animation copy() const;
    /// @brief Returns a deep copy of this animation with a new name.
    Animation copy(const std::string& new_name) const;

    /// @brief Returns the channel at @p index. @throws std::out_of_range if out of range.
    const Channel& channel(size_t index) const;
    /// @brief Indexed access. @throws std::out_of_range if out of range.
    const Channel& operator[](size_t index) const;
    /// @brief Returns the channel at @p index. @throws std::out_of_range if out of range.
    Channel& channel(size_t index);
    /// @brief Indexed access. @throws std::out_of_range if out of range.
    Channel& operator[](size_t index);

    /// @brief Returns the first channel named @p channel_name. @throws std::out_of_range if none matches.
    Channel& channel(const std::string& channel_name);
    /// @brief Name lookup. @throws std::out_of_range if none matches.
    Channel& operator[](const std::string& channel_name);
    /// @brief Returns the channel with @p channel_id. @throws std::out_of_range if none matches.
    Channel& channel(Id channel_id);
    /// @brief Id lookup. @throws std::out_of_range if none matches.
    Channel& operator[](Id channel_id);

    /// @brief Returns the first channel named @p channel_name. @throws std::out_of_range if none matches.
    const Channel& channel(const std::string& channel_name) const;
    /// @brief Name lookup. @throws std::out_of_range if none matches.
    const Channel& operator[](const std::string& channel_name) const;
    /// @brief Returns the channel with @p channel_id. @throws std::out_of_range if none matches.
    const Channel& channel(Id channel_id) const;
    /// @brief Id lookup. @throws std::out_of_range if none matches.
    const Channel& operator[](Id channel_id) const;

    /// @brief Number of channels.
    inline size_t size() const { return m_channels.size(); }
    /// @brief Number of channels (alias for size()).
    inline size_t num_channels() const { return m_channels.size(); }
    /// @brief True if a channel named @p channel_name exists.
    bool has_channel(const std::string& channel_name) const;
    /// @brief True if the animation has no channels.
    inline bool empty() const { return m_channels.empty(); }

    /**
     * @brief Moves the channel at @p from_index to @p to_index.
     * @throws std::out_of_range if either index is out of range.
     */
    void reorder_channel(size_t from_index, size_t to_index);
    /**
     * @brief Moves the channel named @p channel_name to @p to_index.
     * @throws std::out_of_range if the name is not found or @p to_index is out of range.
     */
    void reorder_channel(const std::string& channel_name, size_t to_index);
    /**
     * @brief Moves the channel with @p channel_id to @p to_index.
     * @throws std::out_of_range if the id is not found or @p to_index is out of range.
     */
    void reorder_channel(Id channel_id, size_t to_index);

    /// @brief Removes all channels.
    void clear();
    /// @brief Removes the channel at @p index. @throws std::out_of_range if out of range.
    void remove_channel(size_t index);
    /// @brief Removes the first channel named @p channel_name. @throws std::out_of_range if none matches.
    void remove_channel(const std::string& channel_name);
    /// @brief Removes the channel with @p channel_id. @throws std::out_of_range if none matches.
    void remove_channel(Id channel_id);

    /// @brief The names of all channels, in order.
    std::vector<std::string> channel_names() const;
    /// @brief Direct access to the owned channel vector.
    const std::vector<std::unique_ptr<Channel>>& channels();
    /// @brief Direct read access to the owned channel vector.
    const std::vector<std::unique_ptr<Channel>>& channels() const;

    /// @brief The animation's start time.
    inline double start_time() const { return m_start_time; }
    /// @brief Sets the start time (clamped so it does not exceed the end time).
    void set_start_time(double start_time);

    /// @brief The animation's end time.
    inline double end_time() const { return m_end_time; }
    /// @brief Sets the end time (clamped so it is not below the start time).
    void set_end_time(double end_time);

    /// @brief The animation's duration (end_time() - start_time()).
    double length() const;
    /**
     * @brief Sets the duration by moving the end time relative to the start.
     * @throws std::invalid_argument if @p length is negative.
     */
    void set_length(double length);

    /**
     * @brief Number of samples spanning the animation at @p sample_rate.
     * @param sample_rate Samples per unit time; must be positive.
     * @return Sample count (0 when there are no channels).
     * @throws std::invalid_argument if @p sample_rate is not positive.
     */
    int num_samples(double sample_rate) const;

    /// @brief Equality across name, time range and channels.
    bool operator==(const Animation& other) const;
    /// @brief Negation of operator==.
    bool operator!=(const Animation& other) const;

private:
    static uint64_t s_next_channel_id;
    std::string m_name;
    std::vector<std::unique_ptr<Channel>> m_channels;
    std::unordered_map<Id, Channel*> m_channel_map;
    double m_start_time { 0.0 };
    double m_end_time { 30.0 };

    uint64_t next_channel_id() {
        return s_next_channel_id++;
    }
};

} // namespace anim

#endif // ANIM_ANIMATION_HPP
