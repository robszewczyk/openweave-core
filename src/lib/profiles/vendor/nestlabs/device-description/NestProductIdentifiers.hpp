/*
 *    Copyright (c) 2014-2018 Nest Labs, Inc.
 *    Copyright (c) 2019 Google, LLC.
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
 *      This file defines Nest Labs Weave product identifiers.
 *
 *      Product identifiers are assigned and administered by vendors who
 *      have been assigned an official Weave vendor identifier by Nest Labs.
 *
 */

#ifndef NEST_LABS_WEAVE_PRODUCT_IDENTIFIERS_HPP
#define NEST_LABS_WEAVE_PRODUCT_IDENTIFIERS_HPP

namespace nl {

namespace Weave {

namespace Profiles {

namespace Vendor {

namespace Nestlabs {

namespace DeviceDescription {

//
// Nest Labs Weave Product Identifiers (16 bits max)
//

enum NestWeaveProductId
{
    kNestWeaveProduct_Diamond           = 0x0001,
    kNestWeaveProduct_DiamondBackplate  = 0x0002,
    kNestWeaveProduct_Diamond2          = 0x0003,
    kNestWeaveProduct_Diamond2Backplate = 0x0004,
    kNestWeaveProduct_Topaz =
        0x0005, // DEPRECATED -- Use kNestWeaveProduct_Topaz1LinePowered or kNestWeaveProduct_Topaz1BatteryPowered
    kNestWeaveProduct_AmberBackplate = 0x0006,
    kNestWeaveProduct_Amber          = 0x0007, // DEPRECATED -- Use kNestWeaveProduct_AmberHeatLink
    kNestWeaveProduct_AmberHeatLink  = 0x0007,
    kNestWeaveProduct_Topaz2 =
        0x0009, // DEPRECATED -- Use kNestWeaveProduct_Topaz2LinePowered or kNestWeaveProduct_Topaz2BatteryPowered
    kNestWeaveProduct_Diamond3               = 0x000A,
    kNestWeaveProduct_Diamond3Backplate      = 0x000B,
    kNestWeaveProduct_Quartz                 = 0x000D,
    kNestWeaveProduct_Amber2HeatLink         = 0x000F,
    kNestWeaveProduct_SmokyQuartz            = 0x0010,
    kNestWeaveProduct_Quartz2                = 0x0011,
    kNestWeaveProduct_BlackQuartz            = 0x0012,
    kNestWeaveProduct_Onyx                   = 0x0014,
    kNestWeaveProduct_OnyxBackplate          = 0x0015,
    kNestWeaveProduct_Topaz1LinePowered      = 0x001E,
    kNestWeaveProduct_Topaz1BatteryPowered   = 0x001F,
    kNestWeaveProduct_Topaz2LinePowered      = 0x0020,
    kNestWeaveProduct_Topaz2BatteryPowered   = 0x0021,
    kNestWeaveProduct_SDKSampleBorderRouter  = 0xFE05,
    kNestWeaveProduct_SDKSampleLightActuator = 0xFE06,
    kNestWeaveProduct_SDKSampleButtonSensor  = 0xFE07,
    kNestWeaveProduct_SDKSampleMotionSensor  = 0xFE08,
};

}; // namespace DeviceDescription

}; // namespace Nestlabs

}; // namespace Vendor

}; // namespace Profiles

}; // namespace Weave

}; // namespace nl

#endif // NEST_LABS_WEAVE_PRODUCT_IDENTIFIERS_HPP
