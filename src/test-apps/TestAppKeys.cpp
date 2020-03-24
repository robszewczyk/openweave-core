/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements unit tests for the Weave application keys library.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <stdio.h>
#include <nlunit-test.h>
#include <string.h>
#include <new>

#include "ToolCommon.h"
#include "TestGroupKeyStore.h"
#include <Weave/Support/ErrorStr.h>
#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Core/WeaveEncoding.h>
#include <Weave/Profiles/security/WeaveApplicationKeys.h>
#include <Weave/Profiles/security/WeavePasscodes.h>
#include <Weave/Support/crypto/WeaveCrypto.h>

using namespace nl::Weave::Encoding;
using namespace nl::Weave::Crypto;
using namespace nl::Weave::Profiles::Security::AppKeys;
using namespace nl::Weave::Profiles::Security::Passcodes;

#define DEBUG_PRINT_ENABLE 0

static void ClearSecretKeyMaterial(WeaveGroupKey & key)
{
    key.KeyId     = WeaveKeyId::kNone;
    key.KeyLen    = 0;
    key.StartTime = UINT32_MAX;
    ClearSecretData(key.Key, key.MaxKeySize);
}

void DeriveAppRootKey_Test(nlTestSuite * inSuite, void * inContext)
{
    WEAVE_ERROR err;
    TestGroupKeyStore keyStore;
    WeaveGroupKey rootKey;
    uint32_t keyId;
    uint8_t rootKeyHKDF[kWeaveAppRootKeySize];

    // Derive fabric root key.
    keyId = WeaveKeyId::kFabricRootKey;
    err   = keyStore.GetGroupKey(keyId, rootKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rootKey.KeyLen == kWeaveAppRootKeySize);
    NL_TEST_ASSERT(inSuite, rootKey.KeyId == keyId);

#if DEBUG_PRINT_ENABLE
    printf("Derived fabric root key:\n");
    DumpMemoryCStyle(rootKey.Key, rootKey.KeyLen, "    ", 16);
#endif

    // Use HKDF function manually to derive fabric root key and compare against result generated by GetGroupKey().
    err = HKDFSHA1::DeriveKey(NULL, 0, sFabricSecret, sFabricSecretLen, NULL, 0, kWeaveAppFabricRootKeyDiversifier,
                              kWeaveAppFabricRootKeyDiversifierSize, rootKeyHKDF, sizeof(rootKeyHKDF), kWeaveAppRootKeySize);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(rootKey.Key, rootKeyHKDF, kWeaveAppRootKeySize) == 0);

    // Compare the result against precalculated value.
    NL_TEST_ASSERT(inSuite, memcmp(rootKey.Key, sFabricRootKey, sFabricRootKeyLen) == 0);

    ClearSecretKeyMaterial(rootKey);

    // Derive client root key.
    keyId = WeaveKeyId::kClientRootKey;
    err   = keyStore.GetGroupKey(keyId, rootKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rootKey.KeyLen == kWeaveAppRootKeySize);
    NL_TEST_ASSERT(inSuite, rootKey.KeyId == keyId);

#if DEBUG_PRINT_ENABLE
    printf("Derived client root key:\n");
    DumpMemoryCStyle(rootKey.Key, rootKey.KeyLen, "    ", 16);
#endif

    // Use HKDF function manually to derive client root key and compare against result generated by GetGroupKey().
    err = HKDFSHA1::DeriveKey(NULL, 0, sFabricSecret, sFabricSecretLen, NULL, 0, kWeaveAppClientRootKeyDiversifier,
                              kWeaveAppClientRootKeyDiversifierSize, rootKeyHKDF, sizeof(rootKeyHKDF), kWeaveAppRootKeySize);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(rootKey.Key, rootKeyHKDF, kWeaveAppRootKeySize) == 0);

    // Compare the result against precalculated value.
    NL_TEST_ASSERT(inSuite, memcmp(rootKey.Key, sClientRootKey, sClientRootKeyLen) == 0);

    ClearSecretKeyMaterial(rootKey);

    // Retrieve service root key.
    keyId = WeaveKeyId::kServiceRootKey;
    err   = keyStore.GetGroupKey(keyId, rootKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rootKey.KeyLen == kWeaveAppRootKeySize);
    NL_TEST_ASSERT(inSuite, rootKey.KeyId == keyId);
    NL_TEST_ASSERT(inSuite, memcmp(rootKey.Key, sServiceRootKey, sServiceRootKeyLen) == 0);

    ClearSecretKeyMaterial(rootKey);

    // Try to derive invalid root key.
    keyId = sInvalidRootKeyId;
    err   = keyStore.GetGroupKey(keyId, rootKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_ERROR_KEY_NOT_FOUND);
}

void DeriveAppIntermediateKey_Test(nlTestSuite * inSuite, void * inContext)
{
    WEAVE_ERROR err;
    TestGroupKeyStore keyStore;
    WeaveGroupKey intermediateKey;
    uint32_t keyId;

    // Derive intermediate key from fabric root key and epoch key #2.
    keyId = sIntermediateKeyId_FRK_E2;
    err   = keyStore.GetGroupKey(keyId, intermediateKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

#if DEBUG_PRINT_ENABLE
    printf("Derived intermediate key using fabric root key and epoch key #2 (FRK_E2):\n");
    DumpMemoryCStyle(intermediateKey.Key, intermediateKey.KeyLen, "    ", 16);
#endif

    // Compare the result against precalculated value.
    NL_TEST_ASSERT(inSuite, intermediateKey.KeyLen == kWeaveAppIntermediateKeySize);
    NL_TEST_ASSERT(inSuite, intermediateKey.KeyId == keyId);
    NL_TEST_ASSERT(inSuite, memcmp(intermediateKey.Key, sIntermediateKey_FRK_E2, sIntermediateKeyLen_FRK_E2) == 0);

    ClearSecretKeyMaterial(intermediateKey);

    // Derive intermediate key from fabric root key and current epoch key.
    keyId           = sIntermediateKeyId_FRK_EC;
    sCurrentUTCTime = sEpochKey2_StartTime + 1;
    err             = keyStore.GetGroupKey(keyId, intermediateKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

    NL_TEST_ASSERT(inSuite, intermediateKey.KeyLen == kWeaveAppIntermediateKeySize);
    NL_TEST_ASSERT(inSuite, intermediateKey.KeyId == sIntermediateKeyId_FRK_E2);
    NL_TEST_ASSERT(inSuite, memcmp(intermediateKey.Key, sIntermediateKey_FRK_E2, sIntermediateKeyLen_FRK_E2) == 0);
    NL_TEST_ASSERT(inSuite, sLastUsedEpochKeyId == sEpochKey2_KeyId);
    NL_TEST_ASSERT(inSuite, keyStore.TestValue_LastUsedEpochKeyId() == sEpochKey2_KeyId);
    NL_TEST_ASSERT(inSuite, keyStore.TestValue_NextEpochKeyStartTime() == sEpochKey3_StartTime);

    ClearSecretKeyMaterial(intermediateKey);

    // Derive intermediate key from fabric root key and current epoch key, while the time value is not accurate.
    keyId           = sIntermediateKeyId_FRK_EC;
    sCurrentUTCTime = 0x0;
    err             = keyStore.GetGroupKey(keyId, intermediateKey);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

    NL_TEST_ASSERT(inSuite, intermediateKey.KeyLen == kWeaveAppIntermediateKeySize);
    NL_TEST_ASSERT(inSuite, intermediateKey.KeyId == sIntermediateKeyId_FRK_E2);
    NL_TEST_ASSERT(inSuite, memcmp(intermediateKey.Key, sIntermediateKey_FRK_E2, sIntermediateKeyLen_FRK_E2) == 0);
    NL_TEST_ASSERT(inSuite, keyStore.TestValue_LastUsedEpochKeyId() == sEpochKey2_KeyId);

    ClearSecretKeyMaterial(intermediateKey);
}

void DeriveAppStaticKey_Test(nlTestSuite * inSuite, void * inContext)
{
    WEAVE_ERROR err;
    TestGroupKeyStore keyStore;
    uint8_t appStaticKey[sAppStaticKeyLen_CRK_G10];
    uint32_t keyId;
    uint8_t appStaticKeyHKDF[sAppStaticKeyLen_CRK_G10];
    uint32_t appGroupGlobalId;

    // Derive application static key from client root key and group master key #10.
    keyId = sAppStaticKeyId_CRK_G10;
    err   = keyStore.DeriveApplicationKey(keyId, NULL, 0, sAppStaticKeyDiversifier_CRK_G10, sAppStaticKeyDiversifierLen_CRK_G10,
                                        appStaticKey, sizeof(appStaticKey), sAppStaticKeyLen_CRK_G10, appGroupGlobalId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, appGroupGlobalId == sAppGroupMasterKey10_GlobalId);

#if DEBUG_PRINT_ENABLE
    printf("Derived application static key, using client root key and group master key #10 (CRK_G10):\n");
    DumpMemoryCStyle(appStaticKey, sAppStaticKeyLen_CRK_G10, "    ", 16);
#endif

    // Use HKDF function manually to derive application static key and compare against result generated by DeriveApplicationKey().
    err = HKDFSHA1::DeriveKey(NULL, 0, sClientRootKey, sClientRootKeyLen, sAppGroupMasterKey10_Key, sAppGroupMasterKey10_KeyLen,
                              sAppStaticKeyDiversifier_CRK_G10, sAppStaticKeyDiversifierLen_CRK_G10, appStaticKeyHKDF,
                              sizeof(appStaticKeyHKDF), sAppStaticKeyLen_CRK_G10);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(appStaticKey, appStaticKeyHKDF, sAppStaticKeyLen_CRK_G10) == 0);

    // Compare the result against precalculated value.
    NL_TEST_ASSERT(inSuite, keyId == sAppStaticKeyId_CRK_G10);
    NL_TEST_ASSERT(inSuite, memcmp(appStaticKey, sAppStaticKey_CRK_G10, sAppStaticKeyLen_CRK_G10) == 0);
}

void DeriveAppRotatingKey_Test(nlTestSuite * inSuite, void * inContext)
{
    WEAVE_ERROR err;
    TestGroupKeyStore keyStore;
    uint8_t appRotatingKey[sAppRotatingKeyLen_SRK_E3_G54];
    uint32_t keyId;
    uint8_t intermediateKeyHKDF[kWeaveAppIntermediateKeySize];
    uint8_t appRotatingKeyHKDF[sAppRotatingKeyLen_SRK_E3_G54];
    uint32_t appGroupGlobalId;

    // Derive application rotating key from service root key and group master key #54.
    keyId = sAppRotatingKeyId_SRK_E3_G54;
    err   = keyStore.DeriveApplicationKey(keyId, NULL, 0, sAppRotatingKeyDiversifier_SRK_E3_G54,
                                        sAppRotatingKeyDiversifierLen_SRK_E3_G54, appRotatingKey, sizeof(appRotatingKey),
                                        sAppRotatingKeyLen_SRK_E3_G54, appGroupGlobalId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, appGroupGlobalId == sAppGroupMasterKey54_GlobalId);

#if DEBUG_PRINT_ENABLE
    printf("Derived application rotating key, using service root key, epoch key #3 and group master key #54 (SRK_E3_G54):\n");
    DumpMemoryCStyle(appRotatingKey, sAppRotatingKeyLen_SRK_E3_G54, "    ", 16);
#endif

    // Use HKDF function manually to derive application rotating key and compare against result generated by DeriveApplicationKey().
    err = HKDFSHA1::DeriveKey(NULL, 0, sServiceRootKey, sServiceRootKeyLen, sEpochKey3_Key, sEpochKey3_KeyLen,
                              kWeaveAppIntermediateKeyDiversifier, kWeaveAppIntermediateKeyDiversifierSize, intermediateKeyHKDF,
                              sizeof(intermediateKeyHKDF), kWeaveAppIntermediateKeySize);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

    err = HKDFSHA1::DeriveKey(NULL, 0, intermediateKeyHKDF, kWeaveAppIntermediateKeySize, sAppGroupMasterKey54_Key,
                              sAppGroupMasterKey54_KeyLen, sAppRotatingKeyDiversifier_SRK_E3_G54,
                              sAppRotatingKeyDiversifierLen_SRK_E3_G54, appRotatingKeyHKDF, sizeof(appRotatingKeyHKDF),
                              sAppRotatingKeyLen_SRK_E3_G54);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(appRotatingKey, appRotatingKeyHKDF, sAppRotatingKeyLen_SRK_E3_G54) == 0);

    // Compare the result against precalculated value.
    NL_TEST_ASSERT(inSuite, keyId == sAppRotatingKeyId_SRK_E3_G54);
    NL_TEST_ASSERT(inSuite, memcmp(appRotatingKey, sAppRotatingKey_SRK_E3_G54, sAppRotatingKeyLen_SRK_E3_G54) == 0);
}

void DerivePasscodeKeys_Test(nlTestSuite * inSuite, void * inContext)
{
    WEAVE_ERROR err;
    TestGroupKeyStore keyStore;
    uint8_t appKey[sPasscodeEncRotatingKeyLen_CRK_E0_G4];
    uint32_t keyId;
    uint8_t nonce[sPasscodeEncryptionKeyNonceLen];
    uint8_t intermediateKeyHKDF[kWeaveAppIntermediateKeySize];
    uint8_t appKeyHKDF[sPasscodeEncRotatingKeyLen_CRK_E0_G4];
    uint32_t appGroupGlobalId;

    // 1. Derive passcode encryption (and authentication) ROTATING key using client root key, epoch key #0 and group master key #4.
    keyId = sPasscodeEncRotatingKeyId_CRK_E0_G4;
    LittleEndian::Put32(nonce, sPasscodeEncryptionKeyNonce);
    err = keyStore.DeriveApplicationKey(keyId, nonce, sPasscodeEncryptionKeyNonceLen, sPasscodeEncryptionKeyDiversifier,
                                        sPasscodeEncryptionKeyDiversifierLen, appKey, sizeof(appKey),
                                        sPasscodeEncRotatingKeyLen_CRK_E0_G4, appGroupGlobalId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keyId == sPasscodeEncRotatingKeyId_CRK_E0_G4);
    NL_TEST_ASSERT(inSuite, memcmp(appKey, sPasscodeEncRotatingKey_CRK_E0_G4, sPasscodeEncRotatingKeyLen_CRK_E0_G4) == 0);
    NL_TEST_ASSERT(inSuite, appGroupGlobalId == sAppGroupMasterKey4_GlobalId);

#if DEBUG_PRINT_ENABLE
    printf(
        "Derived passcode encryption (and authentication) rotating key using client root key, epoch key #0 and group master key #4 "
        "(CRK_E0_G4):\n");
    DumpMemoryCStyle(appKey, sPasscodeEncRotatingKeyLen_CRK_E0_G4, "    ", 16);
#endif

    // Use HKDF function manually to derive passcode encryption rotating key and compare against result generated by
    // DeriveApplicationKey().
    err = HKDFSHA1::DeriveKey(NULL, 0, sClientRootKey, sClientRootKeyLen, sEpochKey0_Key, sEpochKey0_KeyLen,
                              kWeaveAppIntermediateKeyDiversifier, kWeaveAppIntermediateKeyDiversifierSize, intermediateKeyHKDF,
                              sizeof(intermediateKeyHKDF), kWeaveAppIntermediateKeySize);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);

    err = HKDFSHA1::DeriveKey(nonce, sPasscodeEncryptionKeyNonceLen, intermediateKeyHKDF, kWeaveAppIntermediateKeySize,
                              sAppGroupMasterKey4_Key, sAppGroupMasterKey4_KeyLen, sPasscodeEncryptionKeyDiversifier,
                              sPasscodeEncryptionKeyDiversifierLen, appKeyHKDF, sizeof(appKeyHKDF),
                              sPasscodeEncRotatingKeyLen_CRK_E0_G4);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(appKey, appKeyHKDF, sPasscodeEncRotatingKeyLen_CRK_E0_G4) == 0);

    // 2. Derive passcode encryption (and authentication) STATIC key using client root key and group master key #4.
    keyId = sPasscodeEncStaticKeyId_CRK_G4;
    LittleEndian::Put32(nonce, sPasscodeEncryptionKeyNonce);
    err = keyStore.DeriveApplicationKey(keyId, nonce, sPasscodeEncryptionKeyNonceLen, sPasscodeEncryptionKeyDiversifier,
                                        sPasscodeEncryptionKeyDiversifierLen, appKey, sizeof(appKey),
                                        sPasscodeEncStaticKeyLen_CRK_G4, appGroupGlobalId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keyId == sPasscodeEncStaticKeyId_CRK_G4);
    NL_TEST_ASSERT(inSuite, memcmp(appKey, sPasscodeEncStaticKey_CRK_G4, sPasscodeEncStaticKeyLen_CRK_G4) == 0);
    NL_TEST_ASSERT(inSuite, appGroupGlobalId == sAppGroupMasterKey4_GlobalId);

#if DEBUG_PRINT_ENABLE
    printf("Derived passcode encryption (and authentication) static key using client root key and group master key #4 (CRK_G4):\n");
    DumpMemoryCStyle(appKey, sPasscodeEncStaticKeyLen_CRK_G4, "    ", 16);
#endif

    // Use HKDF function manually to derive passcode encryption static key and compare against result generated by
    // DeriveApplicationKey().
    err = HKDFSHA1::DeriveKey(nonce, sPasscodeEncryptionKeyNonceLen, sClientRootKey, sClientRootKeyLen, sAppGroupMasterKey4_Key,
                              sAppGroupMasterKey4_KeyLen, sPasscodeEncryptionKeyDiversifier, sPasscodeEncryptionKeyDiversifierLen,
                              appKeyHKDF, sizeof(appKeyHKDF), sPasscodeEncRotatingKeyLen_CRK_E0_G4);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(appKey, appKeyHKDF, sPasscodeEncStaticKeyLen_CRK_G4) == 0);

    // 3. Derive passcode fingerprint (always static) key using client root key and group master key #4.
    keyId = sPasscodeFingerprintKeyId_CRK_G4;
    err = keyStore.DeriveApplicationKey(keyId, NULL, 0, kPasscodeFingerprintKeyDiversifier, kPasscodeFingerprintKeyDiversifierSize,
                                        appKey, sizeof(appKey), sPasscodeFingerprintKeyLen_CRK_G4, appGroupGlobalId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keyId == sPasscodeFingerprintKeyId_CRK_G4);
    NL_TEST_ASSERT(inSuite, memcmp(appKey, sPasscodeFingerprintKey_CRK_G4, sPasscodeFingerprintKeyLen_CRK_G4) == 0);
    NL_TEST_ASSERT(inSuite, appGroupGlobalId == sAppGroupMasterKey4_GlobalId);

#if DEBUG_PRINT_ENABLE
    printf("Derived passcode fingerprint key using client root key and group master key #4 (CRK_G4):\n");
    DumpMemoryCStyle(appKey, sPasscodeFingerprintKeyLen_CRK_G4, "    ", 16);
#endif

    // Use HKDF function manually to derive passcode encryption static key and compare against result generated by
    // DeriveApplicationKey().
    err = HKDFSHA1::DeriveKey(NULL, 0, sClientRootKey, sClientRootKeyLen, sAppGroupMasterKey4_Key, sAppGroupMasterKey4_KeyLen,
                              kPasscodeFingerprintKeyDiversifier, kPasscodeFingerprintKeyDiversifierSize, appKeyHKDF,
                              sizeof(appKeyHKDF), sPasscodeFingerprintKeyLen_CRK_G4);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(appKey, appKeyHKDF, sPasscodeFingerprintKeyLen_CRK_G4) == 0);
}

void GetAppGroupMasterKeyId_Test(nlTestSuite * inSuite, void * inContext)
{
    WEAVE_ERROR err;
    TestGroupKeyStore keyStore;
    uint32_t groupGlobalId;
    uint32_t groupMasterKeyId;

    // Get application group key Id given group global Id - for the app group #0.
    groupGlobalId = sAppGroupMasterKey0_GlobalId;
    err           = GetAppGroupMasterKeyId(groupGlobalId, &keyStore, groupMasterKeyId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, groupMasterKeyId == sAppGroupMasterKey0_KeyId);

    groupMasterKeyId = WeaveKeyId::kNone;
    groupGlobalId    = 0;

    // Get application group key Id given group global Id - for the app group #4.
    groupGlobalId = sAppGroupMasterKey4_GlobalId;
    err           = GetAppGroupMasterKeyId(groupGlobalId, &keyStore, groupMasterKeyId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, groupMasterKeyId == sAppGroupMasterKey4_KeyId);

    groupMasterKeyId = WeaveKeyId::kNone;
    groupGlobalId    = 0;

    // Get application group key Id given group global Id - for the app group #10.
    groupGlobalId = sAppGroupMasterKey10_GlobalId;
    err           = GetAppGroupMasterKeyId(groupGlobalId, &keyStore, groupMasterKeyId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, groupMasterKeyId == sAppGroupMasterKey10_KeyId);

    groupMasterKeyId = WeaveKeyId::kNone;
    groupGlobalId    = 0;

    // Get application group key Id given group global Id - for the app group #54.
    groupGlobalId = sAppGroupMasterKey54_GlobalId;
    err           = GetAppGroupMasterKeyId(groupGlobalId, &keyStore, groupMasterKeyId);
    NL_TEST_ASSERT(inSuite, err == WEAVE_NO_ERROR);
    NL_TEST_ASSERT(inSuite, groupMasterKeyId == sAppGroupMasterKey54_KeyId);
}

int main(int argc, char * argv[])
{
    static const nlTest tests[] = { NL_TEST_DEF("DeriveAppRootKey", DeriveAppRootKey_Test),
                                    NL_TEST_DEF("DeriveAppIntermediateKey", DeriveAppIntermediateKey_Test),
                                    NL_TEST_DEF("DeriveAppStaticKey", DeriveAppStaticKey_Test),
                                    NL_TEST_DEF("DeriveAppRotatingKey", DeriveAppRotatingKey_Test),
                                    NL_TEST_DEF("DerivePasscodeKeys", DerivePasscodeKeys_Test),
                                    NL_TEST_DEF("GetAppGroupMasterKeyId", GetAppGroupMasterKeyId_Test),
                                    NL_TEST_SENTINEL() };

    static nlTestSuite testSuite = { "application-keys", &tests[0] };

    nl_test_set_output_style(OUTPUT_CSV);

    nlTestRunner(&testSuite, NULL);

    return nlTestRunnerStats(&testSuite);
}
