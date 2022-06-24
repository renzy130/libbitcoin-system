/**
 * Copyright (c) 2011-2022 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_SYSTEM_BOOST_HPP
#define LIBBITCOIN_SYSTEM_BOOST_HPP

// "By default, enable_current_exception and enable_error_info are integrated
// directly in the throw_exception function. Defining BOOST_EXCEPTION_DISABLE
// disables this integration."
// www.boost.org/doc/libs/1_78_0/libs/exception/doc/configuration_macros.html
// This does not prevent interfaces that are documented to throw from doing so.
// It only prevents boost from internally wrapping the exception object with
// another class (in boost/throw_exception.hpp). Nearly all instances of the
// internal boost exceptions affecting this library occur in streambuf and are
// caught and presumed discarded in std::istream (standards allow propagation).
// See more comments in streamers.hpp on streams that may throw exceptions.
// Must be set on the command line to ensure it is captured by all includes.
////#define BOOST_EXCEPTION_DISABLE

// std::auto_ptr is deprecated in C++11 and removed in C++17:
// en.cppreference.com/w/cpp/memory/auto_ptr
// boost.locale exposes it to the API, so cannot remove it until C++17:
// github.com/boostorg/locale/issues/27#issuecomment-414932853
// Must use BOOST_LOCALE_HIDE_AUTO_PTR to hide the warnings.
#define BOOST_LOCALE_HIDE_AUTO_PTR

// Apply any warning suppressions to boost.
// Any boost includes within headers will not benefit from suppression, as the
// warnings are included by define.hpp which follows boost includes.
#include <bitcoin/system/define.hpp>

// Include boost in cpp files only from here, so exception disable works.
// Avoid use in header includes due to warning repetition (boost/format.hpp).
#include <boost/algorithm/string.hpp>
////#include <boost/format.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/json.hpp>
#include <boost/locale.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#ifndef HAVE_STD_RANGES
#include <boost/range/adaptor/reversed.hpp>
#endif

#endif
