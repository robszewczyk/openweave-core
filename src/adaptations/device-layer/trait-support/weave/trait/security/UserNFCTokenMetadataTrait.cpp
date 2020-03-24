
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
 *    SOURCE TEMPLATE: trait.cpp
 *    SOURCE PROTO: weave/trait/security/user_nfc_token_metadata_trait.proto
 *
 */

#include <weave/trait/security/UserNFCTokenMetadataTrait.h>

namespace Schema {
namespace Weave {
namespace Trait {
namespace Security {
namespace UserNFCTokenMetadataTrait {

using namespace ::nl::Weave::Profiles::DataManagement;

//
// Property Table
//

const TraitSchemaEngine::PropertyInfo PropertyMap[] = {
    { kPropertyHandle_Root, 1 },     // metadata
    { kPropertyHandle_Metadata, 1 }, // serial_number
    { kPropertyHandle_Metadata, 2 }, // tag_number
};

//
// Schema
//

const TraitSchemaEngine TraitSchema = { {
    kWeaveProfileId,
    PropertyMap,
    sizeof(PropertyMap) / sizeof(PropertyMap[0]),
    2,
#if (TDM_EXTENSION_SUPPORT) || (TDM_VERSIONING_SUPPORT)
    2,
#endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if (TDM_EXTENSION_SUPPORT)
    NULL,
#endif
#if (TDM_VERSIONING_SUPPORT)
    NULL,
#endif
} };

//
// Event Structs
//

const nl::FieldDescriptor MetadataFieldDescriptors[] = {
    { NULL, offsetof(Metadata, serialNumber), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeUTF8String, 0), 1 },

    { NULL, offsetof(Metadata, tagNumber), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeUTF8String, 0), 2 },

};

const nl::SchemaFieldDescriptor Metadata::FieldSchema = { .mNumFieldDescriptorElements = sizeof(MetadataFieldDescriptors) /
                                                              sizeof(MetadataFieldDescriptors[0]),
                                                          .mFields = MetadataFieldDescriptors,
                                                          .mSize   = sizeof(Metadata) };

} // namespace UserNFCTokenMetadataTrait
} // namespace Security
} // namespace Trait
} // namespace Weave
} // namespace Schema
