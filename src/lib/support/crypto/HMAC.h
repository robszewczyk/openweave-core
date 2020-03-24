/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file defines a template object for doing keyed-hash
 *      message authentication code (HMAC) and specialized objects
 *      for HMAC with SHA-1 and SHA-256.
 *
 */

#ifndef HMAC_H_
#define HMAC_H_

#include "WeaveCrypto.h"
#include "HashAlgos.h"
#include <Weave/Support/ASN1.h>
#include <Weave/Core/WeaveTLV.h>

namespace nl {
namespace Weave {
namespace Crypto {

using nl::Weave::ASN1::OID;
using nl::Weave::TLV::TLVReader;
using nl::Weave::TLV::TLVWriter;

template <class H> class NL_DLL_EXPORT HMAC
{
public:
    enum
    {
        kDigestLength = H::kHashLength
    };

    HMAC(void);
    ~HMAC(void);

    void Begin(const uint8_t * keyData, uint16_t keyLen);
    void AddData(const uint8_t * msgData, uint16_t dataLen);
#if WEAVE_WITH_OPENSSL
    void AddData(const BIGNUM & num);
#endif
    void Finish(uint8_t * hashBuf);
    void Reset(void);

private:
    enum
    {
        kBlockLength = H::kBlockLength
    };

    H mHash;
    uint8_t mKey[kBlockLength];
    uint16_t mKeyLen;
};

typedef HMAC<Platform::Security::SHA1> HMACSHA1;

typedef HMAC<Platform::Security::SHA256> HMACSHA256;

class EncodedHMACSignature
{
public:
    enum
    {
        kMaxValueLength = HMACSHA256::kDigestLength
    };

    uint8_t * Sig;
    uint8_t Len;

    bool IsEqual(const EncodedHMACSignature & other) const;

    WEAVE_ERROR ReadSignature(TLVReader & reader);
    WEAVE_ERROR WriteSignature(TLVWriter & writer, uint64_t tag) const;
};

inline WEAVE_ERROR EncodedHMACSignature::WriteSignature(TLVWriter & writer, uint64_t tag) const
{
    return writer.PutBytes(tag, Sig, Len);
}

// =============================================================
// Primary HMAC utility functions used by Weave security code.
// =============================================================

extern WEAVE_ERROR GenerateAndEncodeWeaveHMACSignature(OID sigAlgoOID, TLVWriter & writer, uint64_t tag, const uint8_t * data,
                                                       uint16_t dataLen, const uint8_t * key, uint16_t keyLen);

extern WEAVE_ERROR VerifyHMACSignature(OID sigAlgoOID, const uint8_t * data, uint16_t dataLen, const EncodedHMACSignature & sig,
                                       const uint8_t * key, uint16_t keyLen);

} // namespace Crypto
} // namespace Weave
} // namespace nl

#endif /* HMAC_H_ */
