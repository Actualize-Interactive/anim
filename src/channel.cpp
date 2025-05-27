#include "anim/channel.hpp"
#include "anim/handle_utils.hpp"
#include "channel.hpp"

namespace anim {

static bool nearly_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
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
    return (std::abs(prev_it->time() - time) < std::abs(it->time() - time)) ? *prev_it : *it;
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
    
    if (start_kf.function_type == FunctionType::linear) {
        double t = (time - start_kf.time()) / (end_kf.time() - start_kf.time());
        return start_kf.value() + t * (end_kf.value() - start_kf.value());
    } else if (start_kf.function_type == FunctionType::constant) {
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
    if (keyframe.function_type == FunctionType::linear || keyframe.function_type == FunctionType::constant) {
        return;
    }

    if (keyframe.handle_type == HandleType::flat) { // For flat handles, set the out handle to the keyframe position
        keyframe.out_handle.time = std::clamp(keyframe.out_handle.time, keyframe.position.time, next_keyframe.position.time);
    } else {
        constrain_in_handle_time(keyframe, next_keyframe);
    } 
}

void anim::Channel::update_next_in_handle(Keyframe &keyframe, const Keyframe &prev_keyframe)
{
    if (prev_keyframe.function_type == FunctionType::linear || prev_keyframe.function_type == FunctionType::constant) {
        return;
    }

    if (keyframe.handle_type == HandleType::flat) { // For flat handles, set the in handle to the keyframe position
        keyframe.in_handle.time = std::clamp(keyframe.in_handle.time, prev_keyframe.position.time, keyframe.position.time);
    } else {
        constrain_out_handle_time(keyframe, prev_keyframe);
    }
}

void anim::Channel::update_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr)
{
    if (keyframe.function_type == FunctionType::linear || keyframe.function_type == FunctionType::constant) {
        return;
    }

    if (keyframe.handle_type == HandleType::flat) {
        get_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_type == HandleType::smooth) {
        get_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_type == HandleType::aligned) {
        get_aligned_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else if (keyframe.handle_type == HandleType::free) {
        get_free_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    } else {
        throw std::invalid_argument("Unknown handle type");
    }
}


} // namespace anim
