
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
 *    SOURCE TEMPLATE: trait.cpp.h
 *    SOURCE PROTO: weave/trait/pairing/configuration_done_trait.proto
 *
 */
#ifndef _WEAVE_TRAIT_PAIRING__CONFIGURATION_DONE_TRAIT_H_
#define _WEAVE_TRAIT_PAIRING__CONFIGURATION_DONE_TRAIT_H_

#include <Weave/Profiles/data-management/DataManagement.h>
#include <Weave/Support/SerializationUtils.h>

namespace Schema {
namespace Weave {
namespace Trait {
namespace Pairing {
namespace ConfigurationDoneTrait {

extern const nl::Weave::Profiles::DataManagement::TraitSchemaEngine TraitSchema;

enum
{
    kWeaveProfileId = (0x0U << 16) | 0x1401U
};

//
// Properties
//

enum
{
    kPropertyHandle_Root = 1,

    //---------------------------------------------------------------------------------------------------------------------------//
    //  Name                                IDL Type                            TLV Type           Optional?       Nullable?     //
    //---------------------------------------------------------------------------------------------------------------------------//

    //
    //  app_configuration_complete          bool                                 bool              NO              NO
    //
    kPropertyHandle_AppConfigurationComplete = 2,

    //
    //  device_ready                        bool                                 bool              NO              NO
    //
    kPropertyHandle_DeviceReady = 3,

    //
    // Enum for last handle
    //
    kLastSchemaHandle = 3,
};

} // namespace ConfigurationDoneTrait
} // namespace Pairing
} // namespace Trait
} // namespace Weave
} // namespace Schema
#endif // _WEAVE_TRAIT_PAIRING__CONFIGURATION_DONE_TRAIT_H_
