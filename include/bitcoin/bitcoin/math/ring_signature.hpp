/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_RING_SIGNATURE_HPP
#define LIBBITCOIN_RING_SIGNATURE_HPP

#include <bitcoin/bitcoin/math/elliptic_curve.hpp>

namespace libbitcoin {

/**
 * key_rings is a collection of rings of public keys.
 * Each ring contains several public keys.
 * To create a valid signature, you must use at least one valid private
 * key from each ring.
 * That is given three rings [{A, B, C}, {D, E, F}, {X, Y}], then to
 * create a valid signature, we must use a private key from each of those
 * sets.
 * For example A and E and X. We can summarize this operation as:
 * (A or B or C) and (D or E or F) and (X or Y)
 */
typedef std::vector<point_list> key_rings;

/**
 * A borromean ring signature.
 *
 * theta = {e_0, s_{i_j} : 0 <= i <= n, 0 <= j <= m_i}
 */
struct ring_signature
{
    typedef std::vector<secret_list> s_values_type;

    ec_secret e;
    s_values_type s;
};

/**
 * Create a borromean ring signature.
 * There must exist a valid signing key for each ring of public keys.
 * For example given a ring of [{A, B, C}, {D, E, F}, {X, Y}] then we
 * must have a set of keys that satisfies this constraint:
 * (A or B or C) and (D or E or F) and (X or Y)
 *
 * @param[in]  out      The new signature.
 * @param[in]  secrets  Secret signing keys. There should be at least one key
 *                      from each ring.
 * @param[in]  rings    The rings each with N_i public keys.
 * @param[in]  message  The message data to sign.
 * @param[in]  seed     Randomizing seed data.
 * @return false if the signing operation fails.
 */
BC_API bool sign(ring_signature& out, const secret_list& secrets,
    const key_rings& rings, const data_slice message, const data_slice seed);

/**
 * Verify a borromean ring signature.
 *
 * @param[in]  rings        The rings each with N_i public keys.
 * @param[in]  message      The message data to verify.
 * @param[in]  signature    Signature.
 * @return false if the verify operation fails.
 */
BC_API bool verify(const key_rings& rings, const data_slice message,
    const ring_signature& signature);

} // namespace libbitcoin

#endif

