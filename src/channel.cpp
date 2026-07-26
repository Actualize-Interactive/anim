#include "anim/channel.hpp"
#include "anim/bezier_utils.hpp"
#include "sampling.hpp"

namespace anim {

const Keyframe& Channel::create_keyframe(double time, 
    double value, Function function, HandleMode handle_mode) 
{
    return create_default_keyframe(Point(time, value), function, handle_mode);
}

const Keyframe& Channel::create_keyframe(const Point& position, 
    Function function, HandleMode handle_mode) 
{
    return create_default_keyframe(position, function, handle_mode);
}

const Keyframe& Channel::create_keyframe(double time, double value,
    const Point& in_handle, const Point& out_handle,
    Function function, HandleMode handle_mode) 
{
    return insert_keyframe(Keyframe(time, value, function, handle_mode, in_handle, out_handle));
}

const Keyframe& Channel::create_keyframe(const Point& position,
    const Point& in_handle, const Point& out_handle,
    Function function, HandleMode handle_mode) 
{
    return insert_keyframe(Keyframe(position, function, handle_mode, in_handle, out_handle));
}

const Keyframe& Channel::create_keyframe(const Keyframe& reference_keyframe) 
{
    return insert_keyframe(Keyframe(reference_keyframe));
}

const Keyframe& Channel::emplace_keyframe(Keyframe&& keyframe) {
        return insert_keyframe(std::move(keyframe));
    }

bool Channel::has_keyframe(double time) const
{   // Check if a keyframe exists with in 1/200th of a second toleranc
    return std::any_of(m_keyframes.begin(), m_keyframes.end(),
                       [time](const Keyframe& kf) {
                         return nearly_equal(kf.time(), time, 0.005); 
                        });
}

void Channel::delete_keyframe(size_t index)
{
    if (index < 0 || index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    
    m_keyframes.erase(m_keyframes.begin() + index);
    apply_last_keyframe_inheritance();
}

const Keyframe &Channel::keyframe(size_t index) const
{
    if (index < 0 || index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    
    return m_keyframes[index];
}

const Keyframe &Channel::prev_keyframe(double time) const
{
    if (m_keyframes.empty()) {
        throw std::out_of_range("No keyframes available");
    }
    
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), time,
                               [](const Keyframe& kf, double t) {
                                   return kf.time() < t;
                               });
    
    if (it == m_keyframes.begin()) {
        throw std::out_of_range("No previous keyframe available");
    }
    
    return *(--it);
}

const Keyframe &Channel::next_keyframe(double time) const
{
    if (m_keyframes.empty()) {
        throw std::out_of_range("No keyframes available");
    }
    
    auto it = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), time,
                               [](double t, const Keyframe& kf) {
                                   return t < kf.time();
                               });
    
    if (it == m_keyframes.end()) {
        throw std::out_of_range("No next keyframe available");
    }
    
    return *it;
}

const Keyframe &Channel::closest_keyframe(double time) const
{
    if (m_keyframes.empty()) {
        throw std::out_of_range("No keyframes available");
    }
    
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), time,
                               [](const Keyframe& kf, double t) {
                                   return kf.time() < t;
                               });
    
    if (it == m_keyframes.begin()) {
        return *it; // Closest is the first keyframe
    }
    
    if (it == m_keyframes.end()) {
        return *(--it); // Closest is the last keyframe
    }
    
    // Compare distances to the previous and current keyframes
    auto prev_it = it - 1;
    return (std::abs(prev_it->time() - time) <= std::abs(it->time() - time)) ? *prev_it : *it;
}

const std::vector<Keyframe>& Channel::keyframes() const
{
    return m_keyframes;
}

void Channel::update_keyframe(size_t index, const Keyframe& keyframe)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    if (*it == keyframe) {
        return; // No change, no need to update
    }
    
    // If updating the last keyframe and cache is valid, invalidate it
    if (index == m_keyframes.size() - 1 && m_cache_valid) {
        invalidate_cache();
    }
    
    // Update the keyframe at the specified index
    it = m_keyframes.erase(it);
    it = m_keyframes.insert(it, keyframe);
    clamp_keyframe_time(it, keyframe.time());
    update_local_handles(it);
    
    // No need to apply inheritance for simple keyframe updates
    // Inheritance is only applied during structural changes (add/remove)
}

void Channel::set_keyframe_time(size_t index, double new_time)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    Point new_position(new_time, it->position.value);
    update_keyframe_position(it, new_position);
}
        
void Channel::set_keyframe_value(size_t index, double value)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    update_keyframe_position(it, Point(it->position.time, value));
}

void Channel::set_keyframe_position(size_t index, const Point& position)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    update_keyframe_position(it, position);
}


void Channel::set_keyframe_position(size_t index, double time, double value)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    update_keyframe_position(it, Point(time, value));
}

void Channel::set_keyframe_in_handle(size_t index, const Point& in_handle)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->in_handle = in_handle;
    update_local_handles(it, GrabbedHandle::InHandle);
    
    // Invalidate cache if we updated the last keyframe's handles
    if (index == m_keyframes.size() - 1) {
        invalidate_cache();
    }
}

void Channel::set_keyframe_out_handle(size_t index, const Point& out_handle)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->out_handle = out_handle;
    update_local_handles(it, GrabbedHandle::OutHandle);
    
    // Invalidate cache if we updated the last keyframe's handles
    if (index == m_keyframes.size() - 1) {
        invalidate_cache();
    }
}

void Channel::set_keyframe_function(size_t index, Function function)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    
    auto it = m_keyframes.begin() + index;
    it->function = function;
    update_local_handles(it);
    
    // Apply inheritance logic based on which keyframe was modified
    if (m_keyframes.size() >= 2) {
        size_t last_index = m_keyframes.size() - 1;
        size_t second_last_index = last_index - 1;
        
        if (index == last_index) {
            // Modifying last keyframe - invalidate cache so it keeps its new value
            invalidate_cache();
        } else if (index == second_last_index) {
            // Modifying second-to-last keyframe - last keyframe should inherit
            apply_last_keyframe_inheritance(false);
            // Invalidate cache so the last keyframe's inherited values become its new intended values
            invalidate_cache();
        }
        // Other keyframes don't affect inheritance
    }
}

void Channel::set_keyframe_handle_mode(size_t index, HandleMode handle_mode)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    
    auto it = m_keyframes.begin() + index;
    it->handle_mode = handle_mode;
    update_local_handles(it);
    
    // Apply inheritance logic based on which keyframe was modified
    if (m_keyframes.size() >= 2) {
        size_t last_index = m_keyframes.size() - 1;
        size_t second_last_index = last_index - 1;
        
        if (index == last_index) {
            // Modifying last keyframe - invalidate cache so it keeps its new value
            invalidate_cache();
        } else if (index == second_last_index) {
            // Modifying second-to-last keyframe - last keyframe should inherit
            apply_last_keyframe_inheritance(false);
            // Invalidate cache so the last keyframe's inherited values become its new intended values
            invalidate_cache();
        }
        // Other keyframes don't affect inheritance
    }
}

double Channel::evaluate(double time, double* prev_t) const {
    if (m_keyframes.empty()) {
        return 0.0;
    }
    
    if (m_keyframes.size() == 1) {
        return m_keyframes[0].value();
    }
    
    double start_time = m_keyframes.front().time();
    double end_time = m_keyframes.back().time();
    double duration = end_time - start_time;
    
    // Handle out-of-bounds time based on extend settings
    if (time < start_time) {
        switch (m_extend_start) {
            case Extend::Hold:
                return m_keyframes.front().value();
            case Extend::Repeat:
                if (duration > 0.0) {
                    // Map time into the valid range by cycling
                    double cycles_before = std::ceil((start_time - time) / duration);
                    time = time + cycles_before * duration;
                } else {
                    return m_keyframes.front().value();
                }
                break;
            case Extend::Mirror:
                if (duration > 0.0) {
                    // For mirror mode, we create a ping-pong pattern
                    // Each full cycle is 2*duration long (start->end->start)
                    double distance_before = start_time - time;
                    double cycle_length = 2.0 * duration;
                    
                    // Map to position within a cycle
                    double pos_in_cycle = std::fmod(distance_before, cycle_length);
                    
                    if (pos_in_cycle <= duration) {
                        // First half of cycle: normal direction (going backwards from start)
                        time = start_time + pos_in_cycle;
                    } else {
                        // Second half of cycle: reverse direction
                        time = end_time - (pos_in_cycle - duration);
                    }
                } else {
                    return m_keyframes.front().value();
                }
                break;
        }
    } else if (time > end_time) {
        switch (m_extend_end) {
            case Extend::Hold:
                return m_keyframes.back().value();
            case Extend::Repeat:
                if (duration > 0.0) {
                    // Map time into the valid range by cycling
                    double cycles_after = std::ceil((time - end_time) / duration);
                    time = time - cycles_after * duration;
                } else {
                    return m_keyframes.back().value();
                }
                break;
            case Extend::Mirror:
                if (duration > 0.0) {
                    // For mirror mode, we create a ping-pong pattern
                    // Each full cycle is 2*duration long (start->end->start)
                    double distance_after = time - end_time;
                    double cycle_length = 2.0 * duration;
                    
                    // Map to position within a cycle
                    double pos_in_cycle = std::fmod(distance_after, cycle_length);
                    
                    if (pos_in_cycle <= duration) {
                        // First half of cycle: reverse direction (going backwards from end)
                        time = end_time - pos_in_cycle;
                    } else {
                        // Second half of cycle: normal direction
                        time = start_time + (pos_in_cycle - duration);
                    }
                } else {
                    return m_keyframes.back().value();
                }
                break;
        }
    }
    
    // Find the keyframes that bracket the requested time
    auto upper = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), time,
                                 [](double t, const Keyframe& kf) {
                                     return t < kf.time();
                                 });

    // Guard the boundaries before forming the bracketing pair. When the
    // (possibly extend-remapped) time lands exactly on the last keyframe,
    // upper == end() and dereferencing it is undefined behaviour -- silent in
    // release builds, but a checked-iterator assertion in debug builds.
    // Symmetrically, upper == begin() means time is at/before the first
    // keyframe. In both cases the answer is the boundary keyframe's value.
    if (upper == m_keyframes.end()) {
        return m_keyframes.back().value();
    }
    if (upper == m_keyframes.begin()) {
        return m_keyframes.front().value();
    }
    auto lower = upper - 1;

    const Keyframe& start_kf = *lower;
    const Keyframe& end_kf = *upper;
    
    if (start_kf.function == Function::Linear) {
        double t = (time - start_kf.time()) / (end_kf.time() - start_kf.time());
        return start_kf.value() + t * (end_kf.value() - start_kf.value());
    } else if (start_kf.function == Function::Constant) {
        return start_kf.value();
    }

    double t = bezier_utils::solve_t_for_time(
        start_kf.position, start_kf.out_handle, 
        end_kf.in_handle, end_kf.position, 
        time,
        prev_t
    );
    return bezier_utils::evaluate_cubic_bezier(start_kf.position, start_kf.out_handle, end_kf.in_handle, end_kf.position, t).value;
}




std::vector<double> Channel::evaluate_range(double start_time, double end_time, int num_samples) const {
    if (num_samples <= 1) {
        return {evaluate(start_time)};
    }
    if (start_time > end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }
    
    // For equal times, just return the value at that time
    if (start_time == end_time) {
        return {evaluate(start_time)};
    }
    
    std::vector<double> result(num_samples);
    double step = (end_time - start_time) / (num_samples - 1);
    
    double* prev_t = nullptr; 
    for (int i = 0; i < num_samples; i++) {
        double time = start_time + i * step;
        result[i] = evaluate(time, prev_t);
        prev_t = &result[i]; // Update prev_t to point to the current value
    }
    
    return result;
}

std::vector<double> Channel::evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const {
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    if (start_time > end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }

    // For equal times, just return the value at that time
    if (start_time == end_time) {
        return {evaluate(start_time)};
    }

    // The range is half-open: end_time is not sampled, so a span of n sample
    // periods yields n samples rather than n + 1. This cannot delegate to
    // evaluate_range, which spreads a sample count across a closed interval and
    // so would only produce a spacing of 1 / sample_rate for one particular
    // count -- and not even then when the span is not a whole number of periods.
    const size_t count = detail::sample_count(end_time - start_time, sample_rate);
    std::vector<double> result(count);

    double* prev_t = nullptr;
    for (size_t i = 0; i < count; i++) {
        // Derived from the index rather than accumulated, so every sample sits
        // exactly one period from the last with no drift along a long range.
        double time = start_time + static_cast<double>(i) / sample_rate;
        result[i] = evaluate(time, prev_t);
        prev_t = &result[i]; // Update prev_t to point to the current value
    }

    return result;
}

double Channel::start_time() const {
    if (m_keyframes.empty()) {
        return 0.0;
    }
    return m_keyframes.front().time();
}

double Channel::end_time() const {
    if (m_keyframes.empty()) {
        return 0.0;
    }
    return m_keyframes.back().time();
}

double Channel::length() const {
    if (m_keyframes.empty()) {
        return 0.0;
    }
    return end_time() - start_time();
}

size_t Channel::num_samples(double sample_rate) const
{
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    
    if (m_keyframes.empty()) {
        return 0;
    }
    
    double duration = end_time() - start_time();
    if (duration == 0.0) {
        // A single keyframe, or several sharing one time: there is no span to
        // divide, and evaluate_range_by_rate returns the value at that time.
        return 1;
    }
    return detail::sample_count(duration, sample_rate);
}

Extend Channel::extend_start() const {
    return m_extend_start;
}

Extend Channel::extend_end() const {
    return m_extend_end;
}

void Channel::set_extend_start(Extend extend) {
    m_extend_start = extend;
}

void Channel::set_extend_end(Extend extend) {
    m_extend_end = extend;
}

const Keyframe &Channel::create_default_keyframe(const Point &position, Function function, HandleMode handle_mode)
{
    if (function == Function::Constant || function == Function::Linear) {
        return insert_keyframe(Keyframe(position, function, handle_mode));
    }

    // Find the insertion point for the new keyframe
    auto next_it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), position.time,
                            [](const Keyframe& kf, double t) {
                                return kf.time() < t;
                            });

    Keyframe* prev_kf_ptr = nullptr;
    if (next_it != m_keyframes.begin()) {
        prev_kf_ptr = &*(next_it - 1);
    }

    Keyframe* next_kf_ptr = nullptr; // Initialize to nullptr
    if (next_it != m_keyframes.end()) { // Check before dereferencing
        next_kf_ptr = &*next_it;
    }

    // initialize the handles with smooth mode
    Keyframe new_keyframe(position, function, HandleMode::Smooth);
    update_handles(new_keyframe, prev_kf_ptr, next_kf_ptr);
    new_keyframe.handle_mode = handle_mode; // set to requested mode, insert_keyframe will call update_local_handles
    return insert_keyframe(next_it, std::move(new_keyframe));
}

const Keyframe &Channel::insert_keyframe(Keyframe&& keyframe, GrabbedHandle grabbed_handle)
{
    // Find the insertion point for the new keyframe
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), keyframe.time(),
                               [](const Keyframe& kf, double t) {
                                   return kf.time() < t;
                               });
    return insert_keyframe(it, std::move(keyframe), grabbed_handle);
}

const Keyframe &Channel::insert_keyframe(KeyframeIt it, Keyframe&& keyframe, GrabbedHandle grabbed_handle)
{
    // Restore any cached keyframe BEFORE insertion to avoid index issues
    // Find the cached keyframe by time to avoid index problems
    if (m_cache_valid && m_cached_keyframe_index < m_keyframes.size()) {
        double cached_time = m_last_keyframe_cache.time();
        for (size_t i = 0; i < m_keyframes.size(); ++i) {
            if (nearly_equal(m_keyframes[i].time(), cached_time, 0.005)) {
                auto& cached_keyframe = m_keyframes[i];
                cached_keyframe.function = m_last_keyframe_cache.function;
                cached_keyframe.handle_mode = m_last_keyframe_cache.handle_mode;
                break;
            }
        }
    }
    
    // If a keyframe already exists within 1/200th of a second, replace it in
    // place rather than inserting a duplicate at the same time.
    if (it != m_keyframes.end() && nearly_equal(it->time(), keyframe.time(), 0.005)) {
        *it = std::move(keyframe);
        update_local_handles(it, grabbed_handle);
        apply_last_keyframe_inheritance(false);
        return *it;
    }

    it = m_keyframes.insert(it, std::move(keyframe));
    update_local_handles(it, grabbed_handle);
    apply_last_keyframe_inheritance(false); // Don't restore cache again, just apply inheritance and cache new last keyframe

    return *it;
}

void Channel::update_keyframe_position(KeyframeIt it, const Point& position)
{
    auto in_handle_delta = it->in_handle - it->position;
    auto out_handle_delta = it->out_handle - it->position;

    clamp_keyframe_time(it, position.time);
    it->position.value = position.value;
    
    it->in_handle = it->position + in_handle_delta;
    it->out_handle = it->position + out_handle_delta;

    update_local_handles(it);
    
    // Invalidate cache if we updated the last keyframe's position
    size_t index = it - m_keyframes.begin();
    if (index == m_keyframes.size() - 1) {
        invalidate_cache();
    }
}

void Channel::clamp_keyframe_time(KeyframeIt it, double time)
{
    bool has_prev = (it != m_keyframes.begin());
    bool has_next = (it + 1 != m_keyframes.end());
    
    if (!has_prev && !has_next) { // no neighbors, no clamping
        it->position.time = time;
    } else if (!has_prev) { // if no previous keyframe, clamp to next
        auto next_it = it + 1;
        it->position.time = std::min(time, next_it->time());
    } else if (!has_next) { // if no next keyframe, clamp to previous
        auto prev_it = it - 1;
        it->position.time = std::max(time, prev_it->time());
    } else {
        auto prev_it = it - 1;
        auto next_it = it + 1;
        it->position.time = std::clamp(time, prev_it->time(), next_it->time());
    }
}

void Channel::update_local_handles(std::vector<Keyframe>::iterator it, GrabbedHandle grabbed_handle)
{
    Keyframe* prev_it = nullptr;
    if (it != m_keyframes.begin()) {
        prev_it = &*(it - 1);
        Keyframe* prev_it_prev_it = (it - 1 != m_keyframes.begin()) ? &*(it - 2) : nullptr;
        update_handles(*prev_it, prev_it_prev_it, &*it);
    }
    
    Keyframe* next_it = nullptr;
    if (it + 1 != m_keyframes.end()) {
        next_it = &*(it + 1);
        Keyframe* next_it_next_it = (it + 2 != m_keyframes.end()) ? &*(it + 2) : nullptr;
        update_handles(*next_it, &*it, next_it_next_it);
    }
    update_handles(*it, prev_it, next_it, grabbed_handle);
}

void Channel::update_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr, GrabbedHandle grabbed_handle)
{
    if (keyframe.function == Function::Linear || keyframe.function == Function::Constant) {
        ensure_linear_handles_time_boundary(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::Flat) {
        apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::Smooth) {
        apply_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::Aligned || 
               keyframe.handle_mode == HandleMode::AlignStrict || 
               keyframe.handle_mode == HandleMode::AlignFlex || 
               keyframe.handle_mode == HandleMode::AlignAdjustable) {
        apply_aligned_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr, grabbed_handle);
    } else if (keyframe.handle_mode == HandleMode::Free) {
        constrain_handles_time(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else {
        throw std::invalid_argument("Unknown handle type");
    }
}

void Channel::copy_keyframes_from(const Channel& source) {
    m_keyframes = source.m_keyframes;
    apply_last_keyframe_inheritance();
}

void Channel::apply_last_keyframe_inheritance(bool restore_cache) const {
    if (m_keyframes.size() < 2) {
        return; // No inheritance needed for single keyframe or empty channel
    }
    
    // If this is a structural change (keyframe addition/deletion) and we have a valid cache,
    // restore the cached keyframe FIRST before any other operations
    if (restore_cache && m_cache_valid && m_cached_keyframe_index < m_keyframes.size()) {
        auto& cached_keyframe = m_keyframes[m_cached_keyframe_index];
        cached_keyframe.function = m_last_keyframe_cache.function;
        cached_keyframe.handle_mode = m_last_keyframe_cache.handle_mode;
    }
    
    size_t last_index = m_keyframes.size() - 1;
    size_t second_last_index = last_index - 1;
    
    // Cache the current last keyframe's original state before applying inheritance
    // Always cache when restore_cache is true (structural changes), or when cache is invalid,
    // or when the last keyframe index has changed (new keyframe added)
    if (restore_cache || !m_cache_valid || m_cached_keyframe_index != last_index) {
        m_last_keyframe_cache = m_keyframes[last_index];
        m_cached_keyframe_index = last_index;
        m_cache_valid = true;
    }
    
    // Apply inheritance to the last keyframe
    auto& last_keyframe = m_keyframes[last_index];
    const auto& second_last_keyframe = m_keyframes[second_last_index];
    
    last_keyframe.function = second_last_keyframe.function;
    last_keyframe.handle_mode = second_last_keyframe.handle_mode;
}

void Channel::invalidate_cache() const {
    m_cache_valid = false;
    m_cached_keyframe_index = SIZE_MAX;
}

bool Channel::operator==(const Channel& other) const {
    // Compare names and keyframes, but not IDs (copied channels should be equal)
    if (m_name != other.m_name) {
        return false;
    }
    
    if (m_keyframes.size() != other.m_keyframes.size()) {
        return false;
    }
    
    // Compare all keyframes
    for (size_t i = 0; i < m_keyframes.size(); ++i) {
        if (m_keyframes[i] != other.m_keyframes[i]) {
            return false;
        }
    }
    
    return true;
}

bool Channel::operator!=(const Channel& other) const {
    return !(*this == other);
}

} // namespace anim
