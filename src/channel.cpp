#include "anim/channel.hpp"
#include "anim/handle_utils.hpp"

namespace anim {


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
    Keyframe kf_copy = m_keyframes[index];
    m_keyframes.erase(m_keyframes.begin() + index);
    kf_copy.position.time = new_time;
    // insert_keyframe will re-sort and call update_local_handles,
    // which in turn calls update_handles to adjust handle times and positions if necessary.
    insert_keyframe(std::move(kf_copy));
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
    // update_local_handles will call update_handles, which will respect the current
    // handle_mode. If mode is e.g. 'aligned', it will enforce alignment.
    // If mode is 'smooth' or 'flat', this manually set handle might be overridden.
    update_local_handles(it);
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
    // update_local_handles will call update_handles. If function is linear/constant,
    // update_handles might do nothing for handles, which is correct.
    update_local_handles(it);
}

void anim::Channel::set_keyframe_handle_mode(size_t index, HandleMode handle_mode)
{
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    auto it = m_keyframes.begin() + index;
    it->handle_mode = handle_mode;
    // Changing handle_mode will cause update_handles (via update_local_handles)
    // to recalculate or re-constrain handles accordingly.
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

const Keyframe &anim::Channel::insert_keyframe(Keyframe&& keyframe)
{
    // Find the insertion point for the new keyframe
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), keyframe.time(),
                               [](const Keyframe& kf, double t) {
                                   return kf.time() < t;
                               });
    
    // If the keyframe already exists at this time, replace it
    if (it != m_keyframes.end() && nearly_equal(it->time(), keyframe.time())) {
        *it = std::move(keyframe);
    }

    it = m_keyframes.insert(it, std::move(keyframe));
    update_local_handles(it);
    return *it;
}

void anim::Channel::update_local_handles(std::vector<anim::Keyframe>::iterator it)
{
    Keyframe* prev_it = nullptr;
    if (it != m_keyframes.begin()) {
        prev_it = &*(it - 1);
        update_prev_out_handle(*prev_it, *it);
    }
    
    Keyframe* next_it = nullptr;
    if (it + 1 != m_keyframes.end()) {
        next_it = &*(it + 1);
        update_next_in_handle(*next_it, *it);
    }
    update_handles(*it, prev_it, next_it);
}

void anim::Channel::update_prev_out_handle(Keyframe &keyframe, const Keyframe &next_keyframe)
{
    if (keyframe.function == Function::linear || keyframe.function == Function::constant) {
        return;
    }

    if (keyframe.handle_mode == HandleMode::flat) { // For flat handles, set the out handle to the keyframe position
        keyframe.out_handle.time = std::clamp(keyframe.out_handle.time, keyframe.position.time, next_keyframe.position.time);
    } else {
        constrain_in_handle_time(keyframe, next_keyframe);
    } 
}

void anim::Channel::update_next_in_handle(Keyframe &keyframe, const Keyframe &prev_keyframe)
{
    if (prev_keyframe.function == Function::linear || prev_keyframe.function == Function::constant) {
        return;
    }

    if (keyframe.handle_mode == HandleMode::flat) { // For flat handles, set the in handle to the keyframe position
        keyframe.in_handle.time = std::clamp(keyframe.in_handle.time, prev_keyframe.position.time, keyframe.position.time);
    } else {
        constrain_out_handle_time(keyframe, prev_keyframe);
    }
}

void anim::Channel::update_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr)
{
    if (keyframe.function == Function::linear || keyframe.function == Function::constant) {
        return;
    }

    if (keyframe.handle_mode == HandleMode::flat) {
        calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::smooth) {
        calculate_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::aligned) {
        enforce_aligned_handles(keyframe);
        constrain_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_mode == HandleMode::free) {
        constrain_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else {
        throw std::invalid_argument("Unknown handle type");
    }
}


} // namespace anim
