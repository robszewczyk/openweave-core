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
 *      that generates a Weave Certificate Authority (CA) certificate.
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

#define CMD_NAME "weave gen-ca-cert"

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

static OptionDef gCmdOptionDefs[] = { { "id", kArgumentRequired, 'i' },
                                      { "key", kArgumentRequired, 'k' },
                                      { "ca-cert", kArgumentRequired, 'C' },
                                      { "ca-key", kArgumentRequired, 'K' },
                                      { "self", kNoArgument, 's' },
                                      { "out", kArgumentRequired, 'o' },
                                      { "valid-from", kArgumentRequired, 'V' },
                                      { "lifetime", kArgumentRequired, 'l' },
                                      { "sha1", kNoArgument, '1' },
                                      { "sha256", kNoArgument, '2' },
                                      { } };

static const char * const gCmdOptionHelp =
    "   -i, --id <hex-digits>\n"
    "\n"
    "       An EUI-64 (given in hex) that identifies the CA.\n"
    "\n"
    "   -k, --key <file>\n"
    "\n"
    "       File containing the public key for the new CA certificate. The file may\n"
    "       contain only a public key, or a private key (from which the public key\n"
    "       can be obtained). The file must be in PEM or DER format.\n"
    "\n"
    "   -C, --ca-cert <file>\n"
    "\n"
    "       File containing CA certificate to be used to sign the new CA certificate.\n"
    "       This file must be in PEM format.\n"
    "\n"
    "   -K, --ca-key <file>\n"
    "\n"
    "       File containing CA private key to be used to sign the new CA certificate.\n"
    "       This file may be in PEM or DER format.\n"
    "\n"
    "   -o, --out <file>\n"
    "\n"
    "       File to contain the new CA certificate.  This file will be written in PEM\n"
    "       format).\n"
    "\n"
    "   -s, --self\n"
    "\n"
    "       Generate a self-signed CA certificate.\n"
    "\n"
    "   -V, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]\n"
    "\n"
    "       The start date for the certificate's validity period.  If not specified,\n"
    "       the validity period starts on the current day.\n"
    "\n"
    "   -l, --lifetime <days>\n"
    "\n"
    "       The duration of the certificate's validity period, in whole days.\n"
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
                                "Generate a Weave CA certificate");

static OptionSet * gCmdOptionSets[] = { &gCmdOptions, &gHelpOptions, NULL };

static uint64_t gCertId                 = 0;
static const char * gCACertFileName     = NULL;
static const char * gCAKeyFileName      = NULL;
static const char * gNewCertFileName    = NULL;
static const char * gNewCertKeyFileName = NULL;
static int32_t gValidDays               = 0;
static bool gSelfSign                   = false;
static const EVP_MD * gSigHashAlgo      = NULL;
static struct tm gValidFrom;

bool Cmd_GenCACert(int argc, char * argv[])
{
    bool res              = true;
    X509 * caCert         = NULL;
    X509 * newCert        = NULL;
    EVP_PKEY * caKey      = NULL;
    EVP_PKEY * newCertKey = NULL;
    FILE * newCertFile    = NULL;
    bool certFileCreated  = false;

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

    if (gCertId == 0)
    {
        fprintf(stderr, "Please specify the id for the new certificate using the --id option.\n");
        ExitNow(res = false);
    }

    if (gNewCertKeyFileName == NULL)
    {
        fprintf(stderr,
                "Please use the --key option to specify the public/private key file for the\n"
                "new certificate.\n");
        ExitNow(res = false);
    }

    if (gCACertFileName == NULL && !gSelfSign)
    {
        fprintf(stderr,
                "Please specify a CA certificate to be used to sign the new certificate (using\n"
                "the --ca-cert option) or --self to generate a self-signed certificate.\n");
        ExitNow(res = false);
    }

    else if (gCACertFileName != NULL && gSelfSign)
    {
        fprintf(stderr, "Please specify only one of --ca-cert and --self.\n");
        ExitNow(res = false);
    }

    if (gCACertFileName != NULL && gCAKeyFileName == NULL)
    {
        fprintf(stderr,
                "Please use the the --ca-key option to specify the key file for the CA\n"
                "certificate that will be used to sign the new certificate.\n");
        ExitNow(res = false);
    }

    if (gNewCertFileName == NULL)
    {
        fprintf(stderr, "Please specify the file name for the new CA certificate using the --out option.\n");
        ExitNow(res = false);
    }

    if (gValidDays == 0)
    {
        fprintf(stderr, "Please specify the lifetime for the new CA certificate (in days) using the --lifetime option.\n");
        ExitNow(res = false);
    }

    if (gSigHashAlgo == NULL)
    {
        fprintf(stderr, "Please specify a signature hash algorithm using either the --sha1 or --sha256 options.\n");
        ExitNow(res = false);
    }

    if (strcmp(gNewCertFileName, "-") != 0 && access(gNewCertFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "weave: ERROR: Output file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gNewCertFileName);
        ExitNow(res = false);
    }

    if (!InitOpenSSL())
        ExitNow(res = false);

    if (strcmp(gNewCertFileName, "-") != 0)
    {
        newCertFile = fopen(gNewCertFileName, "w+");
        if (newCertFile == NULL)
        {
            fprintf(stderr,
                    "weave: ERROR: Unable to create output file (%s)\n"
                    "%s.\n",
                    gNewCertFileName, strerror(errno));
            ExitNow(res = false);
        }
        certFileCreated = true;
    }
    else
        newCertFile = stdout;

    if (!ReadPublicKey(gNewCertKeyFileName, "Enter password for private key:", newCertKey))
        ExitNow(res = false);

    if (!gSelfSign)
    {
        if (!ReadCertPEM(gCACertFileName, caCert))
            ExitNow(res = false);

        if (!ReadPrivateKey(gCAKeyFileName, "Enter password for signing CA certificate key:", caKey))
            ExitNow(res = false);
    }
    else
        caKey = newCertKey;

    if (!MakeCACert(gCertId, newCertKey, caCert, caKey, gValidFrom, gValidDays, gSigHashAlgo, newCert))
        ExitNow(res = false);

    if (!PEM_write_X509(newCertFile, newCert))
        ReportOpenSSLErrorAndExit("PEM_write_X509", res = false);

    res         = (newCertFile == stdout || fclose(newCertFile) != EOF);
    newCertFile = NULL;
    if (!res)
    {
        fprintf(stderr,
                "weave: ERROR: Unable to write CA certificate file (%s)\n"
                "%s.\n",
                gNewCertFileName, strerror(errno));
        ExitNow();
    }

exit:
    if (newCert != NULL)
        X509_free(newCert);
    if (caCert != NULL)
        X509_free(caCert);
    if (caKey != NULL && caKey != newCertKey)
        EVP_PKEY_free(caKey);
    if (newCertKey != NULL)
        EVP_PKEY_free(newCertKey);
    if (newCertFile != NULL && newCertFile != stdout)
        fclose(newCertFile);
    if (gNewCertFileName != NULL && certFileCreated && !res)
        unlink(gNewCertFileName);
    return res;
}

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'i':
        if (!ParseEUI64(arg, gCertId))
        {
            PrintArgError("%s: Invalid value specified for certificate id: %s\n", progName, arg);
            return false;
        }
        break;
    case 'C': gCACertFileName = arg; break;
    case 'K': gCAKeyFileName = arg; break;
    case 'o': gNewCertFileName = arg; break;
    case 'k': gNewCertKeyFileName = arg; break;
    case 's': gSelfSign = true; break;
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
