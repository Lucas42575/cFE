/*
**  GSC-18128-1, "Core Flight Executive Version 6.7"
**
**  Copyright (c) 2006-2019 United States Government as represented by
**  the Administrator of the National Aeronautics and Space Administration.
**  All Rights Reserved.
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**    http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
*/

/*
 * Test CCSDS Extended header accessors
 */

/*
 * Includes
 */
#include "utassert.h"
#include "ut_support.h"
#include "cfe_msg_api.h"
#include "test_msg_not.h"
#include "test_msg_utils.h"
#include "test_cfe_msg_ccsdsext.h"
#include "cfe_error.h"
#include "cfe_mission_cfg.h"
#include "cfe_platform_cfg.h"
#include "cfe_msg_defaults.h"
#include <string.h>

/*
 * Defines
 */
#define TEST_EDSVER_MAX 0x1F   /* Maximum value for EDS Version field */
#define TEST_SUBSYS_MAX 0x1FF  /* Maximum value for Subsystem field */
#define TEST_SYSTEM_MAX 0xFFFF /* Maximum value for System field */

#define TEST_DEFAULT_SUBSYS_MASK 0x100 /* Bits that can be set by default subsys if msgid V2 */

/* Extended header initialization specific coverage */
void Test_MSG_Init_Ext(void)
{
    CFE_MSG_Message_t       msg;
    CFE_SB_MsgId_Atom_t     msgidval_exp;
    CFE_MSG_HeaderVersion_t hdrver;
    CFE_MSG_Subsystem_t     subsys;
    CFE_MSG_EDSVersion_t    edsver;
    CFE_MSG_System_t        system;
    CFE_MSG_Endian_t        endian;
    bool                    is_v1;

    /* Get msgid version by checking if msgid sets header version */
    memset(&msg, 0xFF, sizeof(msg));
    ASSERT_EQ(CFE_MSG_SetMsgId(&msg, CFE_SB_ValueToMsgId(0)), CFE_SUCCESS);
    ASSERT_EQ(CFE_MSG_GetHeaderVersion(&msg, &hdrver), CFE_SUCCESS);
    is_v1 = (hdrver == 0);

    UT_Text("Set to all F's, msgid value = 0, and run with clearing");
    memset(&msg, 0xFF, sizeof(msg));
    msgidval_exp = 0;
    ASSERT_EQ(CFE_MSG_Init(&msg, CFE_SB_ValueToMsgId(msgidval_exp), sizeof(msg), true), CFE_SUCCESS);
    Test_MSG_PrintMsg(&msg, 0);

    /* Default EDS version check */
    ASSERT_EQ(CFE_MSG_GetEDSVersion(&msg, &edsver), CFE_SUCCESS);
    ASSERT_EQ(edsver, CFE_PLATFORM_EDSVER);

    /* Default subsystem check */
    ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, &subsys), CFE_SUCCESS);
    if (is_v1)
        ASSERT_EQ(subsys, CFE_PLATFORM_DEFAULT_SUBSYS);
    else
        ASSERT_EQ(subsys, CFE_PLATFORM_DEFAULT_SUBSYS & TEST_DEFAULT_SUBSYS_MASK);

    /* Default system check */
    ASSERT_EQ(CFE_MSG_GetSystem(&msg, &system), CFE_SUCCESS);
    ASSERT_EQ(system, CFE_MISSION_SPACECRAFT_ID);

    /* Default endian check */
    ASSERT_EQ(CFE_MSG_GetEndian(&msg, &endian), CFE_SUCCESS);
#if (CFE_PLATFORM_ENDIAN == CCSDS_LITTLE_ENDIAN)
    ASSERT_EQ(endian, CFE_MSG_Endian_Little);
#else
    ASSERT_EQ(endian, CFE_MSG_Endian_Big);
#endif

    /* Confirm the rest of the fields not already explicitly checked */
    ASSERT_EQ(Test_MSG_Ext_NotZero(&msg) & ~(MSG_EDSVER_FLAG | MSG_ENDIAN_FLAG | MSG_SUBSYS_FLAG | MSG_SYSTEM_FLAG), 0);

    UT_Text("Set to all 0, max msgid value, and run without clearing");
    memset(&msg, 0, sizeof(msg));
    msgidval_exp = CFE_PLATFORM_SB_HIGHEST_VALID_MSGID;
    ASSERT_EQ(CFE_MSG_Init(&msg, CFE_SB_ValueToMsgId(msgidval_exp), sizeof(msg), false), CFE_SUCCESS);
    Test_MSG_PrintMsg(&msg, 0);

    /* Default subsystem check */
    ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, &subsys), CFE_SUCCESS);
    if (is_v1)
        ASSERT_EQ(subsys, 0);
    else
        ASSERT_EQ(subsys, 0xFF);

    /* Confirm the rest of the fields not already explicitly checked */
    ASSERT_EQ(Test_MSG_Ext_NotZero(&msg) & ~MSG_SUBSYS_FLAG, 0);
}

void Test_MSG_EDSVersion(void)
{
    CFE_MSG_Message_t    msg;
    CFE_MSG_EDSVersion_t input[] = {0, TEST_EDSVER_MAX / 2, TEST_EDSVER_MAX};
    CFE_MSG_EDSVersion_t actual  = TEST_EDSVER_MAX;
    int                  i;

    UT_Text("Bad parameter tests, Null pointers and invalid (max valid + 1, max)");
    memset(&msg, 0, sizeof(msg));
    ASSERT_EQ(CFE_MSG_GetEDSVersion(NULL, &actual), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(actual, TEST_EDSVER_MAX);
    ASSERT_EQ(CFE_MSG_GetEDSVersion(&msg, NULL), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetEDSVersion(NULL, input[0]), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(CFE_MSG_SetEDSVersion(&msg, TEST_EDSVER_MAX + 1), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetEDSVersion(&msg, 0xFFFF), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);

    UT_Text("Set to all F's, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0xFF, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEDSVersion(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, TEST_EDSVER_MAX);
        ASSERT_EQ(CFE_MSG_SetEDSVersion(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEDSVersion(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == TEST_EDSVER_MAX)
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), MSG_EDSVER_FLAG);
        }
    }

    UT_Text("Set to all 0, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEDSVersion(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, 0);
        ASSERT_EQ(CFE_MSG_SetEDSVersion(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEDSVersion(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == 0)
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), MSG_EDSVER_FLAG);
        }
    }
}

void Test_MSG_Endian(void)
{
    CFE_MSG_Message_t msg;
    CFE_MSG_Endian_t  input[] = {CFE_MSG_Endian_Big, CFE_MSG_Endian_Little};
    CFE_MSG_Endian_t  actual  = 0;
    int               i;

    UT_Text("Bad parameter tests, Null pointers and invalid (CFE_MSG_Endian_Invalid, CFE_MSG_Endian_Little + 1");
    memset(&msg, 0, sizeof(msg));
    ASSERT_EQ(CFE_MSG_GetEndian(NULL, &actual), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(CFE_MSG_GetEndian(&msg, NULL), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetEndian(NULL, input[0]), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(CFE_MSG_SetEndian(&msg, CFE_MSG_Endian_Invalid), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetEndian(&msg, CFE_MSG_Endian_Little + 1), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);

    UT_Text("Set to all F's, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0xFF, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEndian(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, CFE_MSG_Endian_Little);
        ASSERT_EQ(CFE_MSG_SetEndian(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEndian(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == CFE_MSG_Endian_Little)
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), MSG_ENDIAN_FLAG);
        }
    }

    UT_Text("Set to all 0, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEndian(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, CFE_MSG_Endian_Big);
        ASSERT_EQ(CFE_MSG_SetEndian(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetEndian(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == CFE_MSG_Endian_Big)
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), MSG_ENDIAN_FLAG);
        }
    }
}

void Test_MSG_PlaybackFlag(void)
{
    CFE_MSG_Message_t      msg;
    CFE_MSG_PlaybackFlag_t input[] = {CFE_MSG_PlayFlag_Original, CFE_MSG_PlayFlag_Playback};
    CFE_MSG_PlaybackFlag_t actual  = 0;
    int                    i;

    UT_Text("Bad parameter tests, Null pointers and invalid (CFE_MSG_PlayFlag_Invalid, CFE_MSG_PlayFlag_Playback + 1");
    memset(&msg, 0, sizeof(msg));
    ASSERT_EQ(CFE_MSG_GetPlaybackFlag(NULL, &actual), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(actual, 0);
    ASSERT_EQ(CFE_MSG_GetPlaybackFlag(&msg, NULL), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetPlaybackFlag(NULL, input[0]), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(CFE_MSG_SetPlaybackFlag(&msg, CFE_MSG_PlayFlag_Invalid), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetPlaybackFlag(&msg, CFE_MSG_PlayFlag_Playback + 1), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);

    UT_Text("Set to all F's, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0xFF, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetPlaybackFlag(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, CFE_MSG_PlayFlag_Playback);
        ASSERT_EQ(CFE_MSG_SetPlaybackFlag(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetPlaybackFlag(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == CFE_MSG_PlayFlag_Playback)
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), MSG_PBACK_FLAG);
        }
    }

    UT_Text("Set to all 0, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetPlaybackFlag(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, CFE_MSG_PlayFlag_Original);
        ASSERT_EQ(CFE_MSG_SetPlaybackFlag(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetPlaybackFlag(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == CFE_MSG_PlayFlag_Original)
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), MSG_PBACK_FLAG);
        }
    }
}

void Test_MSG_Subsystem(void)
{
    CFE_MSG_Message_t   msg;
    CFE_MSG_Subsystem_t input[] = {0, TEST_SUBSYS_MAX / 2, TEST_SUBSYS_MAX};
    CFE_MSG_Subsystem_t actual  = TEST_SUBSYS_MAX;
    int                 i;

    UT_Text("Bad parameter tests, Null pointers and invalid (max valid + 1, max)");
    memset(&msg, 0, sizeof(msg));
    ASSERT_EQ(CFE_MSG_GetSubsystem(NULL, &actual), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(actual, TEST_SUBSYS_MAX);
    ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, NULL), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetSubsystem(NULL, input[0]), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(CFE_MSG_SetSubsystem(&msg, TEST_SUBSYS_MAX + 1), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetSubsystem(&msg, 0xFFFF), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);

    UT_Text("Set to all F's, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0xFF, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, TEST_SUBSYS_MAX);
        ASSERT_EQ(CFE_MSG_SetSubsystem(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == TEST_SUBSYS_MAX)
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), MSG_SUBSYS_FLAG);
        }
    }

    UT_Text("Set to all 0, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, 0);
        ASSERT_EQ(CFE_MSG_SetSubsystem(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSubsystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == 0)
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), MSG_SUBSYS_FLAG);
        }
    }
}

void Test_MSG_System(void)
{
    CFE_MSG_Message_t msg;
    CFE_MSG_ApId_t    input[] = {0, TEST_SYSTEM_MAX / 2, TEST_SYSTEM_MAX};
    CFE_MSG_ApId_t    actual  = TEST_SYSTEM_MAX;
    int               i;

    UT_Text("Bad parameter tests, Null pointers");
    memset(&msg, 0, sizeof(msg));
    ASSERT_EQ(CFE_MSG_GetSystem(NULL, &actual), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(actual, TEST_SYSTEM_MAX);
    ASSERT_EQ(CFE_MSG_GetSystem(&msg, NULL), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
    ASSERT_EQ(CFE_MSG_SetSystem(NULL, input[0]), CFE_MSG_BAD_ARGUMENT);
    ASSERT_EQ(Test_MSG_NotZero(&msg), 0);

    UT_Text("Set to all F's, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0xFF, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, TEST_SYSTEM_MAX);
        ASSERT_EQ(CFE_MSG_SetSystem(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == TEST_SYSTEM_MAX)
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotF(&msg), MSG_SYSTEM_FLAG);
        }
    }

    UT_Text("Set to all 0, various valid inputs");
    for (i = 0; i < sizeof(input) / sizeof(input[0]); i++)
    {
        memset(&msg, 0, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, 0);
        ASSERT_EQ(CFE_MSG_SetSystem(&msg, input[i]), CFE_SUCCESS);
        Test_MSG_PrintMsg(&msg, sizeof(msg));
        ASSERT_EQ(CFE_MSG_GetSystem(&msg, &actual), CFE_SUCCESS);
        ASSERT_EQ(actual, input[i]);
        if (input[i] == 0)
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), 0);
        }
        else
        {
            ASSERT_EQ(Test_MSG_NotZero(&msg), MSG_SYSTEM_FLAG);
        }
    }
}

/*
 * Test MSG ccsdsext
 */
void Test_MSG_CCSDSExt(void)
{
    MSG_UT_ADD_SUBTEST(Test_MSG_Init_Ext);
    MSG_UT_ADD_SUBTEST(Test_MSG_EDSVersion);
    MSG_UT_ADD_SUBTEST(Test_MSG_Endian);
    MSG_UT_ADD_SUBTEST(Test_MSG_PlaybackFlag);
    MSG_UT_ADD_SUBTEST(Test_MSG_Subsystem);
    MSG_UT_ADD_SUBTEST(Test_MSG_System);
}
