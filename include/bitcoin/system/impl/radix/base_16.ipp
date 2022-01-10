/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_SYSTEM_RADIX_BASE_16_IPP
#define LIBBITCOIN_SYSTEM_RADIX_BASE_16_IPP

#include <algorithm>
#include <string>
#include <bitcoin/system/constants.hpp>
#include <bitcoin/system/constraints.hpp>
#include <bitcoin/system/data/data.hpp>

namespace libbitcoin {
namespace system {

inline bool is_between(uint8_t value, uint8_t low, uint8_t high) noexcept
{
    return low <= value && value <= high;
}

inline uint8_t from_base16_characters(char high, char low) noexcept
{
    const auto from_base16_digit = [](char character) noexcept
    {
        if (is_between(character, 'A', 'F'))
            return character - 'A' + '\xA';

        if (is_between(character, 'a', 'f'))
            return character - 'a' + '\xa';

        return character - '0' + '\x0';
    };

    return (from_base16_digit(high) << to_half(byte_bits)) |
        from_base16_digit(low);
}

template <size_t Size>
bool decode_base16(data_array<Size>& out, const std::string& in) noexcept
{
    if (in.size() != Size * octet_width)
        return false;

    if (!std::all_of(in.begin(), in.end(), is_base16))
        return false;

    auto data = out.begin();

    for (auto digit = in.begin(); digit != in.end();)
    {
        const auto hi = *digit++;
        const auto lo = *digit++;
        *data++ = from_base16_characters(hi, lo);
    }

    return true;
}

template <size_t Size>
bool decode_hash(data_array<Size>& out, const std::string& in) noexcept
{
    if (in.size() != Size * octet_width)
        return false;

    if (!std::all_of(in.begin(), in.end(), is_base16))
        return false;

    auto data = out.begin();

    for (auto digit = in.rbegin(); digit != in.rend();)
    {
        const auto lo = *digit++;
        const auto hi = *digit++;
        *data++ = from_base16_characters(hi, lo);
    }

    return true;
}

template <size_t Size, if_odd<Size>>
std::string base16_string(const char(&string)[Size]) noexcept
{
    return to_string(base16_chunk(string));
}

template <size_t Size, if_odd<Size>>
data_chunk base16_chunk(const char(&string)[Size]) noexcept
{
    data_chunk out;
    decode_base16(out, string);
    return out;
}

template <size_t Size, if_odd<Size>>
data_array<to_half(sub1(Size))> base16_array(const char(&string)[Size]) noexcept
{
    data_array<to_half(sub1(Size))> out;
    if (!decode_base16(out, string))
        out.fill(0);

    return out;
}

template <size_t Size, if_odd<Size>>
data_array<to_half(sub1(Size))> base16_hash(const char(&string)[Size]) noexcept
{
    data_array<to_half(sub1(Size))> out;
    if (!decode_hash(out, string))
        out.fill(0);

    return out;
}

} // namespace system
} // namespace libbitcoin

#endif
