/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/bitcoin/math/ec_keys.hpp>

#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <secp256k1.h>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/math/secp256k1_initializer.hpp>
#include <bitcoin/bitcoin/utility/assert.hpp>
#include <bitcoin/bitcoin/utility/endian.hpp>

namespace libbitcoin {

// ec_public

ec_public::ec_public(const ec_compressed& point)
  : point_(point.begin(), point.end())
{
}

ec_public::ec_public(const ec_uncompressed& point)
    : point_(point.begin(), point.end())
{
}

ec_public::operator const data_chunk&() const
{
    return point_;
}

const data_chunk& ec_public::data() const
{
    return point_;
}

const bool ec_public::is_compressed() const
{
    return point_.size() == ec_compressed_size;
}

// functions

bool is_point(data_slice data)
{
    const auto size = data.size();
    const auto first = data.data()[0];
    const auto uncompressed_point_byte = first == 0x04;
    const auto compressed_point_byte = first == 0x02 || first == 0x03;
    return 
        (size == ec_compressed_size && compressed_point_byte) ||
        (size == ec_uncompressed_size && uncompressed_point_byte);
}

static bool decompress(ec_uncompressed& out, const ec_compressed& point)
{
    // NOTE: a later version secp256k1 could use secp256k1_ec_pubkey_serialize

    int out_size = ec_compressed_size;
    std::copy(point.begin(), point.end(), out.begin());
    const auto signing_context = signing.context();
    if (secp256k1_ec_pubkey_decompress(signing_context, out.data(), &out_size)
        == 1)
    {
        BITCOIN_ASSERT_MSG(ec_uncompressed_size == static_cast<size_t>(out_size),
            "secp256k1_ec_pubkey_decompress returned invalid size");
        return true;
    }

    return false;
}

bool decompress(ec_uncompressed& out, const ec_public& point)
{
    const data_chunk& data = point;
    if (point.is_compressed())
        return decompress(out, to_array<ec_compressed_size>(data));

    out = to_array<ec_uncompressed_size>(data);
    return true;
}

bool secret_to_public(ec_compressed& out, const ec_secret& secret)
{
    int out_size = ec_compressed_size;
    static constexpr int compression = 1;
    const auto signing_context = signing.context();
    if (secp256k1_ec_pubkey_create(signing_context, out.data(), &out_size,
        secret.data(), compression) == 1)
    {
        BITCOIN_ASSERT_MSG(
            ec_compressed_size == static_cast<size_t>(out_size),
            "secp256k1_ec_pubkey_create returned invalid size");
        return true;
    }

    return false;
}

bool secret_to_public(ec_uncompressed& out, const ec_secret& secret)
{
    int out_size = ec_uncompressed_size;
    static constexpr int compression = 0;
    const auto signing_context = signing.context();
    if (secp256k1_ec_pubkey_create(signing_context, out.data(), &out_size,
        secret.data(), compression) == 1)
    {
        BITCOIN_ASSERT_MSG(
            ec_uncompressed_size == static_cast<size_t>(out_size),
            "secp256k1_ec_pubkey_create returned invalid size");
        return true;
    }

    return false;
}

bool verify(const ec_secret& private_key)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_seckey_verify(verification_context, private_key.data())
        == 1;
}

bool verify(const ec_public& point)
{
    const data_chunk& data = point;
    const auto verification_context = verification.context();
    return secp256k1_ec_pubkey_verify(verification_context,
        data.data(), static_cast<uint32_t>(data.size())) == 1;
}

bool sign(endorsement& out, const ec_secret& secret, const hash_digest& hash)
{
    int out_size = max_endorsement_size;
    out.resize(max_endorsement_size);
    const auto signing_context = signing.context();
    if (secp256k1_ecdsa_sign(signing_context, hash.data(), out.data(),
        &out_size, secret.data(), secp256k1_nonce_function_rfc6979, nullptr)
        != 1)
    {
        BITCOIN_ASSERT_MSG(false, "secp256k1_ecdsa_sign failed");
        ////out.clear();
        return false;
    };

    out.resize(out_size);
    return true;
}

bool verify_signature(const ec_public& point, const hash_digest& hash,
    const endorsement& signature)
{
    const data_chunk& data = point;
    auto signing_context = verification.context();
    auto result = secp256k1_ecdsa_verify(signing_context, hash.data(),
        signature.data(), static_cast<uint32_t>(signature.size()),
        data.data(), static_cast<uint32_t>(data.size()));

    BITCOIN_ASSERT_MSG(result >= 0, "secp256k1_ecdsa_verify failed");
    return result == 1;
}

bool sign_compact(compact_signature& out_signature, uint8_t& out_recovery_id,
    const ec_secret& secret, const hash_digest& hash)
{
    int recid;
    const auto signing_context = signing.context();
    if (secp256k1_ecdsa_sign_compact(signing_context, hash.data(),
        out_signature.data(), secret.data(), secp256k1_nonce_function_rfc6979,
        nullptr, &recid) != 1)
    {
        BITCOIN_ASSERT_MSG(false, "secp256k1_ecdsa_sign_compact failed");
        return false;
    }

    if (recid < 0 || recid > 3)
    {
        BITCOIN_ASSERT_MSG(false, "secp256k1_ecdsa_sign_compact invalid");
        return false;
    }

    out_recovery_id = static_cast<uint8_t>(recid);
    return true;
}

bool recover_public(ec_compressed& out, const compact_signature& signature,
    uint8_t recovery_id, const hash_digest& hash)
{
    if (recovery_id > 3)
        return false;

    int out_size = ec_compressed_size;
    static constexpr int compression = 1;
    const auto recid = static_cast<int>(recovery_id);
    const auto verification_context = verification.context();
    if (secp256k1_ecdsa_recover_compact(verification_context, hash.data(),
        signature.data(), out.data(), &out_size, compression, recid) == 1)
    {
        BITCOIN_ASSERT_MSG(
            ec_compressed_size == static_cast<size_t>(out_size),
            "secp256k1_ecdsa_recover_compact returned invalid size");
        return true;
    }

    return false;
}

bool recover_public(ec_uncompressed& out, const compact_signature& signature,
    uint8_t recovery_id, const hash_digest& hash)
{
    if (recovery_id > 3)
        return false;

    int out_size = ec_uncompressed_size;
    static constexpr int compression = 0;
    const auto recid = static_cast<int>(recovery_id);
    const auto verification_context = verification.context();
    if (secp256k1_ecdsa_recover_compact(verification_context, hash.data(),
        signature.data(), out.data(), &out_size, compression, recid) == 1)
    {
        BITCOIN_ASSERT_MSG(
            ec_uncompressed_size == static_cast<size_t>(out_size),
            "secp256k1_ecdsa_recover_compact returned invalid size");
        return true;
    }

    return false;
}

bool ec_add(ec_compressed& point, const ec_secret& secret)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_pubkey_tweak_add(verification_context, point.data(),
        static_cast<uint32_t>(ec_compressed_size), secret.data()) == 1;
}

bool ec_add(ec_uncompressed& point, const ec_secret& secret)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_pubkey_tweak_add(verification_context, point.data(),
        static_cast<uint32_t>(ec_uncompressed_size), secret.data()) == 1;
}

bool ec_add(ec_secret& left, const ec_secret& right)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_privkey_tweak_add(verification_context, left.data(),
        right.data()) == 1;
}

bool ec_multiply(ec_compressed& point, const ec_secret& secret)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_pubkey_tweak_mul(verification_context, point.data(),
        static_cast<uint32_t>(ec_compressed_size), secret.data()) == 1;
}

bool ec_multiply(ec_uncompressed& point, const ec_secret& secret)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_pubkey_tweak_mul(verification_context, point.data(),
        static_cast<uint32_t>(ec_uncompressed_size), secret.data()) == 1;
}

bool ec_multiply(ec_secret& left, const ec_secret& right)
{
    const auto verification_context = verification.context();
    return secp256k1_ec_privkey_tweak_mul(verification_context, left.data(),
        right.data()) == 1;
}

} // namespace libbitcoin
