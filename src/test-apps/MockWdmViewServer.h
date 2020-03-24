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
 *      This file declares the Weave Data Management mock view server.
 *
 */

#ifndef MOCKWDMRESPONDER_H_
#define MOCKWDMRESPONDER_H_

#include <Weave/Core/WeaveExchangeMgr.h>

class MockWdmViewServer
{
public:
    static MockWdmViewServer * GetInstance();

    virtual WEAVE_ERROR Init(nl::Weave::WeaveExchangeManager * aExchangeMgr, const char * const aTestCaseId) = 0;
};

#endif /* MOCKWDMRESPONDER_H_ */
