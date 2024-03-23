/*
 * SPDX-FileCopyrightText: 2023 Dominik Wójt <domin144@o2.pl>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <iomanip>
#include <cstdint>

std::uint32_t compute_pi_times_2_to_30_low()
{
    /* result is pi * 2^32 / 4, which is surface of quarter of a circle with
     * r = 2^32 divided by 2^32 */
    std::uint64_t result_low = 0;
    std::uint64_t r = 0x100000000ull;
    std::uint64_t x_low = r;
    x_low--;
    for (std::uint64_t y = 1; y <= r; ++y)
    {
        while (x_low * x_low > r * r - y * y)
            --x_low;
        result_low += x_low;
    }
    return result_low >> 32u;
}

std::uint32_t compute_pi_times_2_to_30_high()
{
    /* result is pi * 2^32 / 4, which is surface of quarter of a circle with
     * r = 2^32 divided by 2^32 */
    std::uint64_t r = 0x100000000ull;
    std::uint64_t result_high = 0;
    std::uint64_t x_high = r;
    for (std::uint64_t y = 0; y < r; ++y)
    {
        while ((x_high - 1u) * (x_high - 1u) - 1u >= r * r - y * y - 1u)
            --x_high;
        result_high += x_high;
    }
    return (result_high + 0xfffffffful) >> 32u;
}

int main()
{
    std::uint32_t pi_times_2_to_30 = compute_pi_times_2_to_30_low();
    std::uint32_t two_to_30 = static_cast<std::uint32_t>(1) << 30;
    std::uint32_t pi_integer = pi_times_2_to_30 / two_to_30;
    std::uint32_t pi_fraction =
            pi_times_2_to_30 - pi_integer * two_to_30;
    std::uint32_t pi_fraction_1000000000 =
            static_cast<std::uint32_t>(
                static_cast<std::uint64_t>(pi_fraction)
                * 1000000000
                / two_to_30);
    std::cout
            << "pi ~= " << pi_integer << '.'
            << std::setw(9)
            << std::setfill('0')
            << pi_fraction_1000000000 << std::endl;
    return 0;
}
