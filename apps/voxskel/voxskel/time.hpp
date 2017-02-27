#pragma once

#include <cstdint>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>

namespace voxskel {

using std::chrono::duration_cast;

using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;

inline double ns2sec(uint64_t t) {
    return t * 0.000000001;
}

inline double ns2ms(uint64_t t) {
    return t * 0.000001;
}

inline double us2ms(uint64_t t) {
    return t * 0.001;
}

inline double us2ms(Microseconds t) {
    return us2ms(t.count());
}

inline double us2sec(uint64_t t) {
    return t * 0.000001;
}

inline double us2sec(Microseconds t) {
    return us2sec(t.count());
}

inline double ms2sec(double t) {
    return t * 0.001;
}

std::string getDateString();

uint64_t getMicroseconds();

class Timer {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    bool m_bDisplayTimeAtEnd;
    TimePoint m_StartPoint;

public:
    Timer(bool displayTimeAtEnd = false) :
        m_bDisplayTimeAtEnd(displayTimeAtEnd),
        m_StartPoint(Clock::now()) {
    }

    ~Timer() {
        if (m_bDisplayTimeAtEnd) {
            std::clog << "Ellapsed time = " << us2ms(getMicroEllapsedTime()) << " ms." << std::endl;
        }
    }

    Microseconds getMicroEllapsedTime() const {
        return std::chrono::duration_cast<Microseconds>(Clock::now() - m_StartPoint);
    }

    Nanoseconds getNanoEllapsedTime() const {
        return std::chrono::duration_cast<Nanoseconds>(Clock::now() - m_StartPoint);
    }

    template<typename DurationType>
    DurationType getEllapsedTime() const {
        return std::chrono::duration_cast<DurationType>(Clock::now() - m_StartPoint);
    }
};

template<typename Iterator>
inline const typename std::iterator_traits<Iterator>::value_type accumulateTime(Iterator&& begin, Iterator&& end) {
    using DurationType = typename std::iterator_traits<Iterator>::value_type;
    return std::accumulate(
                std::forward<Iterator>(begin),
                std::forward<Iterator>(end),
                DurationType{ 0 },
                std::plus<DurationType>());
}

template<typename DurationType>
inline const DurationType accumulateTime(const std::vector<DurationType>& times) {
    return accumulateTime(begin(times), end(times));
}

template<typename DurationType>
struct EvalCallTimeRAII {
    Timer m_Timer;
    DurationType& m_TotalTime;

    EvalCallTimeRAII(DurationType& totalTime): m_TotalTime(totalTime) {
    }

    ~EvalCallTimeRAII() {
        m_TotalTime += m_Timer.getEllapsedTime<DurationType>();
    }
};

template<typename DurationType, typename Functor, typename... Args>
inline auto evalCallTime(DurationType& totalTime,
             Functor&& f,
             Args&&... args) -> decltype(f(std::forward<Args>(args)...)) {
    EvalCallTimeRAII<DurationType> timer(totalTime);
    return f(std::forward<Args>(args)...);
}

}
