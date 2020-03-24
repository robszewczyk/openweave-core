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
 *      This file implements the command handler for the 'weave' tool
 *      that generates a Weave device certificate.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#include "weave-tool.h"

using namespace nl::Weave::Profiles::Security;
using namespace nl::Weave::ASN1;

#define CMD_NAME "weave gen-device-cert"

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

static OptionDef gCmdOptionDefs[] = { { "dev-id", kArgumentRequired, 'i' },   { "key", kArgumentRequired, 'k' },
                                      { "ca-cert", kArgumentRequired, 'C' },  { "ca-key", kArgumentRequired, 'K' },
                                      { "out", kArgumentRequired, 'o' },      { "out-key", kArgumentRequired, 'O' },
                                      { "curve", kArgumentRequired, 'u' },    { "valid-from", kArgumentRequired, 'V' },
                                      { "lifetime", kArgumentRequired, 'l' }, { "sha1", kNoArgument, '1' },
                                      { "sha256", kNoArgument, '2' },         { } };

static const char * const gCmdOptionHelp =
    "   -i, --dev-id <hex-digits>\n"
    "\n"
    "       The device id (in hex) for which the certificate should be generated.\n"
    "\n"
    "   -C, --ca-cert <file>\n"
    "\n"
    "       File containing CA certificate to be used to sign the device certificate\n"
    "       (in PEM format).\n"
    "\n"
    "   -K, --ca-key <file>\n"
    "\n"
    "       File containing CA private key to be used to sign the device certificate\n"
    "       (in PEM format).\n"
    "\n"
    "   -k, --key <file>\n"
    "\n"
    "       File containing the public and private keys for the new certificate (in\n"
    "       PEM format). If not specified, a new key pair will be generated.\n"
    "\n"
    "   -o, --out <file>\n"
    "\n"
    "       File to contain the new device certificate. (Will be written in Weave base-64\n"
    "       format).\n"
    "\n"
    "   -O, --out-key <file>\n"
    "\n"
    "       File to contain the public/private for the new device certificate (in Weave\n"
    "       base-64 format). This option must be specified if the --key option is not.\n"
    "\n"
    "   -V, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]\n"
    "\n"
    "       The start date for the certificate's validity period.  If not specified,\n"
    "       the validity period starts on the current day.\n"
    "\n"
    "   -l, --lifetime <days>\n"
    "\n"
    "       The lifetime for the new certificate, in whole days.\n"
    "\n"
    "   -u, --curve <elliptic-curve-name>\n"
    "\n"
    "       The elliptic curve to use when generating the public/private keys. This option\n"
    "       must be specified if the --key option is not given.\n"
    "\n"
    "   -1, --sha1\n"
    "\n"
    "       Sign the certificate using a SHA-1 hash.\n"
    "\n"
    "   -2, --sha256\n"
    "\n"
    "       Sign the certificate using a SHA-256 hash.\n"
    "\n";

static OptionSet gCmdOptions = { HandleOption, gCmdOptionDefs, "COMMAND OPTIONS", gCmdOptionHelp };

static HelpOptions gHelpOptions(CMD_NAME, "Usage: " CMD_NAME " [ <options...> ]\n", WEAVE_VERSION_STRING "\n" COPYRIGHT_STRING,
                                "Generate a Weave device certificate");

static OptionSet * gCmdOptionSets[] = { &gCmdOptions, &gHelpOptions, NULL };

static uint64_t gDevId                 = 0;
static const char * gCurveName         = NULL;
static const char * gCACertFileName    = NULL;
static const char * gCAKeyFileName     = NULL;
static const char * gInCertKeyFileName = NULL;
static const char * gOutCertFileName   = NULL;
static const char * gOutKeyFileName    = NULL;
static int32_t gValidDays              = 0;
static const EVP_MD * gSigHashAlgo     = NULL;
static struct tm gValidFrom;

bool Cmd_GenDeviceCert(int argc, char * argv[])
{
    bool res            = true;
    int curveNID        = 0;
    FILE * outCertFile  = NULL;
    FILE * outKeyFile   = NULL;
    X509 * caCert       = NULL;
    X509 * devCert      = NULL;
    EVP_PKEY * caKey    = NULL;
    EVP_PKEY * devKey   = NULL;
    uint8_t * weaveCert = NULL;
    uint32_t weaveCertLen;
    char * weaveCertB64 = NULL;
    uint8_t * weaveKey  = NULL;
    uint32_t weaveKeyLen;
    char * weaveKeyB64   = NULL;
    bool certFileCreated = false;
    bool keyFileCreated  = false;

    {
        time_t now         = time(NULL);
        gValidFrom         = *gmtime(&now);
        gValidFrom.tm_hour = 0;
        gValidFrom.tm_min  = 0;
        gValidFrom.tm_sec  = 0;
    }

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        ExitNow(res = true);
    }

    if (!ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets))
    {
        ExitNow(res = false);
    }

    if (gDevId == 0)
    {
        fprintf(stderr, "Please specify the device id using the --dev-id option.\n");
        ExitNow(res = false);
    }

    if (gCACertFileName == NULL)
    {
        fprintf(stderr, "Please specify the CA certificate file name using the --ca-cert option.\n");
        ExitNow(res = false);
    }

    if (gCAKeyFileName == NULL)
    {
        fprintf(stderr, "Please specify the CA key file name using the --ca-key option.\n");
        ExitNow(res = false);
    }

    if (gOutCertFileName == NULL)
    {
        fprintf(stderr, "Please specify the file name for the new device certificate using the --out option.\n");
        ExitNow(res = false);
    }

    if (gInCertKeyFileName == NULL && gOutKeyFileName == NULL)
    {
        fprintf(stderr, "Please specify the file name for the new public/private key using the --out-key option.\n");
        ExitNow(res = false);
    }

    if (gValidDays == 0)
    {
        fprintf(stderr, "Please specify the lifetime for the new CA certificate (in days) using the --lifetime option.\n");
        ExitNow(res = false);
    }

    if (gInCertKeyFileName == NULL && gCurveName == NULL)
    {
        fprintf(stderr, "Please specify an elliptic curve using the --curve option.\n");
        ExitNow(res = false);
    }

    if (gCurveName != NULL)
    {
        curveNID = OBJ_sn2nid(gCurveName);
        if (curveNID == 0)
        {
            fprintf(stderr, "Unknown elliptic curve: %s\n", gCurveName);
            ExitNow(res = false);
        }
    }

    if (gSigHashAlgo == NULL)
    {
        fprintf(stderr, "Please specify a signature hash algorithm using either the --sha1 or --sha256 options.\n");
        ExitNow(res = false);
    }

    if (access(gOutCertFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "weave: ERROR: Output certificate file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutCertFileName);
        ExitNow(res = false);
    }

    if (gOutKeyFileName != NULL && access(gOutKeyFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "weave: ERROR: Output key file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gOutKeyFileName);
        ExitNow(res = false);
    }

    outCertFile = fopen(gOutCertFileName, "w+");
    if (outCertFile == NULL)
    {
        fprintf(stderr,
                "weave: ERROR: Unable to create output certificate file (%s)\n"
                "%s.\n",
                gOutCertFileName, strerror(errno));
        ExitNow(res = false);
    }
    certFileCreated = true;

    if (gOutKeyFileName != NULL)
    {
        outKeyFile = fopen(gOutKeyFileName, "w+");
        if (outKeyFile == NULL)
        {
            fprintf(stderr,
                    "weave: ERROR: Unable to create output key file (%s)\n"
                    "%s.\n",
                    gOutKeyFileName, strerror(errno));
            ExitNow(res = false);
        }
    }
    keyFileCreated = true;

    if (!InitOpenSSL())
        ExitNow(res = false);

    if (!ReadCertPEM(gCACertFileName, caCert))
        ExitNow(res = false);

    if (gInCertKeyFileName != NULL)
    {
        if (!ReadPrivateKey(gInCertKeyFileName, "Enter password for private key:", devKey))
            ExitNow(res = false);

        // If a curve name was specified on the command line, verify that it matches the curve
        // of the private key.
        if (gCurveName != NULL && EC_GROUP_get_curve_name(EC_KEY_get0_group(EVP_PKEY_get1_EC_KEY(devKey))) != curveNID)
        {
            fprintf(stderr, "weave: ERROR: Value given for --curve option does not match specified key (%s)\n", gCurveName);
            ExitNow(res = false);
        }
    }

    if (!ReadPrivateKey(gCAKeyFileName, "Enter password for CA certificate:", caKey))
        ExitNow(res = false);

    if (!MakeDeviceCert(gDevId, caCert, caKey, gCurveName, gValidFrom, gValidDays, gSigHashAlgo, devCert, devKey))
        ExitNow(res = false);

    if (!WeaveEncodeCert(devCert, weaveCert, weaveCertLen))
        ExitNow(res = false);

    if (!EncodePrivateKey(devKey, kKeyFormat_Weave_Raw, weaveKey, weaveKeyLen))
        ExitNow(res = false);

    weaveCertB64 = Base64Encode(weaveCert, weaveCertLen);
    if (weaveCertB64 == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        ExitNow(res = false);
    }

    weaveKeyB64 = Base64Encode(weaveKey, weaveKeyLen);
    if (weaveKeyB64 == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        ExitNow(res = false);
    }

    if (fputs(weaveCertB64, outCertFile) == EOF)
    {
        fprintf(stderr,
                "weave: ERROR: Unable to write output certificate file (%s)\n"
                "%s.\n",
                gOutCertFileName, strerror(errno));
        ExitNow(res = false);
    }

    if (outKeyFile != NULL && fputs(weaveKeyB64, outKeyFile) == EOF)
    {
        fprintf(stderr,
                "weave: ERROR: Unable to write output key file (%s)\n"
                "%s.\n",
                gOutKeyFileName, strerror(errno));
        ExitNow(res = false);
    }

    res         = (fclose(outCertFile) != EOF);
    outCertFile = NULL;
    if (!res)
    {
        fprintf(stderr,
                "weave: ERROR: Unable to write output certificate file (%s)\n"
                "%s.\n",
                gOutCertFileName, strerror(errno));
        ExitNow();
    }

    if (outKeyFile != NULL)
    {
        res        = (fclose(outKeyFile) != EOF);
        outKeyFile = NULL;
        if (!res)
        {
            fprintf(stderr,
                    "weave: ERROR: Unable to write output key file (%s)\n"
                    "%s.\n",
                    gOutKeyFileName, strerror(errno));
            ExitNow();
        }
    }

exit:
    if (weaveCertB64 != NULL)
        free(weaveCertB64);
    if (weaveKeyB64 != NULL)
        free(weaveKeyB64);
    if (caCert != NULL)
        X509_free(caCert);
    if (devCert != NULL)
        X509_free(devCert);
    if (caKey != NULL)
        EVP_PKEY_free(caKey);
    if (devKey != NULL)
        EVP_PKEY_free(devKey);
    if (weaveCert != NULL)
        free(weaveCert);
    if (weaveKey != NULL)
        free(weaveKey);
    if (outCertFile != NULL)
        fclose(outCertFile);
    if (outKeyFile != NULL)
        fclose(outKeyFile);
    if (gOutCertFileName != NULL && certFileCreated && !res)
        unlink(gOutCertFileName);
    if (gOutKeyFileName != NULL && keyFileCreated && !res)
        unlink(gOutKeyFileName);
    return res;
}

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'i':
        if (!ParseEUI64(arg, gDevId))
        {
            PrintArgError("%s: Invalid value specified for device id: %s\n", progName, arg);
            return false;
        }
        break;
    case 'k': gInCertKeyFileName = arg; break;
    case 'C': gCACertFileName = arg; break;
    case 'K': gCAKeyFileName = arg; break;
    case 'o': gOutCertFileName = arg; break;
    case 'O': gOutKeyFileName = arg; break;
    case 'u': gCurveName = arg; break;
    case 'V':
        if (!ParseDateTime(arg, gValidFrom))
        {
            PrintArgError("%s: Invalid value specified for certificate validity date: %s\n", progName, arg);
            return false;
        }
        break;
    case 'l':
        if (!ParseInt(arg, gValidDays) || gValidDays < 0)
        {
            PrintArgError("%s: Invalid value specified for certificate lifetime: %s\n", progName, arg);
            return false;
        }
        break;
    case '1': gSigHashAlgo = EVP_sha1(); break;
    case '2': gSigHashAlgo = EVP_sha256(); break;
    default: PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", progName, name); return false;
    }

    return true;
}
