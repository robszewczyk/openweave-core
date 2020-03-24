/*
 *
 *    Copyright (c) 2020 Google LLC.
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
 *      TraitSchemaDirectory which locate the trait schema from different trait header files
 */

#ifndef TRAIT_SCHEMA_DIRECTORY_H_
#define TRAIT_SCHEMA_DIRECTORY_H_

#include <Weave/Profiles/data-management/Current/WdmManagedNamespace.h>
#include <Weave/Profiles/data-management/DataManagement.h>

namespace nl {
namespace Weave {
namespace DeviceManager {
using namespace ::nl::Weave::Profiles::DataManagement_Current;

class TraitSchemaDirectory
{
public:
    static const nl::Weave::Profiles::DataManagement::TraitSchemaEngine * GetTraitSchemaEngine(uint32_t aProfileId);
};

} // namespace DeviceManager
} // namespace Weave
} // namespace nl

#endif // TRAIT_SCHEMA_DIRECTORY_H_
