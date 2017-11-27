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
 *      This file implements the Weave Data Management mock Update client.
 *
 */

#define WEAVE_CONFIG_ENABLE_LOG_FILE_LINE_FUNC_ON_ERROR 1

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <new>
#include "MockWdmUpdateClient.h"
#include <Weave/Profiles/data-management/Current/WdmManagedNamespace.h>
#include <Weave/Core/WeaveError.h>
#include <Weave/Core/WeaveSecurityMgr.h>
#include <Weave/Support/CodeUtils.h>
#include <Weave/Profiles/WeaveProfiles.h>
#include <Weave/Support/TimeUtils.h>

// We want and assume the default managed namespace is Current and that is, explicitly, the managed namespace this code desires.
#include <Weave/Profiles/data-management/DataManagement.h>

#include "MockSinkTraits.h"
#include "MockSourceTraits.h"
#include "TestGroupKeyStore.h"

#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING

using namespace nl::Weave;
using namespace nl::Weave::TLV;
using namespace nl::Weave::Profiles;
using namespace nl::Weave::Profiles::DataManagement;
using namespace Weave::Trait::Locale;

const nl::Weave::ExchangeContext::Timeout kResponseTimeoutMsec = 15000;
const nl::Weave::ExchangeContext::Timeout kWRMPActiveRetransTimeoutMsec = 3000;
const nl::Weave::ExchangeContext::Timeout kWRMPInitialRetransTimeoutMsec = 3000;
const uint16_t kWRMPMaxRetrans = 3;
const uint16_t kWRMPAckTimeoutMsec = 200;
const uint16_t ksignatureType = 1;
const nl::Weave::Profiles::Time::timesync_t kUpdateTimeoutMicroSecs = 30 * nl::kMicrosecondsPerSecond;


static nl::Weave::WRMPConfig gWRMPConfig = { kWRMPInitialRetransTimeoutMsec, kWRMPActiveRetransTimeoutMsec, kWRMPAckTimeoutMsec, kWRMPMaxRetrans };

/**
 *  Log the specified message in the form of @a aFormat.
 *
 *  @param[in]     aFormat   A pointer to a NULL-terminated C string with
 *                           C Standard Library-style format specifiers
 *                           containing the log message to be formatted and
 *                           logged.
 *  @param[in]     ...       An argument list whose elements should correspond
 *                           to the format specifiers in @a aFormat.
 *
 */
static void TLVPrettyPrinter(const char *aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    // There is no proper Weave logging routine for us to use here
    vprintf(aFormat, args);

    va_end(args);
}

static WEAVE_ERROR DebugPrettyPrint(nl::Weave::TLV::TLVReader & aReader)
{
    return nl::Weave::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
}

class MockWdmUpdateClientImpl: public MockWdmUpdateClient
{
public:
    MockWdmUpdateClientImpl();

    virtual WEAVE_ERROR Init (nl::Weave::WeaveExchangeManager *aExchangeMgr, const char * const aTestCaseId, const int aTestSecurityMode,
                                            const uint32_t aKeyId);

    virtual WEAVE_ERROR StartTesting(const uint64_t aPublisherNodeId, const uint16_t aSubnetId);

private:
    nl::Weave::WeaveExchangeManager *mExchangeMgr;
    uint64_t mPublisherNodeId;
    uint16_t mPublisherSubnetId;
    Binding * mBinding;
    UpdateClient mUpdateClient;

    WEAVE_ERROR PrepareBinding(void);

    WEAVE_ERROR SendUpdateRequest(void);

    int mTestCaseId;
    int mTestSecurityMode;
    uint32_t mKeyId;
    TraitPath mTraitPaths[1];

    static void EventCallback (void * const aAppState, UpdateClient::EventType aEvent, const UpdateClient::InEventParam & aInParam, UpdateClient::OutEventParam & aOutParam);

    static WEAVE_ERROR AddArgumentCallback(UpdateClient * apClient, void * const aAppState, nl::Weave::TLV::TLVWriter &aOutWriter);

    enum {
        kLocaleSettingsSinkIndex = 0,
    };

    LocaleSettingsTraitDataSink mLocaleSettingsTraitDataSink;
    SingleResourceSinkTraitCatalog mSinkCatalog;
    SingleResourceTraitCatalog<TraitDataSink>::CatalogItem mSinkCatalogStore[1];

    static void BindingEventCallback (void * const apAppState, const nl::Weave::Binding::EventType aEvent,
        const nl::Weave::Binding::InEventParam & aInParam, nl::Weave::Binding::OutEventParam & aOutParam);
};

static MockWdmUpdateClientImpl gWdmUpdateClient;

MockWdmUpdateClient * MockWdmUpdateClient::GetInstance ()
{
    return &gWdmUpdateClient;
}

MockWdmUpdateClientImpl::MockWdmUpdateClientImpl()
    : mSinkCatalog(ResourceIdentifier(ResourceIdentifier::SELF_NODE_ID), mSinkCatalogStore, sizeof(mSinkCatalogStore) / sizeof(mSinkCatalogStore[0]))
{
    mSinkCatalog.AddAt(0, &mLocaleSettingsTraitDataSink, kLocaleSettingsSinkIndex);
}

WEAVE_ERROR MockWdmUpdateClientImpl::Init (nl::Weave::WeaveExchangeManager *aExchangeMgr,
                                            const char * const aTestCaseId,
                                            const int aTestSecurityMode,
                                            const uint32_t aKeyId)
{
    mExchangeMgr = aExchangeMgr;
    mBinding = NULL;
    onCompleteTest = NULL;

    if (NULL != aTestCaseId)
    {
        mTestCaseId = atoi(aTestCaseId);
    }
    else
    {
        mTestCaseId = 0;
    }

    mTestSecurityMode = aTestSecurityMode;
    mKeyId = aKeyId;

    WeaveLogDetail(DataManagement, "Test Case ID: %d", mTestCaseId);
    WeaveLogDetail(DataManagement, "Security Mode: %d", mTestSecurityMode);
    WeaveLogDetail(DataManagement, "Key ID: %d", aKeyId);

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR MockWdmUpdateClientImpl::StartTesting(const uint64_t aPublisherNodeId, const uint16_t aSubnetId)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    mPublisherNodeId = aPublisherNodeId;
    mPublisherSubnetId = aSubnetId;

    mBinding = mExchangeMgr->NewBinding(BindingEventCallback, this);
    VerifyOrExit(NULL != mBinding, err = WEAVE_ERROR_NO_MEMORY);

    if (mBinding->CanBePrepared())
    {
        err = mBinding->RequestPrepare();
        SuccessOrExit(err);
    }

    mTraitPaths[0].mTraitDataHandle = kLocaleSettingsSinkIndex;
    mTraitPaths[0].mPropertyPathHandle = kRootPropertyPathHandle;

exit:
    WeaveLogFunctError(err);

    if (err != WEAVE_NO_ERROR && mBinding != NULL)
    {
        mBinding->Release();
        mBinding = NULL;
    }
    return err;
}

WEAVE_ERROR MockWdmUpdateClientImpl::AddArgumentCallback(UpdateClient * apClient, void * const aAppState, nl::Weave::TLV::TLVWriter &aOutWriter)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    uint32_t dummyUInt = 7;
    bool dummyBool = false;
    nl::Weave::TLV::TLVType dummyType = nl::Weave::TLV::kTLVType_NotSpecified;

    err = aOutWriter.StartContainer(nl::Weave::TLV::ContextTag(UpdateRequest::kCsTag_Argument), nl::Weave::TLV::kTLVType_Structure, dummyType);
    SuccessOrExit(err);

    err = aOutWriter.Put(nl::Weave::TLV::ContextTag(1), dummyUInt);
    SuccessOrExit(err);
    err = aOutWriter.PutBoolean(nl::Weave::TLV::ContextTag(2), dummyBool);
    SuccessOrExit(err);

    err = aOutWriter.EndContainer(dummyType);
    SuccessOrExit(err);

exit:
    WeaveLogFunctError(err);

    return err;
}

WEAVE_ERROR MockWdmUpdateClientImpl::PrepareBinding()
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    Binding::Configuration bindingConfig = mBinding->BeginConfiguration()
        .Target_NodeId(mPublisherNodeId) // TODO: aPublisherNodeId
        .Transport_UDP_WRM()
        .Transport_DefaultWRMPConfig(gWRMPConfig)

        // (default) max num of msec between any outgoing message and next incoming message (could be a response to it)
        .Exchange_ResponseTimeoutMsec(kResponseTimeoutMsec);

    if (nl::Weave::kWeaveSubnetId_NotSpecified != mPublisherSubnetId)
    {
        bindingConfig.TargetAddress_WeaveFabric(mPublisherSubnetId);
    }

    switch (mTestSecurityMode)
    {
    case WeaveSecurityMode::kCASE:
        WeaveLogDetail(DataManagement, "security mode is kWdmSecurity_CASE");
        bindingConfig.Security_SharedCASESession();
        break;

    case WeaveSecurityMode::kGroupEnc:
        WeaveLogDetail(DataManagement, "security mode is kWdmSecurity_GroupKey");
        if (mKeyId == WeaveKeyId::kNone)
        {
            WeaveLogDetail(DataManagement, "Please specify a group encryption key id using the --group-enc-... options.\n");
            err = WEAVE_ERROR_INVALID_KEY_ID;
            SuccessOrExit(err);
        }
        bindingConfig.Security_Key(mKeyId);
        //.Security_Key(0x5536);
        //.Security_Key(0x4436);
        break;

    case WeaveSecurityMode::kNone:
        WeaveLogDetail(DataManagement, "security mode is None");
        bindingConfig.Security_None();
        break;

    default:
        WeaveLogDetail(DataManagement, "security mode is not supported");
        err = WEAVE_ERROR_UNSUPPORTED_AUTH_MODE;
        SuccessOrExit(err);
    }

    err = bindingConfig.PrepareBinding();
    SuccessOrExit(err);

exit:
    WeaveLogFunctError(err);
    return err;
}

WEAVE_ERROR MockWdmUpdateClientImpl::SendUpdateRequest()
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    PacketBuffer * buf = NULL;
    err = mUpdateClient.Init(mBinding, this, EventCallback);
    SuccessOrExit(err);

    {
        TLVWriter outWriter;
        err = mUpdateClient.StartUpdate(buf, 0, NULL);
        SuccessOrExit(err);

        err = mUpdateClient.StartElement(LocaleSettingsTrait::kWeaveProfileId, 0x0, 0x0, 0x0, NULL, NULL, 0, outWriter);
        SuccessOrExit(err);

        {
            err = outWriter.PutString(nl::Weave::TLV::ContextTag(1), "en_gb");
            if (err == WEAVE_NO_ERROR)
            {
                err = mUpdateClient.FinalizeElement(outWriter);
            }
            SuccessOrExit(err);
        }
    }

    err = mUpdateClient.SendUpdate();
    buf = NULL;
    SuccessOrExit(err);

exit:

    if (buf != NULL)
    {
        PacketBuffer::Free(buf);
    }

    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogDetail(DataManagement, "Fail in update request");
        mUpdateClient.Shutdown();
    }

    WeaveLogFunctError(err);
    return err;
}

void MockWdmUpdateClientImpl::BindingEventCallback (void * const apAppState, const nl::Weave::Binding::EventType aEvent,
    const nl::Weave::Binding::InEventParam & aInParam, nl::Weave::Binding::OutEventParam & aOutParam)
{

    WEAVE_ERROR err = WEAVE_NO_ERROR;

    WeaveLogDetail(DataManagement, "%s: Event(%d)", __func__, aEvent);

    MockWdmUpdateClientImpl * const initiator = reinterpret_cast<MockWdmUpdateClientImpl *>(apAppState);

    switch (aEvent)
    {
    case nl::Weave::Binding::kEvent_PrepareRequested:
        WeaveLogDetail(DataManagement, "kEvent_PrepareRequested");
        err = initiator->PrepareBinding();
        SuccessOrExit(err);
        break;

    case nl::Weave::Binding::kEvent_PrepareFailed:
        err = aInParam.PrepareFailed.Reason;
        WeaveLogDetail(DataManagement, "kEvent_PrepareFailed: reason");
        break;

    case nl::Weave::Binding::kEvent_BindingFailed:
        err = aInParam.BindingFailed.Reason;
        WeaveLogDetail(DataManagement, "kEvent_BindingFailed: reason");
        break;

    case nl::Weave::Binding::kEvent_BindingReady:
        WeaveLogDetail(DataManagement, "kEvent_BindingReady");
        err = initiator->SendUpdateRequest();
        break;

    case nl::Weave::Binding::kEvent_DefaultCheck:
        WeaveLogDetail(DataManagement, "kEvent_DefaultCheck");

    default:
        nl::Weave::Binding::DefaultEventHandler(apAppState, aEvent, aInParam, aOutParam);
    }

exit:

    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogDetail(DataManagement, "error in BindingEventCallback");
        initiator->mBinding->Release();
        initiator->mBinding = NULL;
    }

    WeaveLogFunctError(err);
}

void MockWdmUpdateClientImpl::EventCallback (void * const aAppState, UpdateClient::EventType aEvent, const UpdateClient::InEventParam & aInParam, UpdateClient::OutEventParam & aOutParam)
{
    MockWdmUpdateClientImpl * const initiator = reinterpret_cast<MockWdmUpdateClientImpl *>(aAppState);

    switch (aEvent)
    {
    case UpdateClient::kEvent_UpdateComplete:

        if (WEAVE_NO_ERROR == aInParam.UpdateComplete.Reason)
        {
            WeaveLogDetail(DataManagement, "kEvent_StatusReportReceived");

            nl::Weave::TLV::TLVReader reader;
            reader.Init(aInParam.UpdateComplete.mMessage);
            DebugPrettyPrint(reader);
            WeaveLogDetail(DataManagement, "Update: Good Iteration");
        }
        else
        {
            WeaveLogDetail(DataManagement, "Update: Bad Iteration");
        }
        break;

    default:
        WeaveLogDetail(DataManagement, "Unknown UpdateClient event: %d", aEvent);
        break;
    }

    initiator->mUpdateClient.Shutdown();
    initiator->onCompleteTest();
}

#endif // WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
