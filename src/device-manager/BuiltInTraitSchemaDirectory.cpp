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
 *      WARNING: This is experimental feature for Weave Data Management Client
 */

// This is a .cpp file in openweave-core that includes the "built-in schema"
// source file, if defined...

#include <Weave/Core/WeaveCore.h>

#if WEAVE_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL
#include <Weave/DeviceManager/TraitSchemaDirectory.h>

#if defined(WEAVE_CONFIG_CUSTOM_BUILTIN_SCHEMA_INCLUDE)
#define DirectoryGen_(dir) #dir
#define DirectoryGen(dir)  DirectoryGen_(dir)
#include DirectoryGen(WEAVE_CONFIG_CUSTOM_BUILTIN_SCHEMA_INCLUDE)
#else
#include <weave/trait/locale/LocaleSettingsTrait.h>
#include <weave/trait/locale/LocaleCapabilitiesTrait.h>
#include <nest/test/trait/TestCTrait.h>

namespace nl {
namespace Weave {
namespace DeviceManager {
using namespace ::nl::Weave::Profiles::DataManagement_Current;

const TraitSchemaEngine * TraitSchemaDirectory::GetTraitSchemaEngine(uint32_t aProfileId)
{
    if (aProfileId == ::Weave::Trait::Locale::LocaleSettingsTrait::kWeaveProfileId)
    {
        return &::Weave::Trait::Locale::LocaleSettingsTrait::TraitSchema;
    }
    if (aProfileId == ::Schema::Nest::Test::Trait::TestCTrait::kWeaveProfileId)
    {
        return &::Schema::Nest::Test::Trait::TestCTrait::TraitSchema;
    }
    if (aProfileId == ::Weave::Trait::Locale::LocaleCapabilitiesTrait::kWeaveProfileId)
    {
        return &::Weave::Trait::Locale::LocaleCapabilitiesTrait::TraitSchema;
    }
    else
    {
        WeaveLogError(DataManagement, "Schema does not exit with profile id %" PRIu32 " ", aProfileId);
        return NULL;
    }
}

} // namespace DeviceManager
} // namespace Weave
} // namespace nl

#include <weave/trait/locale/LocaleSettingsTrait.cpp>
#include <weave/trait/locale/LocaleCapabilitiesTrait.cpp>
#include <nest/test/trait/TestCTrait.cpp>

#endif // !defined(WEAVE_CONFIG_CUSTOM_BUILTIN_SCHEMA_INCLUDE)
#endif // WEAVE_CONFIG_DATA_MANAGEMENT_CLIENT_EXPERIMENTAL
