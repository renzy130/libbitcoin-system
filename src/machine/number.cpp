/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/system/machine/number.hpp>

#include <cstdint>
#include <cstdlib>
#include <bitcoin/system/assert.hpp>
#include <bitcoin/system/chain/chain.hpp>
#include <bitcoin/system/constants.hpp>
#include <bitcoin/system/data/data.hpp>
#include <bitcoin/system/math/math.hpp>
#include <bitcoin/system/serial/serial.hpp>

namespace libbitcoin {
namespace system {
namespace machine {

using namespace bc::system::chain;

number::number()
  : number(numbers::positive_0)
{
}

number::number(int64_t value)
  : value_(value)
{
}

// Properties
//-----------------------------------------------------------------------------

// The data is interpreted as little-endian.
bool number::set_data(const data_chunk& data, size_t max_size)
{
    // Bitcoin defined max script number sizes are 4 and 5 bytes.
    // The negation technique below requires that the value_ sign bit is never
    // set following conversion from little endian data, so guard here.
    BITCOIN_ASSERT_MSG(max_size < sizeof(value_), "invalid number size");

    if (data.size() > max_size)
        return false;

    value_ = 0;

    if (data.empty())
        return true;

    value_ = from_little_endian<int64_t>(data);
    const auto negative_bit_set = get_left(data.back());

    // Clear the negative bit and negate the value.
    // The bit is the leftmost of the bits decoded (offset from right).
    if (negative_bit_set)
        value_ = -set_right(value_, sub1(to_bits(data.size())), false);

    return true;
}

// The result is little-endian.
data_chunk number::data() const
{
    if (is_zero(value_))
        return {};

    auto data = to_little_endian_chunk(absolute(value_));
    const auto negative_bit_set = get_left(data.back());

    // Push a 0x80 byte that will be popped off when converting to an integral.
    if (negative_bit_set && is_negative())
        data.push_back(numbers::negative_sign);

    // Push a 0x00 byte to make the most significant byte non-negative again.
    else if (negative_bit_set)
        data.push_back(numbers::positive_0);

    // Set the negative bit, since it will be subtracted and interpreted as
    // a negative when converting to an integral.
    else if (is_negative())
        set_left_into(data.back());

    return data;
}

int32_t number::int32() const
{
    return limit<int32_t>(value_);
}

int64_t number::int64() const
{
    return value_;
}

// Stack Helpers
//-----------------------------------------------------------------------------

bool number::is_true() const
{
    return !is_zero(value_);
}

bool number::is_false() const
{
    return is_zero(value_);
}

bool number::is_negative() const
{
    return system::is_negative(value_);
}

// Operators
//-----------------------------------------------------------------------------

bool number::operator>(int64_t value) const
{
    return value_ > value;
}

bool number::operator<(int64_t value) const
{
    return value_ < value;
}

bool number::operator>=(int64_t value) const
{
    return value_ >= value;
}

bool number::operator<=(int64_t value) const
{
    return value_ <= value;
}

bool number::operator==(int64_t value) const
{
    return value_ == value;
}

bool number::operator!=(int64_t value) const
{
    return value_ != value;
}

bool number::operator>(const number& other) const
{
    return operator>(other.value_);
}

bool number::operator<(const number& other) const
{
    return operator<(other.value_);
}

bool number::operator>=(const number& other) const
{
    return operator>=(other.value_);
}

bool number::operator<=(const number& other) const
{
    return operator<=(other.value_);
}

bool number::operator==(const number& other) const
{
    return operator==(other.value_);
}

bool number::operator!=(const number& other) const
{
    return operator!=(other.value_);
}

number number::operator+(int64_t value) const
{
    BITCOIN_ASSERT_MSG(is_zero(value) ||
        (value > 0 && value_ <= max_int64 - value) ||
        (value < 0 && value_ >= min_int64 - value), "overflow");

    return number(value_ + value);
}

number number::operator-(int64_t value) const
{
    BITCOIN_ASSERT_MSG(is_zero(value) ||
        (value > 0 && value_ >= min_int64 + value) ||
        (value < 0 && value_ <= max_int64 + value), "underflow");

    return number(value_ - value);
}

number number::operator+(const number& other) const
{
    return operator+(other.value_);
}

number number::operator-(const number& other) const
{
    return operator-(other.value_);
}

number number::operator+() const
{
    return *this;
}

number number::operator-() const
{
    BITCOIN_ASSERT_MSG(value_ != min_int64, "out of range");

    return number(-value_);
}

number& number::operator+=(const number& other)
{
    return operator+=(other.value_);
}

number& number::operator-=(const number& other)
{
    return operator-=(other.value_);
}

number& number::operator+=(int64_t value)
{
    BITCOIN_ASSERT_MSG(is_zero(value) ||
        (value > 0 && value_ <= max_int64 - value) ||
        (value < 0 && value_ >= min_int64 - value), "overflow");

    value_ += value;
    return *this;
}

number& number::operator-=(int64_t value)
{
    BITCOIN_ASSERT_MSG(is_zero(value) ||
        (value > 0 && value_ >= min_int64 + value) ||
        (value < 0 && value_ <= max_int64 + value), "underflow");

    value_ -= value;
    return *this;
}

} // namespace machine
} // namespace system
} // namespace libbitcoin
