#include "anim/channel.hpp"
#include "anim/handle_utils.hpp"

#include <iostream>

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

const Keyframe& Channel::emplace_keyframe(Keyframe&& keyframe) {
        return insert_keyframe(std::move(keyframe));
    }

bool Channel::has_keyframe(double time) const
{
    return std::any_of(m_keyframes.begin(), m_keyframes.end(),
                       [time](const Keyframe& kf) {
                         return nearly_equal(kf.time(), time); 
                        });
}

void anim::Channel::delete_keyframe(size_t index)
{
    if (index < 0 || index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    m_keyframes.erase(m_keyframes.begin() + index);
}

const Keyframe &anim::Channel::keyframe(size_t index) const
{
    if (index < 0 || index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    return m_keyframes[index];
}

const Keyframe &anim::Channel::prev_keyframe(double time) const
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

const Keyframe &anim::Channel::next_keyframe(double time) const
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

const Keyframe &anim::Channel::closest_keyframe(double time) const
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

void anim::Channel::update_keyframe(size_t index, const Keyframe& keyframe)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    // To ensure sorting and handle updates are correct, remove the old one and insert the new one.
    // insert_keyframe will handle finding the correct sorted position and calling update_local_handles.
    Keyframe kf_copy = keyframe; 
    m_keyframes.erase(m_keyframes.begin() + index);
    insert_keyframe(std::move(kf_copy));
}

void anim::Channel::set_keyframe_time(size_t index, double new_time)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    
    bool has_prev = (it != m_keyframes.begin());
    bool has_next = (it + 1 != m_keyframes.end());
    
    if (!has_prev && !has_next) { // no neighbors, no clamping
        it->position.time = new_time;
    } else if (!has_prev) { // if no previous keyframe, clamp to next
        auto next_it = it + 1;
        it->position.time = std::min(new_time, next_it->time());
    } else if (!has_next) { // if no next keyframe, clamp to previous
        auto prev_it = it - 1;
        it->position.time = std::max(new_time, prev_it->time());
    } else {
        auto prev_it = it - 1;
        auto next_it = it + 1;
        it->position.time = std::clamp(new_time, prev_it->time(), next_it->time());
    }
    update_local_handles(it);
}
        
void anim::Channel::set_keyframe_value(size_t index, double value)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->position.value = value;
    update_local_handles(it);
}

void anim::Channel::set_keyframe_in_handle(size_t index, const Point& in_handle)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->in_handle = in_handle;
    update_local_handles(it, false);
}

void anim::Channel::set_keyframe_out_handle(size_t index, const Point& out_handle)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->out_handle = out_handle;
    update_local_handles(it);
}

void anim::Channel::set_keyframe_function(size_t index, Function function)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->function = function;
    update_local_handles(it);
}

void anim::Channel::set_keyframe_handle_mode(size_t index, HandleMode handle_mode)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->handle_mode = handle_mode;
    update_local_handles(it);
}

double Channel::evaluate(double time) const {
    if (m_keyframes.empty()) {
        return 0.0;
    }
    
    if (m_keyframes.size() == 1) {
        return m_keyframes[0].value();
    }
    
    if (time <= m_keyframes.front().time()) {
        return m_keyframes.front().value();
    }
    
    if (time >= m_keyframes.back().time()) {
        return m_keyframes.back().value();
    }
    
    // Find the keyframes that bracket the requested time
    auto upper = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), time,
                                 [](double t, const Keyframe& kf) {
                                     return t < kf.time();
                                 });
    auto lower = upper - 1;
    
    const Keyframe& start_kf = *lower;
    const Keyframe& end_kf = *upper;
    
    if (start_kf.function == Function::linear) {
        double t = (time - start_kf.time()) / (end_kf.time() - start_kf.time());
        return start_kf.value() + t * (end_kf.value() - start_kf.value());
    } else if (start_kf.function == Function::constant) {
        return start_kf.value();
    }
    
    double start_time = start_kf.time();
    double end_time = end_kf.time();
    double t = (time - start_time) / (end_time - start_time);
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
    
    for (int i = 0; i < num_samples; i++) {
        double time = start_time + i * step;
        result[i] = evaluate(time);
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
    
    int num_samples = static_cast<int>(std::ceil((end_time - start_time) * sample_rate)) + 1;
    return evaluate_range(start_time, end_time, num_samples);
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

size_t anim::Channel::num_samples(double sample_rate) const
{
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    
    if (m_keyframes.empty()) {
        return 0;
    }
    
    double duration = end_time() - start_time();
    return static_cast<size_t>(std::ceil(duration * sample_rate)) + 1; // +1 to include the start time
    
}

const Keyframe &anim::Channel::create_default_keyframe(const Point &position, Function function, HandleMode handle_mode)
{
    if (function == Function::constant || function == Function::linear) {
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
    Keyframe new_keyframe(position, function, HandleMode::smooth);
    update_handles(new_keyframe, prev_kf_ptr, next_kf_ptr, next_it != m_keyframes.end());
    std::cout << "New Keyframe, time: " << new_keyframe.time() << ", value: " << new_keyframe.value()
                << ", in_handle.time: " << new_keyframe.in_handle.time << ", in_handle.value: " << new_keyframe.in_handle.value
                << ", out_handle.time: " << new_keyframe.out_handle.time << ", out_handle.value: " << new_keyframe.out_handle.value
                << ", function: " << static_cast<int>(new_keyframe.function)
                << ", handle_mode: " << static_cast<int>(new_keyframe.handle_mode) << std::endl;


    new_keyframe.handle_mode = handle_mode; // set to requested mode, inser_keyframe will call update_local_handles
    return insert_keyframe(next_it, std::move(new_keyframe));
}

const Keyframe &anim::Channel::insert_keyframe(Keyframe&& keyframe, bool source_is_out_handle)
{
    // Find the insertion point for the new keyframe
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), keyframe.time(),
                               [](const Keyframe& kf, double t) {
                                   return kf.time() < t;
                               });
    return insert_keyframe(it, std::move(keyframe), source_is_out_handle);
}

const Keyframe &anim::Channel::insert_keyframe(KeyframeIt it, Keyframe&& keyframe, bool source_is_out_handle)
{
    // If the keyframe already exists at this time, replace it
    if (it != m_keyframes.end() && nearly_equal(it->time(), keyframe.time())) {
        *it = std::move(keyframe);
    }

    it = m_keyframes.insert(it, std::move(keyframe));
    update_local_handles(it, source_is_out_handle);

    auto& new_keyframe = *it; // reference to the newly inserted keyframe
    std::cout << "Updated Keyframe, time: " << new_keyframe.time() << ", value: " << new_keyframe.value()
                << ", in_handle.time: " << new_keyframe.in_handle.time << ", in_handle.value: " << new_keyframe.in_handle.value
                << ", out_handle.time: " << new_keyframe.out_handle.time << ", out_handle.value: " << new_keyframe.out_handle.value
                << ", function: " << static_cast<int>(new_keyframe.function)
                << ", handle_mode: " << static_cast<int>(new_keyframe.handle_mode) << std::endl;

    return *it;
}

void anim::Channel::update_local_handles(std::vector<anim::Keyframe>::iterator it, bool source_is_out_handle)
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
    update_handles(*it, prev_it, next_it, source_is_out_handle);
}

void anim::Channel::update_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr, bool source_is_out_handle)
{
    if (keyframe.function == Function::linear || keyframe.function == Function::constant) {
        return;
    }

    if (keyframe.handle_mode == HandleMode::flat) {
        calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::smooth) {
        calculate_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::aligned) {
        enforce_aligned_handles(keyframe, source_is_out_handle);
        // constrain_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::free) {
        constrain_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else {
        throw std::invalid_argument("Unknown handle type");
    }
}


} // namespace anim
