/*
 * SPDX-FileCopyrightText: 2016 Dominik Wójt <domin144@o2.pl>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <iostream>
#include <cstdint>

void my_assert(const bool condition, const char *const description)
{
    if(!condition)
        throw std::runtime_error(description);
}

template<typename TRatio>
struct Ratio_to_double
{
    using Ratio = TRatio;
    static constexpr double value = double(Ratio::num) / Ratio::den;
};

class Timer
{
private:
    using Clock = std::chrono::high_resolution_clock;

private:
    Clock::time_point m_start_point;
    std::string m_message;

public:
    Timer(const std::string &message) :
        m_start_point(Clock::now()),
        m_message(message)
    { }

    ~Timer()
    {
        const Clock::duration time_elapsed = Clock::now() - m_start_point;
        const double time_in_ms =
                time_elapsed.count()
                * Ratio_to_double<
                    std::ratio_divide<
                        Clock::duration::period,
                        std::milli> >::value;
        std::cerr
                << "Timer \"" << m_message << "\" : "
                << time_in_ms << " ms\n";
    }
};

using Index = std::int_fast64_t;

template<Index base, typename TNumber>
TNumber cached_power(TNumber exponent)
{
    constexpr TNumber b = base;
    static const TNumber cache[] =
    {
        1,
        b,
        b * b,
        b * b * b,
        b * b * b * b,
        b * b * b * b * b,
        b * b * b * b * b * b,
        b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b * b * b * b * b * b,
        b * b * b * b * b * b * b * b * b * b * b * b * b * b * b * b
    };

    const TNumber cache_size = sizeof(cache)/sizeof(cache[0]);
    if(exponent < cache_size)
        return cache[exponent];
    else
        return power(b, exponent);
}

#endif /* UTILS_H */
