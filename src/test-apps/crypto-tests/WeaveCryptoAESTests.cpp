/*
 *
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
 *      This file implements interface to Weave Crypto AES Tests library.
 *
 */

#include <string.h>

#include <nlunit-test.h>

#include <Weave/Support/crypto/AESBlockCipher.h>
#include <Weave/Support/crypto/CTRMode.h>

#include "WeaveCryptoTests.h"

using namespace nl::Weave::Crypto;
using namespace nl::Weave::Platform::Security;

#define TEXT_BUFFER_LENGHT 100

bool AES128CTRMode_DoTest(const uint8_t * key, const uint8_t * ctr, const uint8_t * plainText, size_t plainTextLen,
                          const uint8_t * expectedCipherText)
{
    uint8_t cipherText[TEXT_BUFFER_LENGHT]         = { 0 };
    uint8_t decryptedPlainText[TEXT_BUFFER_LENGHT] = { 0 };
    bool res                                       = true;

    for (size_t chunkSize = 1; chunkSize <= plainTextLen; chunkSize++)
    {
        AES128CTRMode aes128CTR;

        aes128CTR.SetKey(key);
        aes128CTR.SetCounter(ctr);

        for (size_t chunkStart = 0; chunkStart < plainTextLen; chunkStart += chunkSize)
        {
            uint16_t inLen = plainTextLen - chunkStart;
            if (inLen > chunkSize)
                inLen = chunkSize;
            aes128CTR.EncryptData(plainText + chunkStart, inLen, cipherText + chunkStart);
        }

        aes128CTR.Reset();

        if (memcmp(cipherText, expectedCipherText, plainTextLen) != 0)
        {
            res = false;
            break;
        }
    }

    {
        AES128CTRMode aes128CTR;

        aes128CTR.SetKey(key);
        aes128CTR.SetCounter(ctr);

        aes128CTR.EncryptData(cipherText, plainTextLen, decryptedPlainText);

        if (memcmp(decryptedPlainText, plainText, plainTextLen) != 0)
            res = false;
    }

    return res;
}

static void Check_AES128CTRMode_Test1(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #1 from RFC-3686.
    static uint8_t key[]       = { 0xAE, 0x68, 0x52, 0xF8, 0x12, 0x10, 0x67, 0xCC, 0x4B, 0xF7, 0xA5, 0x76, 0x55, 0x77, 0xF3, 0x9E };
    static uint8_t ctr[]       = { 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    static uint8_t plainText[] = { 0x53, 0x69, 0x6E, 0x67, 0x6C, 0x65, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x6D, 0x73, 0x67 };
    static uint8_t expectedCipherText[] = { 0xE4, 0x09, 0x5D, 0x4F, 0xB7, 0xA7, 0xB3, 0x79,
                                            0x2D, 0x61, 0x75, 0xA3, 0x26, 0x13, 0x11, 0xB8 };

    res = AES128CTRMode_DoTest(key, ctr, plainText, sizeof(plainText), expectedCipherText);

    // Invalid ciphertext generated by AES128CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

static void Check_AES128CTRMode_Test2(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #2 from RFC-3686.
    static uint8_t key[]       = { 0x7E, 0x24, 0x06, 0x78, 0x17, 0xFA, 0xE0, 0xD7, 0x43, 0xD6, 0xCE, 0x1F, 0x32, 0x53, 0x91, 0x63 };
    static uint8_t ctr[]       = { 0x00, 0x6C, 0xB6, 0xDB, 0xC0, 0x54, 0x3B, 0x59, 0xDA, 0x48, 0xD9, 0x0B, 0x00, 0x00, 0x00, 0x01 };
    static uint8_t plainText[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    static uint8_t expectedCipherText[] = { 0x51, 0x04, 0xA1, 0x06, 0x16, 0x8A, 0x72, 0xD9, 0x79, 0x0D, 0x41,
                                            0xEE, 0x8E, 0xDA, 0xD3, 0x88, 0xEB, 0x2E, 0x1E, 0xFC, 0x46, 0xDA,
                                            0x57, 0xC8, 0xFC, 0xE6, 0x30, 0xDF, 0x91, 0x41, 0xBE, 0x28 };

    res = AES128CTRMode_DoTest(key, ctr, plainText, sizeof(plainText), expectedCipherText);

    // Invalid ciphertext generated by AES128CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

static void Check_AES128CTRMode_Test3(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #3 from RFC-3686.
    static uint8_t key[]       = { 0x76, 0x91, 0xBE, 0x03, 0x5E, 0x50, 0x20, 0xA8, 0xAC, 0x6E, 0x61, 0x85, 0x29, 0xF9, 0xA0, 0xDC };
    static uint8_t ctr[]       = { 0x00, 0xE0, 0x01, 0x7B, 0x27, 0x77, 0x7F, 0x3F, 0x4A, 0x17, 0x86, 0xF0, 0x00, 0x00, 0x00, 0x01 };
    static uint8_t plainText[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                   0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                   0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23 };
    static uint8_t expectedCipherText[] = { 0xC1, 0xCF, 0x48, 0xA8, 0x9F, 0x2F, 0xFD, 0xD9, 0xCF, 0x46, 0x52, 0xE9,
                                            0xEF, 0xDB, 0x72, 0xD7, 0x45, 0x40, 0xA4, 0x2B, 0xDE, 0x6D, 0x78, 0x36,
                                            0xD5, 0x9A, 0x5C, 0xEA, 0xAE, 0xF3, 0x10, 0x53, 0x25, 0xB2, 0x07, 0x2F };

    res = AES128CTRMode_DoTest(key, ctr, plainText, sizeof(plainText), expectedCipherText);

    // Invalid ciphertext generated by AES128CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

static void Check_AES128CTRMode_Test4(nlTestSuite * inSuite, void * inContext)
{
    AES128CTRMode aes128CTR;
    bool res;

    static uint8_t key[]       = { 0x76, 0x91, 0xBE, 0x03, 0x5E, 0x50, 0x20, 0xA8, 0xAC, 0x6E, 0x61, 0x85, 0x29, 0xF9, 0xA0, 0xDC };
    static uint64_t nodeId     = 0x18B43000001E8687ULL;
    static uint32_t msgId      = 42;
    static uint8_t plainText[] = { 0xb1, 0x91, 0xa3, 0xfb, 0xa8, 0xba, 0x41, 0x43, 0xea, 0xa8, 0xd7, 0xbf, 0xeb, 0x46, 0xdd,
                                   0x0a, 0xd6, 0x53, 0xff, 0x5c, 0xcb, 0x78, 0x3e, 0xbc, 0x73, 0xdb, 0x19, 0xc7, 0xcc, 0x21,
                                   0xf6, 0x62, 0x31, 0x30, 0x0b, 0x8d, 0x26, 0x5b, 0x43, 0xc9, 0x1e, 0x92, 0xab, 0x09, 0xc9,
                                   0x60, 0x37, 0x64, 0x22, 0x54, 0x50, 0xe0, 0x29, 0x15, 0xd5, 0x23, 0x80, 0x9c, 0x13, 0xe4,
                                   0x57, 0x81, 0xaa, 0x5c, 0x1e, 0x19, 0xf5, 0x0f, 0x78, 0x1c, 0xa7, 0x40, 0x1c, 0xf4, 0x26,
                                   0xbb, 0x34, 0x86, 0x27, 0xa7, 0xae, 0x67, 0x42, 0xd5, 0x8d, 0x13, 0xe9, 0xd3, 0xed, 0x8f,
                                   0x67, 0xd4, 0x4f, 0x6a, 0x33, 0x90, 0xfd, 0xb8, 0x0f, 0x63 };
    static uint8_t expectedCipherText[] = {
        0x88, 0xF0, 0x69, 0xDA, 0x18, 0xBB, 0xE6, 0x25, 0x5E, 0x87, 0xF2, 0x9D, 0x4E, 0x2E, 0xF7, 0x40, 0xD7, 0x53, 0x02, 0xE9,
        0xF8, 0x3B, 0x68, 0x6D, 0x6D, 0x32, 0x66, 0xBD, 0x53, 0x04, 0xB0, 0xA3, 0xDE, 0x70, 0x38, 0x42, 0x59, 0x84, 0x9E, 0xD1,
        0x41, 0x18, 0x41, 0xD1, 0x5B, 0xE5, 0x91, 0xE0, 0x70, 0x84, 0x4E, 0xAA, 0x68, 0xAE, 0x33, 0x0F, 0x88, 0xC3, 0x5E, 0xAA,
        0xE0, 0x81, 0x65, 0xBF, 0x82, 0xE0, 0x4F, 0x6A, 0x3E, 0xB4, 0xE5, 0x0F, 0x3E, 0x78, 0x97, 0xED, 0x8A, 0x14, 0xB3, 0xBC,
        0xF3, 0x7D, 0x54, 0x9C, 0x41, 0xCE, 0x7A, 0xA2, 0x26, 0x10, 0x69, 0x9D, 0xA5, 0xF4, 0x8A, 0xF8, 0x06, 0x61, 0xF2, 0xB7,
    };

    uint8_t outBuf[sizeof(plainText)];

    aes128CTR.SetKey(key);
    aes128CTR.SetWeaveMessageCounter(nodeId, msgId);

    aes128CTR.EncryptData(plainText, sizeof(plainText), outBuf);

    res = (memcmp(outBuf, expectedCipherText, sizeof(plainText)) == 0);

    // Invalid ciphertext generated by AES128CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    aes128CTR.Reset();

    aes128CTR.SetKey(key);
    aes128CTR.SetWeaveMessageCounter(nodeId, msgId);

    aes128CTR.EncryptData(outBuf, sizeof(outBuf), outBuf);

    res = (memcmp(outBuf, plainText, sizeof(plainText)) == 0);

    // Invalid plaintext generated by AES128CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    aes128CTR.Reset();
}

bool AES256CTRMode_DoTest(const uint8_t * key, const uint8_t * ctr, const uint8_t * plainText, size_t plainTextLen,
                          const uint8_t * expectedCipherText)
{
    uint8_t cipherText[TEXT_BUFFER_LENGHT]         = { 0 };
    uint8_t decryptedPlainText[TEXT_BUFFER_LENGHT] = { 0 };
    bool res                                       = true;

    for (size_t chunkSize = 1; chunkSize <= plainTextLen; chunkSize++)
    {
        AES256CTRMode aes256CTR;

        aes256CTR.SetKey(key);
        aes256CTR.SetCounter(ctr);

        for (size_t chunkStart = 0; chunkStart < plainTextLen; chunkStart += chunkSize)
        {
            uint16_t inLen = plainTextLen - chunkStart;
            if (inLen > chunkSize)
                inLen = chunkSize;
            aes256CTR.EncryptData(plainText + chunkStart, inLen, cipherText + chunkStart);
        }

        aes256CTR.Reset();

        if (memcmp(cipherText, expectedCipherText, plainTextLen) != 0)
        {
            res = false;
            break;
        }
    }

    {
        AES256CTRMode aes256CTR;

        aes256CTR.SetKey(key);
        aes256CTR.SetCounter(ctr);

        aes256CTR.EncryptData(cipherText, plainTextLen, decryptedPlainText);

        if (memcmp(decryptedPlainText, plainText, plainTextLen) != 0)
            res = false;
    }

    return res;
}

static void Check_AES256CTRMode_Test1(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #7 from RFC-3686.
    static uint8_t key[]       = { 0x77, 0x6B, 0xEF, 0xF2, 0x85, 0x1D, 0xB0, 0x6F, 0x4C, 0x8A, 0x05, 0x42, 0xC8, 0x69, 0x6F, 0x6C,
                             0x6A, 0x81, 0xAF, 0x1E, 0xEC, 0x96, 0xB4, 0xD3, 0x7F, 0xC1, 0xD6, 0x89, 0xE6, 0xC1, 0xC1, 0x04 };
    static uint8_t ctr[]       = { 0x00, 0x00, 0x00, 0x60, 0xDB, 0x56, 0x72, 0xC9, 0x7A, 0xA8, 0xF0, 0xB2, 0x00, 0x00, 0x00, 0x01 };
    static uint8_t plainText[] = { 0x53, 0x69, 0x6E, 0x67, 0x6C, 0x65, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x6D, 0x73, 0x67 };
    static uint8_t expectedCipherText[] = { 0x14, 0x5A, 0xD0, 0x1D, 0xBF, 0x82, 0x4E, 0xC7,
                                            0x56, 0x08, 0x63, 0xDC, 0x71, 0xE3, 0xE0, 0xC0 };

    res = AES256CTRMode_DoTest(key, ctr, plainText, sizeof(plainText), expectedCipherText);

    // Invalid ciphertext generated by AES256CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

static void Check_AES256CTRMode_Test2(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #8 from RFC-3686.
    static uint8_t key[]       = { 0xF6, 0xD6, 0x6D, 0x6B, 0xD5, 0x2D, 0x59, 0xBB, 0x07, 0x96, 0x36, 0x58, 0x79, 0xEF, 0xF8, 0x86,
                             0xC6, 0x6D, 0xD5, 0x1A, 0x5B, 0x6A, 0x99, 0x74, 0x4B, 0x50, 0x59, 0x0C, 0x87, 0xA2, 0x38, 0x84 };
    static uint8_t ctr[]       = { 0x00, 0xFA, 0xAC, 0x24, 0xC1, 0x58, 0x5E, 0xF1, 0x5A, 0x43, 0xD8, 0x75, 0x00, 0x00, 0x00, 0x01 };
    static uint8_t plainText[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    static uint8_t expectedCipherText[] = { 0xF0, 0x5E, 0x23, 0x1B, 0x38, 0x94, 0x61, 0x2C, 0x49, 0xEE, 0x00,
                                            0x0B, 0x80, 0x4E, 0xB2, 0xA9, 0xB8, 0x30, 0x6B, 0x50, 0x8F, 0x83,
                                            0x9D, 0x6A, 0x55, 0x30, 0x83, 0x1D, 0x93, 0x44, 0xAF, 0x1C };

    res = AES256CTRMode_DoTest(key, ctr, plainText, sizeof(plainText), expectedCipherText);

    // Invalid ciphertext generated by AES256CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

static void Check_AES256CTRMode_Test3(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #9 from RFC-3686.
    static uint8_t key[]       = { 0xFF, 0x7A, 0x61, 0x7C, 0xE6, 0x91, 0x48, 0xE4, 0xF1, 0x72, 0x6E, 0x2F, 0x43, 0x58, 0x1D, 0xE2,
                             0xAA, 0x62, 0xD9, 0xF8, 0x05, 0x53, 0x2E, 0xDF, 0xF1, 0xEE, 0xD6, 0x87, 0xFB, 0x54, 0x15, 0x3D };
    static uint8_t ctr[]       = { 0x00, 0x1C, 0xC5, 0xB7, 0x51, 0xA5, 0x1D, 0x70, 0xA1, 0xC1, 0x11, 0x48, 0x00, 0x00, 0x00, 0x01 };
    static uint8_t plainText[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                   0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                   0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23 };
    static uint8_t expectedCipherText[] = { 0xEB, 0x6C, 0x52, 0x82, 0x1D, 0x0B, 0xBB, 0xF7, 0xCE, 0x75, 0x94, 0x46,
                                            0x2A, 0xCA, 0x4F, 0xAA, 0xB4, 0x07, 0xDF, 0x86, 0x65, 0x69, 0xFD, 0x07,
                                            0xF4, 0x8C, 0xC0, 0xB5, 0x83, 0xD6, 0x07, 0x1F, 0x1E, 0xC0, 0xE6, 0xB8 };

    res = AES256CTRMode_DoTest(key, ctr, plainText, sizeof(plainText), expectedCipherText);

    // Invalid ciphertext generated by AES256CTRMode::EncryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

bool AES128BlockCipher_DoTest(const uint8_t * key, const uint8_t * plainText, const uint8_t * expectedCipherText)
{
    uint8_t cipherText[AES128BlockCipherEnc::kBlockLength];
    uint8_t decryptedPlainText[AES128BlockCipherDec::kBlockLength];
    AES128BlockCipherEnc aes128BlockEnc;
    AES128BlockCipherDec aes128BlockDec;

    aes128BlockEnc.SetKey(key);
    aes128BlockEnc.EncryptBlock(plainText, cipherText);
    if (memcmp(cipherText, expectedCipherText, AES128BlockCipherEnc::kBlockLength) != 0)
    {
        return false;
    }

    aes128BlockDec.SetKey(key);
    aes128BlockDec.DecryptBlock(cipherText, decryptedPlainText);
    if (memcmp(plainText, decryptedPlainText, AES128BlockCipherDec::kBlockLength) != 0)
    {
        return false;
    }

    return true;
}

static void Check_AES128BlockCipher_Test1(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #1 from sp800-38a.pdf.
    static uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    static uint8_t plainTextBlock1[]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                         0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };
    static uint8_t plainTextBlock2[]  = { 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
                                         0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51 };
    static uint8_t plainTextBlock3[]  = { 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
                                         0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef };
    static uint8_t plainTextBlock4[]  = { 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
                                         0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
    static uint8_t cipherTextBlock1[] = { 0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                                          0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97 };
    static uint8_t cipherTextBlock2[] = { 0xf5, 0xd3, 0xd5, 0x85, 0x03, 0xb9, 0x69, 0x9d,
                                          0xe7, 0x85, 0x89, 0x5a, 0x96, 0xfd, 0xba, 0xaf };
    static uint8_t cipherTextBlock3[] = { 0x43, 0xb1, 0xcd, 0x7f, 0x59, 0x8e, 0xce, 0x23,
                                          0x88, 0x1b, 0x00, 0xe3, 0xed, 0x03, 0x06, 0x88 };
    static uint8_t cipherTextBlock4[] = { 0x7b, 0x0c, 0x78, 0x5e, 0x27, 0xe8, 0xad, 0x3f,
                                          0x82, 0x23, 0x20, 0x71, 0x04, 0x72, 0x5d, 0xd4 };

    res = AES128BlockCipher_DoTest(key, plainTextBlock1, cipherTextBlock1);

    // Invalid ciphertext generated by AES128BlockCipherEnc::EncryptData() or AES128BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    res = AES128BlockCipher_DoTest(key, plainTextBlock2, cipherTextBlock2);
    // Invalid ciphertext generated by AES128BlockCipherEnc::EncryptData() or AES128BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    res = AES128BlockCipher_DoTest(key, plainTextBlock3, cipherTextBlock3);

    // Invalid ciphertext generated by AES128BlockCipherEnc::EncryptData() or AES128BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    res = AES128BlockCipher_DoTest(key, plainTextBlock4, cipherTextBlock4);

    // Invalid ciphertext generated by AES128BlockCipherEnc::EncryptData() or AES128BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

bool AES256BlockCipher_DoTest(const uint8_t * key, const uint8_t * plainText, const uint8_t * expectedCipherText)
{
    uint8_t cipherText[AES256BlockCipherEnc::kBlockLength];
    uint8_t decryptedPlainText[AES256BlockCipherDec::kBlockLength];
    AES256BlockCipherEnc aes256BlockEnc;
    AES256BlockCipherDec aes256BlockDec;

    aes256BlockEnc.SetKey(key);
    aes256BlockEnc.EncryptBlock(plainText, cipherText);
    if (memcmp(cipherText, expectedCipherText, AES256BlockCipherEnc::kBlockLength) != 0)
    {
        return false;
    }

    aes256BlockDec.SetKey(key);
    aes256BlockDec.DecryptBlock(cipherText, decryptedPlainText);
    if (memcmp(plainText, decryptedPlainText, AES256BlockCipherDec::kBlockLength) != 0)
    {
        return false;
    }

    return true;
}

static void Check_AES256BlockCipher_Test1(nlTestSuite * inSuite, void * inContext)
{
    bool res;

    // This is Test Vector #1 from sp800-38a.pdf.
    static uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                             0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
    static uint8_t plainTextBlock1[]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                         0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };
    static uint8_t plainTextBlock2[]  = { 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
                                         0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51 };
    static uint8_t plainTextBlock3[]  = { 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
                                         0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef };
    static uint8_t plainTextBlock4[]  = { 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
                                         0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
    static uint8_t cipherTextBlock1[] = { 0xf3, 0xee, 0xd1, 0xbd, 0xb5, 0xd2, 0xa0, 0x3c,
                                          0x06, 0x4b, 0x5a, 0x7e, 0x3d, 0xb1, 0x81, 0xf8 };
    static uint8_t cipherTextBlock2[] = { 0x59, 0x1c, 0xcb, 0x10, 0xd4, 0x10, 0xed, 0x26,
                                          0xdc, 0x5b, 0xa7, 0x4a, 0x31, 0x36, 0x28, 0x70 };
    static uint8_t cipherTextBlock3[] = { 0xb6, 0xed, 0x21, 0xb9, 0x9c, 0xa6, 0xf4, 0xf9,
                                          0xf1, 0x53, 0xe7, 0xb1, 0xbe, 0xaf, 0xed, 0x1d };
    static uint8_t cipherTextBlock4[] = { 0x23, 0x30, 0x4b, 0x7a, 0x39, 0xf9, 0xf3, 0xff,
                                          0x06, 0x7d, 0x8d, 0x8f, 0x9e, 0x24, 0xec, 0xc7 };

    res = AES256BlockCipher_DoTest(key, plainTextBlock1, cipherTextBlock1);
    // Invalid ciphertext generated by AES256BlockCipherEnc::EncryptData() or AES256BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    res = AES256BlockCipher_DoTest(key, plainTextBlock2, cipherTextBlock2);
    // Invalid ciphertext generated by AES256BlockCipherEnc::EncryptData() or AES256BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    res = AES256BlockCipher_DoTest(key, plainTextBlock3, cipherTextBlock3);
    // Invalid ciphertext generated by AES256BlockCipherEnc::EncryptData() or AES256BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);

    res = AES256BlockCipher_DoTest(key, plainTextBlock4, cipherTextBlock4);
    // Invalid ciphertext generated by AES256BlockCipherEnc::EncryptData() or AES256BlockCipherDec::DecryptData()
    NL_TEST_ASSERT(inSuite, res == true);
}

static const nlTest sTests[] = { NL_TEST_DEF("AES128CTRMode Test1", Check_AES128CTRMode_Test1),
                                 NL_TEST_DEF("AES128CTRMode Test2", Check_AES128CTRMode_Test2),
                                 NL_TEST_DEF("AES128CTRMode Test3", Check_AES128CTRMode_Test3),
                                 NL_TEST_DEF("AES128CTRMode Test4", Check_AES128CTRMode_Test4),
                                 NL_TEST_DEF("AES256CTRMode Test1", Check_AES256CTRMode_Test1),
                                 NL_TEST_DEF("AES256CTRMode Test2", Check_AES256CTRMode_Test2),
                                 NL_TEST_DEF("AES256CTRMode Test3", Check_AES256CTRMode_Test3),
                                 NL_TEST_DEF("AES128BlockCipher Test1", Check_AES128BlockCipher_Test1),
                                 NL_TEST_DEF("AES256BlockCipher Test1", Check_AES256BlockCipher_Test1),
                                 NL_TEST_SENTINEL() };

int WeaveCryptoAESTests(void)
{
    nlTestSuite theSuite = { "Weave Crypto AES Tests", &sTests[0] };

    nl_test_set_output_style(OUTPUT_CSV);

    nlTestRunner(&theSuite, NULL);

    return nlTestRunnerStats(&theSuite);
}
