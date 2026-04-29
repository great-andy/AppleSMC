/*
 * AppleSMC
 * Copyright (C) 2026 Great Andy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "AppleSMC.h"

#include <fnmatch.h>
#include <getopt.h>
#include <libgen.h>
#include <format>

size_t AppleSMC::ReadKeyCount()
{
    size_t count = 0;
    SMCValue returnValue;
    if (mSmcManager.ReadKey("#KEY", returnValue))
    {
        count = returnValue.GetUInt32();
    }
    return count;
}

void AppleSMC::PrintAll()
{
    size_t numKeys = ReadKeyCount();
    for (size_t i = 0; i < numKeys; i++)
    {
        SMCKeyData keyData;
        keyData. mCommand = SMCManager::SMC_CMD_READ_INDEX;
        keyData.mData32 = UInt32(i);

        SMCKeyData returnData;
        kern_return_t result = mSmcManager.IOConnectCall(keyData, returnData);
        if (result == kIOReturnSuccess)
        {
            std::string key = SMCUtils::UInt32ToString(returnData.mKey);
            if (!mPattern.empty() && !key.empty())
            {
                if (::fnmatch(mPattern.c_str(), key.c_str(), 0) != 0)
                {
                    continue;
                }
            }

            SMCValue returnValue;
            mSmcManager.ReadKeyRaw(key, returnValue);
            returnValue.Print();
        }
    }
}

void AppleSMC::PrintFans()
{
    size_t totalFans = 0;
    SMCValue returnValue;
    if (mSmcManager.ReadKey("FNum", returnValue))
    {
        totalFans = returnValue.GetUInt8();
        printf("Total fans in system: %d\n", SInt32(totalFans));
    }

    const UInt8 fanNumbers[] = "0123456789ABCDEFGHIJ";
    totalFans = std::min(totalFans, sizeof(fanNumbers) - 1);
    for (size_t i = 0; i < totalFans; i++)
    {
        printf("\nFan #%d:\n", SInt32(i));

        std::string key = std::format("F{}ID", fanNumbers[i]);
        SMCValue returnValue;
        mSmcManager.ReadKeyRaw(key, returnValue);
        if (returnValue.HasData())
        {
            std::string str = returnValue.GetString();
            str.erase(0, std::min(str.length(), size_t(4)));
            printf("    Fan ID       : %s\n", str.c_str());
        }

        key = std::format("F{}Ac", fanNumbers[i]);
        mSmcManager.ReadKeyRaw(key, returnValue);
        printf("    Current speed: %.0f\n", returnValue.GetFloatFromType());

        key = std::format("F{}Mn", fanNumbers[i]);
        mSmcManager.ReadKeyRaw(key, returnValue);
        printf("    Minimum speed: %.0f\n", returnValue.GetFloatFromType());

        key = std::format("F{}Mx", fanNumbers[i]);
        mSmcManager.ReadKeyRaw(key, returnValue);
        printf("    Maximum speed: %.0f\n", returnValue.GetFloatFromType());

        key = std::format("F{}Sf", fanNumbers[i]);
        mSmcManager.ReadKeyRaw(key, returnValue);
        printf("    Safe speed   : %.0f\n", returnValue.GetFloatFromType());

        key = std::format("F{}Tg", fanNumbers[i]);
        mSmcManager.ReadKeyRaw(key, returnValue);
        printf("    Target speed : %.0f\n", returnValue.GetFloatFromType());

        bool isForced = false;
        mSmcManager.ReadKeyRaw("FS! ", returnValue);
        if (returnValue.HasData())
        {
            isForced = ((returnValue.GetUInt16() & (1 << i)) != 0);
        }
        else
        {
            key = std::format("F{}Md", fanNumbers[i]);
            mSmcManager.ReadKeyRaw(key, returnValue);
            isForced = (returnValue.GetFloatFromType() != 0);
        }
        printf("    Mode         : %s\n", isForced ? "forced" : "auto");
    }
}

void AppleSMC::PrintVersion() const
{
#ifdef APP_VERSION
    const char* version = APP_VERSION;
#else
    const char* version = "V0.0-unknown";
#endif
    printf("%s version %s\n", mAppName.c_str(), version);
}

void AppleSMC::PrintUsage() const
{
    printf("Apple System Management Control tool\n");
    printf("Usage: %s [options]\n", mAppName.c_str());
    printf("Options:\n");
    printf("  -l['*|?'], --list=['*|?']  List all keys and values, optional pattern matching\n");
    printf("  -f,        --fans          List fans info decoded\n");
    printf("  -k,        --key <key>     Set key to read / write\n");
    printf("  -r[key],   --read=[key]    Read the value of a key, optional set key\n");
    printf("  -w,        --write <value> Write the hex value to a key\n");
    printf("  -h,        --help          Print this message and exit.\n");
    printf("  -v,        --version       Print version and exit.\n");
}

void AppleSMC::ParseOptions(int argc, char* const* argv)
{
    if (argc > 0)
    {
        mAppName = ::basename(argv[0]);

        struct ::option longOptions[] =
        {
            {"list",    optional_argument, 0, 'l'},
            {"fans",    no_argument,       0, 'f'},
            {"key",     required_argument, 0, 'k'},
            {"read",    optional_argument, 0, 'r'},
            {"write",   required_argument, 0, 'w'},
            {"help",    no_argument,       0, 'h'},
            {"version", no_argument,       0, 'v'},
            {0, 0, 0, 0}
        };

        ::opterr = 0;

        int opt;
        while ((opt = ::getopt_long(argc, argv, "l::fk:r::w:hv", longOptions, NULL)) != -1)
        {
            switch (opt)
            {
                case 'l':
                    mOperation = OP_LIST;
                    if (optarg)
                    {
                        mPattern = optarg;
                    }
                    break;
                case 'f':
                    mOperation = OP_READ_FANS;
                    break;

                case 'k':
                    mKey = ::optarg;
                    break;
                case 'r':
                    mOperation = OP_READ;
                    if (optarg)
                    {
                        mKey = optarg;
                    }
                    break;
                case 'w':
                    mVal = ::optarg;
                    mOperation = OP_WRITE;
                    break;

                case 'v':
                    mOperation = OP_VERSION;
                    break;
                case 'h':
                    mOperation = OP_HELP;
                    break;

                case '?':
                    if (::optopt != 0)
                    {
                        printf("Invalid option -%c\n", ::optopt);
                    }
                    else if ((::optind > 0) && (::optind <= argc))
                    {
                        printf("Invalid option %s\n", argv[::optind - 1]);
                    }
                    mOperation = OP_NONE;
                    break;

                default:
                    break;
            }
        }

        for (int i = ::optind; i < argc; i++)
        {
            printf("Invalid option %s\n", argv[i]);
            mOperation = OP_NONE;
        }
    }
}

bool AppleSMC::Execute()
{
    bool success = true;

    if ((mOperation >= OP_LIST) && (mOperation <= OP_WRITE))
    {
        if (!mSmcManager.Open())
        {
            mOperation = OP_NONE;
            success = false;
        }
    }

    switch (mOperation)
    {
        case OP_LIST:
            PrintAll();
            break;

        case OP_READ_FANS:
            PrintFans();
            break;

        case OP_READ:
        {

            if (!SMCUtils::IsKeySizeValid(mKey))
            {
                printf("Error: specify a valid key to read\n");
                success = false;
                break;
            }
            SMCValue returnValue;
            if (mSmcManager.ReadKey(mKey, returnValue))
            {
                returnValue.Print();
            }
            else
            {
                success = false;
            }
            break;
        }

        case OP_WRITE:
        {
            if (!SMCUtils::IsKeySizeValid(mKey))
            {
                printf("Error: specify a valid key to write\n");
                success = false;
                break;
            }
            if (mVal.empty())
            {
                printf("Error: specify a hex value to write\n");
                success = false;
                break;
            }
            if (!mSmcManager.WriteKey(mKey, mVal))
            {
                success = false;
            }
            break;
        }

        case OP_HELP:
            PrintUsage();
            break;

        case OP_VERSION:
            PrintVersion();
            break;

        default:
        case OP_NONE:
            if (success)
            {
                PrintUsage();
                success = false;
            }
            break;
    }

    return success;
}
