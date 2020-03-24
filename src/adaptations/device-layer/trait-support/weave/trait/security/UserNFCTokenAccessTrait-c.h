
/*
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2016-2018 Nest Labs, Inc.
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

/*
 *    THIS FILE IS GENERATED. DO NOT MODIFY.
 *
 *    SOURCE TEMPLATE: trait.c.h
 *    SOURCE PROTO: weave/trait/security/user_nfc_token_access_trait.proto
 *
 */
#ifndef _WEAVE_TRAIT_SECURITY__USER_NFC_TOKEN_ACCESS_TRAIT_C_H_
#define _WEAVE_TRAIT_SECURITY__USER_NFC_TOKEN_ACCESS_TRAIT_C_H_

//
// Enums
//

// UserNFCTokenAccessResult
typedef enum
{
    USER_NFC_TOKEN_ACCESS_RESULT_SUCCESS                 = 1,
    USER_NFC_TOKEN_ACCESS_RESULT_FAILURE_UNKNOWN_TOKEN   = 2,
    USER_NFC_TOKEN_ACCESS_RESULT_FAILURE_INVALID_TOKEN   = 3,
    USER_NFC_TOKEN_ACCESS_RESULT_FAILURE_OUT_OF_SCHEDULE = 4,
    USER_NFC_TOKEN_ACCESS_RESULT_FAILURE_TOKEN_DISABLED  = 5,
    USER_NFC_TOKEN_ACCESS_RESULT_FAILURE_INVALID_VERSION = 6,
    USER_NFC_TOKEN_ACCESS_RESULT_FAILURE_OTHER_REASON    = 7,
} schema_weave_security_user_nfc_token_access_trait_user_nfc_token_access_result_t;

#endif // _WEAVE_TRAIT_SECURITY__USER_NFC_TOKEN_ACCESS_TRAIT_C_H_
