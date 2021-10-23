/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/* This driver supports the Nintendo Switch Pro controller.
   Code and logic contributed by Valve Corporation under the SDL zlib license.
*/
#include "../../SDL_internal.h"

#ifdef SDL_JOYSTICK_HIDAPI

#include "SDL_hints.h"
#include "SDL_events.h"
#include "SDL_timer.h"
#include "SDL_joystick.h"
#include "SDL_gamecontroller.h"
#include "../../SDL_hints_c.h"
#include "../SDL_sysjoystick.h"
#include "SDL_hidapijoystick_c.h"
#include "SDL_hidapi_rumble.h"


#ifdef SDL_JOYSTICK_HIDAPI_SWITCH

/* Define this if you want to log all packets from the controller */
/*#define DEBUG_SWITCH_PROTOCOL*/

/* Define this to get log output for rumble logic */
/*#define DEBUG_RUMBLE*/

/* The initialization sequence doesn't appear to work correctly on Windows unless
   the reads and writes are on the same thread.

   ... and now I can't reproduce this, so I'm leaving it in, but disabled for now.
 */
/*#define SWITCH_SYNCHRONOUS_WRITES*/

/* How often you can write rumble commands to the controller.
   If you send commands more frequently than this, you can turn off the controller
   in Bluetooth mode, or the motors can miss the command in USB mode.
 */
#define RUMBLE_WRITE_FREQUENCY_MS   30

/* How often you have to refresh a long duration rumble to keep the motors running */
#define RUMBLE_REFRESH_FREQUENCY_MS 50

#define SWITCH_GYRO_SCALE      14.2842f
#define SWITCH_ACCEL_SCALE     4096.f

typedef enum {
    k_eSwitchInputReportIDs_SubcommandReply       = 0x21,
    k_eSwitchInputReportIDs_FullControllerState   = 0x30,
    k_eSwitchInputReportIDs_SimpleControllerState = 0x3F,
    k_eSwitchInputReportIDs_CommandAck            = 0x81,
} ESwitchInputReportIDs;

typedef enum {
    k_eSwitchOutputReportIDs_RumbleAndSubcommand = 0x01,
    k_eSwitchOutputReportIDs_Rumble              = 0x10,
    k_eSwitchOutputReportIDs_Proprietary         = 0x80,
} ESwitchOutputReportIDs;

typedef enum {
    k_eSwitchSubcommandIDs_BluetoothManualPair = 0x01,
    k_eSwitchSubcommandIDs_RequestDeviceInfo   = 0x02,
    k_eSwitchSubcommandIDs_SetInputReportMode  = 0x03,
    k_eSwitchSubcommandIDs_SetHCIState         = 0x06,
    k_eSwitchSubcommandIDs_SPIFlashRead        = 0x10,
    k_eSwitchSubcommandIDs_SetPlayerLights     = 0x30,
    k_eSwitchSubcommandIDs_SetHomeLight        = 0x38,
    k_eSwitchSubcommandIDs_EnableIMU           = 0x40,
    k_eSwitchSubcommandIDs_SetIMUSensitivity   = 0x41,
    k_eSwitchSubcommandIDs_EnableVibration     = 0x48,
} ESwitchSubcommandIDs;

typedef enum {
    k_eSwitchProprietaryCommandIDs_Status    = 0x01,
    k_eSwitchProprietaryCommandIDs_Handshake = 0x02,
    k_eSwitchProprietaryCommandIDs_HighSpeed = 0x03,
    k_eSwitchProprietaryCommandIDs_ForceUSB  = 0x04,
    k_eSwitchProprietaryCommandIDs_ClearUSB  = 0x05,
    k_eSwitchProprietaryCommandIDs_ResetMCU  = 0x06,
} ESwitchProprietaryCommandIDs;

typedef enum {
    k_eSwitchDeviceInfoControllerType_Unknown        = 0x0,
    k_eSwitchDeviceInfoControllerType_JoyConLeft     = 0x1,
    k_eSwitchDeviceInfoControllerType_JoyConRight    = 0x2,
    k_eSwitchDeviceInfoControllerType_ProController  = 0x3,
} ESwitchDeviceInfoControllerType;

#define k_unSwitchOutputPacketDataLength 49
#define k_unSwitchMaxOutputPacketLength  64
#define k_unSwitchBluetoothPacketLength  k_unSwitchOutputPacketDataLength
#define k_unSwitchUSBPacketLength        k_unSwitchMaxOutputPacketLength

#define k_unSPIStickCalibrationStartOffset  0x603D
#define k_unSPIStickCalibrationEndOffset    0x604E
#define k_unSPIStickCalibrationLength       (k_unSPIStickCalibrationEndOffset - k_unSPIStickCalibrationStartOffset + 1)

#pragma pack(1)
typedef struct
{
    Uint8 rgucButtons[2];
    Uint8 ucStickHat;
    Uint8 rgucJoystickLeft[2];
    Uint8 rgucJoystickRight[2];
} SwitchInputOnlyControllerStatePacket_t;

typedef struct
{
    Uint8 rgucButtons[2];
    Uint8 ucStickHat;
    Sint16 sJoystickLeft[2];
    Sint16 sJoystickRight[2];
} SwitchSimpleStatePacket_t;

typedef struct
{
    Uint8 ucCounter;
    Uint8 ucBatteryAndConnection;
    Uint8 rgucButtons[3];
    Uint8 rgucJoystickLeft[3];
    Uint8 rgucJoystickRight[3];
    Uint8 ucVibrationCode;
} SwitchControllerStatePacket_t;

typedef struct
{
    SwitchControllerStatePacket_t controllerState;

    struct {
        Sint16 sAccelX;
        Sint16 sAccelY;
        Sint16 sAccelZ;

        Sint16 sGyroX;
        Sint16 sGyroY;
        Sint16 sGyroZ;
    } imuState[3];
} SwitchStatePacket_t;

typedef struct
{
    Uint32 unAddress;
    Uint8 ucLength;
} SwitchSPIOpData_t;

typedef struct
{
    SwitchControllerStatePacket_t m_controllerState;

    Uint8 ucSubcommandAck;
    Uint8 ucSubcommandID;

    #define k_unSubcommandDataBytes 35
    union {
        Uint8 rgucSubcommandData[k_unSubcommandDataBytes];

        struct {
            SwitchSPIOpData_t opData;
            Uint8 rgucReadData[k_unSubcommandDataBytes - sizeof(SwitchSPIOpData_t)];
        } spiReadData;

        struct {
            Uint8 rgucFirmwareVersion[2];
            Uint8 ucDeviceType;
            Uint8 ucFiller1;
            Uint8 rgucMACAddress[6];
            Uint8 ucFiller2;
            Uint8 ucColorLocation;
        } deviceInfo;
    };
} SwitchSubcommandInputPacket_t;

typedef struct
{
	Uint8 ucPacketType;
	Uint8 ucCommandID;
	Uint8 ucFiller;

	Uint8 ucDeviceType;
	Uint8 rgucMACAddress[6];
} SwitchProprietaryStatusPacket_t;

typedef struct
{
    Uint8 rgucData[4];
} SwitchRumbleData_t;

typedef struct
{
    Uint8 ucPacketType;
    Uint8 ucPacketNumber;
    SwitchRumbleData_t rumbleData[2];
} SwitchCommonOutputPacket_t;

typedef struct
{
    SwitchCommonOutputPacket_t commonData;

    Uint8 ucSubcommandID;
    Uint8 rgucSubcommandData[k_unSwitchOutputPacketDataLength - sizeof(SwitchCommonOutputPacket_t) - 1];
} SwitchSubcommandOutputPacket_t;

typedef struct
{
    Uint8 ucPacketType;
    Uint8 ucProprietaryID;

    Uint8 rgucProprietaryData[k_unSwitchOutputPacketDataLength - 1 - 1];
} SwitchProprietaryOutputPacket_t;
#pragma pack()

typedef struct {
    SDL_HIDAPI_Device *device;
    SDL_bool m_bInputOnly;
    SDL_bool m_bHasHomeLED;
    SDL_bool m_bUsingBluetooth;
    SDL_bool m_bIsGameCube;
    SDL_bool m_bUseButtonLabels;
    ESwitchDeviceInfoControllerType m_eControllerType;
    Uint8 m_rgucMACAddress[6];
    Uint8 m_nCommandNumber;
    SwitchCommonOutputPacket_t m_RumblePacket;
    Uint8 m_rgucReadBuffer[k_unSwitchMaxOutputPacketLength];
    SDL_bool m_bRumbleActive;
    Uint32 m_unRumbleSent;
    SDL_bool m_bRumblePending;
    SDL_bool m_bRumbleZeroPending;
    Uint32 m_unRumblePending;
    SDL_bool m_bHasSensors;
    SDL_bool m_bReportSensors;

    SwitchInputOnlyControllerStatePacket_t m_lastInputOnlyState;
    SwitchSimpleStatePacket_t m_lastSimpleState;
    SwitchStatePacket_t m_lastFullState;

    struct StickCalibrationData {
        struct {
            Sint16 sCenter;
            Sint16 sMin;
            Sint16 sMax;
        } axis[2];
    } m_StickCalData[2];

    struct StickExtents {
        struct {
            Sint16 sMin;
            Sint16 sMax;
        } axis[2];
    } m_StickExtents[2];
} SDL_DriverSwitch_Context;


static SDL_bool
HasHomeLED(int vendor_id, int product_id)
{
    /* The Power A Nintendo Switch Pro controllers don't have a Home LED */
    if (vendor_id == 0 && product_id == 0) {
        return SDL_FALSE;
    }

    /* HORI Wireless Switch Pad */
    if (vendor_id == 0x0f0d && product_id == 0x00f6) {
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

static SDL_bool
IsGameCubeFormFactor(int vendor_id, int product_id)
{
    static Uint32 gamecube_formfactor[] = {
        MAKE_VIDPID(0x0e6f, 0x0185),    /* PDP Wired Fight Pad Pro for Nintendo Switch */
        MAKE_VIDPID(0x20d6, 0xa711),    /* Core (Plus) Wired Controller */
    };
    Uint32 id = MAKE_VIDPID(vendor_id, product_id);
    int i;

    for (i = 0; i < SDL_arraysize(gamecube_formfactor); ++i) {
        if (id == gamecube_formfactor[i]) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static SDL_bool
HIDAPI_DriverSwitch_IsSupportedDevice(const char *name, SDL_GameControllerType type, Uint16 vendor_id, Uint16 product_id, Uint16 version, int interface_number, int interface_class, int interface_subclass, int interface_protocol)
{
    /* The HORI Wireless Switch Pad enumerates as a HID device when connected via USB
       with the same VID/PID as when connected over Bluetooth but doesn't actually
       support communication over USB. The most reliable way to block this without allowing the
       controller to continually attempt to reconnect is to filter it out by manufactuer/product string.
       Note that the controller does have a different product string when connected over Bluetooth.
     */
    if (SDL_strcmp(name, "HORI Wireless Switch Pad") == 0) {
        return SDL_FALSE;
    }
    return (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO) ? SDL_TRUE : SDL_FALSE;
}

static const char *
HIDAPI_DriverSwitch_GetDeviceName(Uint16 vendor_id, Uint16 product_id)
{
    /* Give a user friendly name for this controller */
    if (vendor_id == USB_VENDOR_NINTENDO) {
        if (product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_GRIP) {
            return "Nintendo Switch Joy-Con Grip";
        }

        if (product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_LEFT) {
            return "Nintendo Switch Joy-Con Left";
        }

        if (product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_RIGHT) {
            return "Nintendo Switch Joy-Con Right";
        }
    }

    return "Nintendo Switch Pro Controller";
}

static int ReadInput(SDL_DriverSwitch_Context *ctx)
{
    /* Make sure we don't try to read at the same time a write is happening */
    if (SDL_AtomicGet(&ctx->device->rumble_pending) > 0) {
        return 0;
    }

    return hid_read_timeout(ctx->device->dev, ctx->m_rgucReadBuffer, sizeof(ctx->m_rgucReadBuffer), 0);
}

static int WriteOutput(SDL_DriverSwitch_Context *ctx, const Uint8 *data, int size)
{
#ifdef SWITCH_SYNCHRONOUS_WRITES
    return hid_write(ctx->device->dev, data, size);
#else
    /* Use the rumble thread for general asynchronous writes */
    if (SDL_HIDAPI_LockRumble() < 0) {
        return -1;
    }
    return SDL_HIDAPI_SendRumbleAndUnlock(ctx->device, data, size);
#endif /* SWITCH_SYNCHRONOUS_WRITES */
}

static SwitchSubcommandInputPacket_t *ReadSubcommandReply(SDL_DriverSwitch_Context *ctx, ESwitchSubcommandIDs expectedID)
{
    /* Average response time for messages is ~30ms */
    Uint32 TimeoutMs = 100;
    Uint32 startTicks = SDL_GetTicks();

    int nRead = 0;
    while ((nRead = ReadInput(ctx)) != -1) {
        if (nRead > 0) {
            if (ctx->m_rgucReadBuffer[0] == k_eSwitchInputReportIDs_SubcommandReply) {
                SwitchSubcommandInputPacket_t *reply = (SwitchSubcommandInputPacket_t *)&ctx->m_rgucReadBuffer[1];
                if (reply->ucSubcommandID == expectedID && (reply->ucSubcommandAck & 0x80)) {
                    return reply;
                }
            }
        } else {
            SDL_Delay(1);
        }

        if (SDL_TICKS_PASSED(SDL_GetTicks(), startTicks + TimeoutMs)) {
            break;
        }
    }
    return NULL;
}

static SDL_bool ReadProprietaryReply(SDL_DriverSwitch_Context *ctx, ESwitchProprietaryCommandIDs expectedID)
{
    /* Average response time for messages is ~30ms */
    Uint32 TimeoutMs = 100;
    Uint32 startTicks = SDL_GetTicks();

    int nRead = 0;
    while ((nRead = ReadInput(ctx)) != -1) {
        if (nRead > 0) {
            if (ctx->m_rgucReadBuffer[0] == k_eSwitchInputReportIDs_CommandAck && ctx->m_rgucReadBuffer[1] == expectedID) {
                return SDL_TRUE;
            }
        } else {
            SDL_Delay(1);
        }

        if (SDL_TICKS_PASSED(SDL_GetTicks(), startTicks + TimeoutMs)) {
            break;
        }
    }
    return SDL_FALSE;
}

static void ConstructSubcommand(SDL_DriverSwitch_Context *ctx, ESwitchSubcommandIDs ucCommandID, Uint8 *pBuf, Uint8 ucLen, SwitchSubcommandOutputPacket_t *outPacket)
{
    SDL_memset(outPacket, 0, sizeof(*outPacket));

    outPacket->commonData.ucPacketType = k_eSwitchOutputReportIDs_RumbleAndSubcommand;
    outPacket->commonData.ucPacketNumber = ctx->m_nCommandNumber;

    SDL_memcpy(&outPacket->commonData.rumbleData, &ctx->m_RumblePacket.rumbleData, sizeof(ctx->m_RumblePacket.rumbleData));

    outPacket->ucSubcommandID = ucCommandID;
    SDL_memcpy(outPacket->rgucSubcommandData, pBuf, ucLen);

    ctx->m_nCommandNumber = (ctx->m_nCommandNumber + 1) & 0xF;
}

static SDL_bool WritePacket(SDL_DriverSwitch_Context *ctx, void *pBuf, Uint8 ucLen)
{
    Uint8 rgucBuf[k_unSwitchMaxOutputPacketLength];
    const size_t unWriteSize = ctx->m_bUsingBluetooth ? k_unSwitchBluetoothPacketLength : k_unSwitchUSBPacketLength;

    if (ucLen > k_unSwitchOutputPacketDataLength) {
        return SDL_FALSE;
    }

    if (ucLen < unWriteSize) {
        SDL_memcpy(rgucBuf, pBuf, ucLen);
        SDL_memset(rgucBuf+ucLen, 0, unWriteSize-ucLen);
        pBuf = rgucBuf;
        ucLen = (Uint8)unWriteSize;
    }
    return (WriteOutput(ctx, (Uint8 *)pBuf, ucLen) >= 0);
}

static SDL_bool WriteSubcommand(SDL_DriverSwitch_Context *ctx, ESwitchSubcommandIDs ucCommandID, Uint8 *pBuf, Uint8 ucLen, SwitchSubcommandInputPacket_t **ppReply)
{
    int nRetries = 5;
    SwitchSubcommandInputPacket_t *reply = NULL;

    while (!reply && nRetries--) {
        SwitchSubcommandOutputPacket_t commandPacket;
        ConstructSubcommand(ctx, ucCommandID, pBuf, ucLen, &commandPacket);

        if (!WritePacket(ctx, &commandPacket, sizeof(commandPacket))) {
            continue;
        }

        reply = ReadSubcommandReply(ctx, ucCommandID);
    }

    if (ppReply) {
        *ppReply = reply;
    }
    return reply != NULL;
}

static SDL_bool WriteProprietary(SDL_DriverSwitch_Context *ctx, ESwitchProprietaryCommandIDs ucCommand, Uint8 *pBuf, Uint8 ucLen, SDL_bool waitForReply)
{
    int nRetries = 5;

    while (nRetries--) {
        SwitchProprietaryOutputPacket_t packet;

        if ((!pBuf && ucLen > 0) || ucLen > sizeof(packet.rgucProprietaryData)) {
            return SDL_FALSE;
        }

        SDL_zero(packet);
        packet.ucPacketType = k_eSwitchOutputReportIDs_Proprietary;
        packet.ucProprietaryID = ucCommand;
        if (pBuf) {
            SDL_memcpy(packet.rgucProprietaryData, pBuf, ucLen);
        }

        if (!WritePacket(ctx, &packet, sizeof(packet))) {
            continue;
        }

        if (!waitForReply || ReadProprietaryReply(ctx, ucCommand)) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static Uint8 EncodeRumbleHighAmplitude(Uint16 amplitude) {
    /* More information about these values can be found here:
     * https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
     */
    Uint16 hfa[101][2] = { {0, 0x0},{514, 0x2},{775, 0x4},{921, 0x6},{1096, 0x8},{1303, 0x0a},{1550, 0x0c},
        {1843, 0x0e},{2192, 0x10},{2606, 0x12},{3100, 0x14},{3686, 0x16},{4383, 0x18},{5213, 0x1a},
        {6199, 0x1c},{7372, 0x1e},{7698, 0x20},{8039, 0x22},{8395, 0x24},{8767, 0x26},{9155, 0x28},
        {9560, 0x2a},{9984, 0x2c},{10426, 0x2e},{10887, 0x30},{11369, 0x32},{11873, 0x34},{12398, 0x36},
        {12947, 0x38},{13520, 0x3a},{14119, 0x3c},{14744, 0x3e},{15067, 0x40},{15397, 0x42},{15734, 0x44},
        {16079, 0x46},{16431, 0x48},{16790, 0x4a},{17158, 0x4c},{17534, 0x4e},{17918, 0x50},{18310, 0x52},
        {18711, 0x54},{19121, 0x56},{19540, 0x58},{19967, 0x5a},{20405, 0x5c},{20851, 0x5e},{21308, 0x60},
        {21775, 0x62},{22251, 0x64},{22739, 0x66},{23236, 0x68},{23745, 0x6a},{24265, 0x6c},{24797, 0x6e},
        {25340, 0x70},{25894, 0x72},{26462, 0x74},{27041, 0x76},{27633, 0x78},{28238, 0x7a},{28856, 0x7c},
        {29488, 0x7e},{30134, 0x80},{30794, 0x82},{31468, 0x84},{32157, 0x86},{32861, 0x88},{33581, 0x8a},
        {34316, 0x8c},{35068, 0x8e},{35836, 0x90},{36620, 0x92},{37422, 0x94},{38242, 0x96},{39079, 0x98},
        {39935, 0x9a},{40809, 0x9c},{41703, 0x9e},{42616, 0xa0},{43549, 0xa2},{44503, 0xa4},{45477, 0xa6},
        {46473, 0xa8},{47491, 0xaa},{48531, 0xac},{49593, 0xae},{50679, 0xb0},{51789, 0xb2},{52923, 0xb4},
        {54082, 0xb6},{55266, 0xb8},{56476, 0xba},{57713, 0xbc},{58977, 0xbe},{60268, 0xc0},{61588, 0xc2},
        {62936, 0xc4},{64315, 0xc6},{65535, 0xc8} };
    int index = 0;
    for ( ; index < 101; index++) {
        if (amplitude <= hfa[index][0]) {
            return (Uint8)hfa[index][1];
        }
    }
    return (Uint8)hfa[100][1];
}

static Uint16 EncodeRumbleLowAmplitude(Uint16 amplitude) {
    /* More information about these values can be found here:
     * https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
     */
    Uint16 lfa[101][2] = { {0, 0x0040},{514, 0x8040},{775, 0x0041},{921, 0x8041},{1096, 0x0042},
        {1303, 0x8042},{1550, 0x0043},{1843, 0x8043},{2192, 0x0044},{2606, 0x8044},{3100, 0x0045},
        {3686, 0x8045},{4383, 0x0046},{5213, 0x8046},{6199, 0x0047},{7372, 0x8047},{7698, 0x0048},
        {8039, 0x8048},{8395, 0x0049},{8767, 0x8049},{9155, 0x004a},{9560, 0x804a},{9984, 0x004b},
        {10426, 0x804b},{10887, 0x004c},{11369, 0x804c},{11873, 0x004d},{12398, 0x804d},{12947, 0x004e},
        {13520, 0x804e},{14119, 0x004f},{14744, 0x804f},{15067, 0x0050},{15397, 0x8050},{15734, 0x0051},
        {16079, 0x8051},{16431, 0x0052},{16790, 0x8052},{17158, 0x0053},{17534, 0x8053},{17918, 0x0054},
        {18310, 0x8054},{18711, 0x0055},{19121, 0x8055},{19540, 0x0056},{19967, 0x8056},{20405, 0x0057},
        {20851, 0x8057},{21308, 0x0058},{21775, 0x8058},{22251, 0x0059},{22739, 0x8059},{23236, 0x005a},
        {23745, 0x805a},{24265, 0x005b},{24797, 0x805b},{25340, 0x005c},{25894, 0x805c},{26462, 0x005d},
        {27041, 0x805d},{27633, 0x005e},{28238, 0x805e},{28856, 0x005f},{29488, 0x805f},{30134, 0x0060},
        {30794, 0x8060},{31468, 0x0061},{32157, 0x8061},{32861, 0x0062},{33581, 0x8062},{34316, 0x0063},
        {35068, 0x8063},{35836, 0x0064},{36620, 0x8064},{37422, 0x0065},{38242, 0x8065},{39079, 0x0066},
        {39935, 0x8066},{40809, 0x0067},{41703, 0x8067},{42616, 0x0068},{43549, 0x8068},{44503, 0x0069},
        {45477, 0x8069},{46473, 0x006a},{47491, 0x806a},{48531, 0x006b},{49593, 0x806b},{50679, 0x006c},
        {51789, 0x806c},{52923, 0x006d},{54082, 0x806d},{55266, 0x006e},{56476, 0x806e},{57713, 0x006f},
        {58977, 0x806f},{60268, 0x0070},{61588, 0x8070},{62936, 0x0071},{64315, 0x8071},{65535, 0x0072} };
    int index = 0;
    for (; index < 101; index++) {
        if (amplitude <= lfa[index][0]) {
            return lfa[index][1];
        }
    }
    return lfa[100][1];
}

static void SetNeutralRumble(SwitchRumbleData_t *pRumble)
{
    pRumble->rgucData[0] = 0x00;
    pRumble->rgucData[1] = 0x01;
    pRumble->rgucData[2] = 0x40;
    pRumble->rgucData[3] = 0x40;
}

static void EncodeRumble(SwitchRumbleData_t *pRumble, Uint16 usHighFreq, Uint8 ucHighFreqAmp, Uint8 ucLowFreq, Uint16 usLowFreqAmp)
{
    if (ucHighFreqAmp > 0 || usLowFreqAmp > 0) {
        // High-band frequency and low-band amplitude are actually nine-bits each so they
        // take a bit from the high-band amplitude and low-band frequency bytes respectively
        pRumble->rgucData[0] = usHighFreq & 0xFF;
        pRumble->rgucData[1] = ucHighFreqAmp | ((usHighFreq >> 8) & 0x01);

        pRumble->rgucData[2]  = ucLowFreq | ((usLowFreqAmp >> 8) & 0x80);
        pRumble->rgucData[3]  = usLowFreqAmp & 0xFF;

#ifdef DEBUG_RUMBLE
        SDL_Log("Freq: %.2X %.2X  %.2X, Amp: %.2X  %.2X %.2X\n",
            usHighFreq & 0xFF, ((usHighFreq >> 8) & 0x01), ucLowFreq,
            ucHighFreqAmp, ((usLowFreqAmp >> 8) & 0x80), usLowFreqAmp & 0xFF);
#endif
    } else {
        SetNeutralRumble(pRumble);
    }
}

static SDL_bool WriteRumble(SDL_DriverSwitch_Context *ctx)
{
    /* Write into m_RumblePacket rather than a temporary buffer to allow the current rumble state
     * to be retained for subsequent rumble or subcommand packets sent to the controller
     */
    ctx->m_RumblePacket.ucPacketType = k_eSwitchOutputReportIDs_Rumble;
    ctx->m_RumblePacket.ucPacketNumber = ctx->m_nCommandNumber;
    ctx->m_nCommandNumber = (ctx->m_nCommandNumber + 1) & 0xF;

    /* Refresh the rumble state periodically */
    ctx->m_unRumbleSent = SDL_GetTicks();

    return WritePacket(ctx, (Uint8 *)&ctx->m_RumblePacket, sizeof(ctx->m_RumblePacket));
}

static SDL_bool BReadDeviceInfo(SDL_DriverSwitch_Context *ctx)
{
    SwitchSubcommandInputPacket_t *reply = NULL;

    ctx->m_bUsingBluetooth = SDL_FALSE;

    if (WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_Status, NULL, 0, SDL_TRUE)) {
        SwitchProprietaryStatusPacket_t *status = (SwitchProprietaryStatusPacket_t *)&ctx->m_rgucReadBuffer[0];
        size_t i;

        ctx->m_eControllerType = (ESwitchDeviceInfoControllerType)status->ucDeviceType;
        for (i = 0; i < sizeof (ctx->m_rgucMACAddress); ++i)
            ctx->m_rgucMACAddress[i] = status->rgucMACAddress[ sizeof(ctx->m_rgucMACAddress) - i - 1 ];

        return SDL_TRUE;
    }

    ctx->m_bUsingBluetooth = SDL_TRUE;

    if (WriteSubcommand(ctx, k_eSwitchSubcommandIDs_RequestDeviceInfo, NULL, 0, &reply)) {
        // Byte 2: Controller ID (1=LJC, 2=RJC, 3=Pro)
        ctx->m_eControllerType = (ESwitchDeviceInfoControllerType)reply->deviceInfo.ucDeviceType;

        // Bytes 4-9: MAC address (big-endian)
        memcpy(ctx->m_rgucMACAddress, reply->deviceInfo.rgucMACAddress, sizeof(ctx->m_rgucMACAddress));

        return SDL_TRUE;
    }

    ctx->m_bUsingBluetooth = SDL_FALSE;

    return SDL_FALSE;
}

static SDL_bool BTrySetupUSB(SDL_DriverSwitch_Context *ctx)
{
    /* We have to send a connection handshake to the controller when communicating over USB
     * before we're able to send it other commands. Luckily this command is not supported
     * over Bluetooth, so we can use the controller's lack of response as a way to
     * determine if the connection is over USB or Bluetooth
     */
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_Handshake, NULL, 0, SDL_TRUE)) {
        return SDL_FALSE;
    }
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_HighSpeed, NULL, 0, SDL_TRUE)) {
        /* The 8BitDo M30 and SF30 Pro don't respond to this command, but otherwise work correctly */
        /*return SDL_FALSE;*/
    }
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_Handshake, NULL, 0, SDL_TRUE)) {
        /* This fails on the right Joy-Con when plugged into the charging grip */
        /*return SDL_FALSE;*/
    }
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_ForceUSB, NULL, 0, SDL_FALSE)) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static SDL_bool SetVibrationEnabled(SDL_DriverSwitch_Context *ctx, Uint8 enabled)
{
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_EnableVibration, &enabled, sizeof(enabled), NULL);

}
static SDL_bool SetInputMode(SDL_DriverSwitch_Context *ctx, Uint8 input_mode)
{
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SetInputReportMode, &input_mode, 1, NULL);
}

static SDL_bool SetHomeLED(SDL_DriverSwitch_Context *ctx, Uint8 brightness)
{
    Uint8 ucLedIntensity = 0;
    Uint8 rgucBuffer[4];

    if (brightness > 0) {
        if (brightness < 65) {
            ucLedIntensity = (brightness + 5) / 10;
        } else {
            ucLedIntensity = (Uint8)SDL_ceilf(0xF * SDL_powf((float)brightness / 100.f, 2.13f));
        }
    }

    rgucBuffer[0] = (0x0 << 4) | 0x1;  /* 0 mini cycles (besides first), cycle duration 8ms */
    rgucBuffer[1] = ((ucLedIntensity & 0xF) << 4) | 0x0;  /* LED start intensity (0x0-0xF), 0 cycles (LED stays on at start intensity after first cycle) */
    rgucBuffer[2] = ((ucLedIntensity & 0xF) << 4) | 0x0;  /* First cycle LED intensity, 0x0 intensity for second cycle */
    rgucBuffer[3] = (0x0 << 4) | 0x0;  /* 8ms fade transition to first cycle, 8ms first cycle LED duration */

    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SetHomeLight, rgucBuffer, sizeof(rgucBuffer), NULL);
}

static SDL_bool SetSlotLED(SDL_DriverSwitch_Context *ctx, Uint8 slot)
{
    Uint8 led_data = (1 << slot);
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SetPlayerLights, &led_data, sizeof(led_data), NULL);
}

static SDL_bool SetIMUEnabled(SDL_DriverSwitch_Context* ctx, SDL_bool enabled)
{
    Uint8 imu_data = enabled ? 1 : 0;
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_EnableIMU, &imu_data, sizeof(imu_data), NULL);
}

static SDL_bool LoadStickCalibration(SDL_DriverSwitch_Context *ctx, Uint8 input_mode)
{
    Uint8 *pStickCal;
    size_t stick, axis;
    SwitchSubcommandInputPacket_t *reply = NULL;

    /* Read Calibration Info */
    SwitchSPIOpData_t readParams;
    readParams.unAddress = k_unSPIStickCalibrationStartOffset;
    readParams.ucLength = k_unSPIStickCalibrationLength;

    if (!WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SPIFlashRead, (uint8_t *)&readParams, sizeof(readParams), &reply)) {
        return SDL_FALSE;
    }

    /* Stick calibration values are 12-bits each and are packed by bit
     * For whatever reason the fields are in a different order for each stick
     * Left:  X-Max, Y-Max, X-Center, Y-Center, X-Min, Y-Min
     * Right: X-Center, Y-Center, X-Min, Y-Min, X-Max, Y-Max
     */
    pStickCal = reply->spiReadData.rgucReadData;

    /* Left stick */
    ctx->m_StickCalData[0].axis[0].sMax    = ((pStickCal[1] << 8) & 0xF00) | pStickCal[0];     /* X Axis max above center */
    ctx->m_StickCalData[0].axis[1].sMax    = (pStickCal[2] << 4) | (pStickCal[1] >> 4);         /* Y Axis max above center */
    ctx->m_StickCalData[0].axis[0].sCenter = ((pStickCal[4] << 8) & 0xF00) | pStickCal[3];     /* X Axis center */
    ctx->m_StickCalData[0].axis[1].sCenter = (pStickCal[5] << 4) | (pStickCal[4] >> 4);        /* Y Axis center */
    ctx->m_StickCalData[0].axis[0].sMin    = ((pStickCal[7] << 8) & 0xF00) | pStickCal[6];      /* X Axis min below center */
    ctx->m_StickCalData[0].axis[1].sMin    = (pStickCal[8] << 4) | (pStickCal[7] >> 4);        /* Y Axis min below center */

    /* Right stick */
    ctx->m_StickCalData[1].axis[0].sCenter = ((pStickCal[10] << 8) & 0xF00) | pStickCal[9];     /* X Axis center */
    ctx->m_StickCalData[1].axis[1].sCenter = (pStickCal[11] << 4) | (pStickCal[10] >> 4);      /* Y Axis center */
    ctx->m_StickCalData[1].axis[0].sMin    = ((pStickCal[13] << 8) & 0xF00) | pStickCal[12];    /* X Axis min below center */
    ctx->m_StickCalData[1].axis[1].sMin    = (pStickCal[14] << 4) | (pStickCal[13] >> 4);      /* Y Axis min below center */
    ctx->m_StickCalData[1].axis[0].sMax    = ((pStickCal[16] << 8) & 0xF00) | pStickCal[15];    /* X Axis max above center */
    ctx->m_StickCalData[1].axis[1].sMax    = (pStickCal[17] << 4) | (pStickCal[16] >> 4);      /* Y Axis max above center */

    /* Filter out any values that were uninitialized (0xFFF) in the SPI read */
    for (stick = 0; stick < 2; ++stick) {
        for (axis = 0; axis < 2; ++axis) {
            if (ctx->m_StickCalData[stick].axis[axis].sCenter == 0xFFF) {
                ctx->m_StickCalData[stick].axis[axis].sCenter = 0;
            }
            if (ctx->m_StickCalData[stick].axis[axis].sMax == 0xFFF) {
                ctx->m_StickCalData[stick].axis[axis].sMax = 0;
            }
            if (ctx->m_StickCalData[stick].axis[axis].sMin == 0xFFF) {
                ctx->m_StickCalData[stick].axis[axis].sMin = 0;
            }
        }
    }

    if (input_mode == k_eSwitchInputReportIDs_SimpleControllerState) {
        for (stick = 0; stick < 2; ++stick) {
            for(axis = 0; axis < 2; ++axis) {
                ctx->m_StickExtents[stick].axis[axis].sMin = (Sint16)(SDL_MIN_SINT16 * 0.5f);
                ctx->m_StickExtents[stick].axis[axis].sMax = (Sint16)(SDL_MAX_SINT16 * 0.5f);
            }
        }
    } else {
        for (stick = 0; stick < 2; ++stick) {
            for(axis = 0; axis < 2; ++axis) {
                ctx->m_StickExtents[stick].axis[axis].sMin = -(Sint16)(ctx->m_StickCalData[stick].axis[axis].sMin * 0.7f);
                ctx->m_StickExtents[stick].axis[axis].sMax = (Sint16)(ctx->m_StickCalData[stick].axis[axis].sMax * 0.7f);
            }
        }
    }
    return SDL_TRUE;
}

static Sint16 ApplyStickCalibrationCentered(SDL_DriverSwitch_Context *ctx, int nStick, int nAxis, Sint16 sRawValue, Sint16 sCenter)
{
    sRawValue -= sCenter;

    if (sRawValue > ctx->m_StickExtents[nStick].axis[nAxis].sMax) {
        ctx->m_StickExtents[nStick].axis[nAxis].sMax = sRawValue;
    }
    if (sRawValue < ctx->m_StickExtents[nStick].axis[nAxis].sMin) {
        ctx->m_StickExtents[nStick].axis[nAxis].sMin = sRawValue;
    }

    return (Sint16)HIDAPI_RemapVal(sRawValue, ctx->m_StickExtents[nStick].axis[nAxis].sMin, ctx->m_StickExtents[nStick].axis[nAxis].sMax, SDL_MIN_SINT16, SDL_MAX_SINT16);
}

static Sint16 ApplyStickCalibration(SDL_DriverSwitch_Context *ctx, int nStick, int nAxis, Sint16 sRawValue)
{
    return ApplyStickCalibrationCentered(ctx, nStick, nAxis, sRawValue, ctx->m_StickCalData[nStick].axis[nAxis].sCenter);
}

static void SDLCALL SDL_GameControllerButtonReportingHintChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)userdata;
    ctx->m_bUseButtonLabels = SDL_GetStringBoolean(hint, SDL_TRUE);
}

static Uint8 RemapButton(SDL_DriverSwitch_Context *ctx, Uint8 button)
{
    if (!ctx->m_bUseButtonLabels) {
        /* Use button positions */
        if (ctx->m_bIsGameCube) {
            switch (button) {
            case SDL_CONTROLLER_BUTTON_B:
                return SDL_CONTROLLER_BUTTON_X;
            case SDL_CONTROLLER_BUTTON_X:
                return SDL_CONTROLLER_BUTTON_B;
            default:
                break;
            }
        } else {
            switch (button) {
            case SDL_CONTROLLER_BUTTON_A:
                return SDL_CONTROLLER_BUTTON_B;
            case SDL_CONTROLLER_BUTTON_B:
                return SDL_CONTROLLER_BUTTON_A;
            case SDL_CONTROLLER_BUTTON_X:
                return SDL_CONTROLLER_BUTTON_Y;
            case SDL_CONTROLLER_BUTTON_Y:
                return SDL_CONTROLLER_BUTTON_X;
            default:
                break;
            }
        }
    }
    return button;
}
 
static SDL_bool
HIDAPI_DriverSwitch_InitDevice(SDL_HIDAPI_Device *device)
{
    return HIDAPI_JoystickConnected(device, NULL);
}

static int
HIDAPI_DriverSwitch_GetDevicePlayerIndex(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id)
{
    return -1;
}

static void
HIDAPI_DriverSwitch_SetDevicePlayerIndex(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id, int player_index)
{
}

static SDL_bool
HIDAPI_DriverSwitch_OpenJoystick(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    SDL_DriverSwitch_Context *ctx;
    Uint8 input_mode;

    ctx = (SDL_DriverSwitch_Context *)SDL_calloc(1, sizeof(*ctx));
    if (!ctx) {
        SDL_OutOfMemory();
        goto error;
    }
    ctx->device = device;
    device->context = ctx;

    device->dev = hid_open_path(device->path, 0);
    if (!device->dev) {
        SDL_SetError("Couldn't open %s", device->path);
        goto error;
    }

    /* Find out whether or not we can send output reports */
    ctx->m_bInputOnly = SDL_IsJoystickNintendoSwitchProInputOnly(device->vendor_id, device->product_id);
    if (!ctx->m_bInputOnly) {
        ctx->m_bHasHomeLED = HasHomeLED(device->vendor_id, device->product_id);

        /* Initialize rumble data */
        SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[0]);
        SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[1]);

        if (!BReadDeviceInfo(ctx)) {
            SDL_SetError("Couldn't read device info");
            goto error;
        }

        if (!ctx->m_bUsingBluetooth) {
            if (!BTrySetupUSB(ctx)) {
                SDL_SetError("Couldn't setup USB mode");
                goto error;
            }
        }

        /* Determine the desired input mode (needed before loading stick calibration) */
        if (ctx->m_bUsingBluetooth) {
            input_mode = k_eSwitchInputReportIDs_SimpleControllerState;
        } else {
            input_mode = k_eSwitchInputReportIDs_FullControllerState;
        }

        /* The official Nintendo Switch Pro Controller supports FullControllerState over bluetooth
         * just fine. We really should use that, or else the epowerlevel code in
         * HandleFullControllerState is completely pointless. We need full state if we want battery
         * level and we only care about battery level over bluetooth anyway.
         */
        if (device->vendor_id == USB_VENDOR_NINTENDO &&
                (device->product_id == USB_PRODUCT_NINTENDO_SWITCH_PRO ||
                device->product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_GRIP ||
                device->product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_LEFT ||
                device->product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_RIGHT)) {
            input_mode = k_eSwitchInputReportIDs_FullControllerState;
        }
        
        if (input_mode == k_eSwitchInputReportIDs_FullControllerState) {
            SDL_PrivateJoystickAddSensor(joystick, SDL_SENSOR_GYRO, 200.0f);
            SDL_PrivateJoystickAddSensor(joystick, SDL_SENSOR_ACCEL, 200.0f);
            ctx->m_bHasSensors = SDL_TRUE;
        }

        if (!LoadStickCalibration(ctx, input_mode)) {
            SDL_SetError("Couldn't load stick calibration");
            goto error;
        }

        if (!SetVibrationEnabled(ctx, 1)) {
            SDL_SetError("Couldn't enable vibration");
            goto error;
        }

        /* Set desired input mode */
        if (!SetInputMode(ctx, input_mode)) {
            SDL_SetError("Couldn't set input mode");
            goto error;
        }

        /* Start sending USB reports */
        if (!ctx->m_bUsingBluetooth) {
            /* ForceUSB doesn't generate an ACK, so don't wait for a reply */
            if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_ForceUSB, NULL, 0, SDL_FALSE)) {
                SDL_SetError("Couldn't start USB reports");
                goto error;
            }
        }

        /* Set the LED state */
        if (ctx->m_bHasHomeLED) {
            if (SDL_GetHintBoolean(SDL_HINT_JOYSTICK_HIDAPI_SWITCH_HOME_LED, SDL_TRUE)) {
                SetHomeLED(ctx, 100);
            } else {
                SetHomeLED(ctx, 0);
            }
        }
        SetSlotLED(ctx, (joystick->instance_id % 4));

        /* Set the serial number */
        {
            char serial[18];

            SDL_snprintf(serial, sizeof(serial), "%.2x-%.2x-%.2x-%.2x-%.2x-%.2x",
                ctx->m_rgucMACAddress[0],
                ctx->m_rgucMACAddress[1],
                ctx->m_rgucMACAddress[2],
                ctx->m_rgucMACAddress[3],
                ctx->m_rgucMACAddress[4],
                ctx->m_rgucMACAddress[5]);
            joystick->serial = SDL_strdup(serial);
        }
    }

    if (IsGameCubeFormFactor(device->vendor_id, device->product_id)) {
        /* This is a controller shaped like a GameCube controller, with a large central A button */
        ctx->m_bIsGameCube = SDL_TRUE;
    }

    SDL_AddHintCallback(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS,
                        SDL_GameControllerButtonReportingHintChanged, ctx);

    /* Initialize the joystick capabilities */
    if (ctx->m_eControllerType == k_eSwitchDeviceInfoControllerType_JoyConLeft ||
        ctx->m_eControllerType == k_eSwitchDeviceInfoControllerType_JoyConRight) {
        joystick->nbuttons = 20;
    } else {
        joystick->nbuttons = 16;
    }
    joystick->naxes = SDL_CONTROLLER_AXIS_MAX;
    joystick->epowerlevel = SDL_JOYSTICK_POWER_WIRED;

    return SDL_TRUE;

error:
    SDL_LockMutex(device->dev_lock);
    {
        if (device->dev) {
            hid_close(device->dev);
            device->dev = NULL;
        }
        if (device->context) {
            SDL_free(device->context);
            device->context = NULL;
        }
    }
    SDL_UnlockMutex(device->dev_lock);
    return SDL_FALSE;
}

static int
HIDAPI_DriverSwitch_ActuallyRumbleJoystick(SDL_DriverSwitch_Context *ctx, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    /* Experimentally determined rumble values. These will only matter on some controllers as tested ones
     * seem to disregard these and just use any non-zero rumble values as a binary flag for constant rumble
     *
     * More information about these values can be found here:
     * https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
     */
    const Uint16 k_usHighFreq = 0x0074;
    const Uint8  k_ucHighFreqAmp = EncodeRumbleHighAmplitude(high_frequency_rumble);
    const Uint8  k_ucLowFreq = 0x3D;
    const Uint16 k_usLowFreqAmp = EncodeRumbleLowAmplitude(low_frequency_rumble);

    if (low_frequency_rumble || high_frequency_rumble) {
        EncodeRumble(&ctx->m_RumblePacket.rumbleData[0], k_usHighFreq, k_ucHighFreqAmp, k_ucLowFreq, k_usLowFreqAmp);
        EncodeRumble(&ctx->m_RumblePacket.rumbleData[1], k_usHighFreq, k_ucHighFreqAmp, k_ucLowFreq, k_usLowFreqAmp);
    } else {
        SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[0]);
        SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[1]);
    }

    ctx->m_bRumbleActive = (low_frequency_rumble || high_frequency_rumble) ? SDL_TRUE : SDL_FALSE;

    if (!WriteRumble(ctx)) {
        SDL_SetError("Couldn't send rumble packet");
        return -1;
    }
    return 0;
}

static int
HIDAPI_DriverSwitch_SendPendingRumble(SDL_DriverSwitch_Context *ctx)
{
    if (!SDL_TICKS_PASSED(SDL_GetTicks(), ctx->m_unRumbleSent + RUMBLE_WRITE_FREQUENCY_MS)) {
        return 0;
    }

    if (ctx->m_bRumblePending) {
        Uint16 low_frequency_rumble = (Uint16)(ctx->m_unRumblePending >> 16);
        Uint16 high_frequency_rumble = (Uint16)ctx->m_unRumblePending;

#ifdef DEBUG_RUMBLE
        SDL_Log("Sent pending rumble %d/%d, %d ms after previous rumble\n", low_frequency_rumble, high_frequency_rumble, SDL_GetTicks() - ctx->m_unRumbleSent);
#endif
        ctx->m_bRumblePending = SDL_FALSE;
        ctx->m_unRumblePending = 0;

        return HIDAPI_DriverSwitch_ActuallyRumbleJoystick(ctx, low_frequency_rumble, high_frequency_rumble);
    }

    if (ctx->m_bRumbleZeroPending) {
        ctx->m_bRumbleZeroPending = SDL_FALSE;

#ifdef DEBUG_RUMBLE
        SDL_Log("Sent pending zero rumble, %d ms after previous rumble\n", SDL_GetTicks() - ctx->m_unRumbleSent);
#endif
        return HIDAPI_DriverSwitch_ActuallyRumbleJoystick(ctx, 0, 0);
    }

    return 0;
}

static int
HIDAPI_DriverSwitch_RumbleJoystick(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)device->context;

    if (ctx->m_bInputOnly) {
        return SDL_Unsupported();
    }

    if (ctx->m_bRumblePending) {
        if (HIDAPI_DriverSwitch_SendPendingRumble(ctx) < 0) {
            return -1;
        }
    }

    if (!SDL_TICKS_PASSED(SDL_GetTicks(), ctx->m_unRumbleSent + RUMBLE_WRITE_FREQUENCY_MS)) {
        if (low_frequency_rumble || high_frequency_rumble) {
            Uint32 unRumblePending = ((Uint32)low_frequency_rumble << 16) | high_frequency_rumble;

            /* Keep the highest rumble intensity in the given interval */
            if (unRumblePending > ctx->m_unRumblePending) {
                ctx->m_unRumblePending = unRumblePending;
            }
            ctx->m_bRumblePending = SDL_TRUE;
            ctx->m_bRumbleZeroPending = SDL_FALSE;
        } else {
            /* When rumble is complete, turn it off */
            ctx->m_bRumbleZeroPending = SDL_TRUE;
        }
        return 0;
    }

#ifdef DEBUG_RUMBLE
    SDL_Log("Sent rumble %d/%d\n", low_frequency_rumble, high_frequency_rumble);
#endif

    return HIDAPI_DriverSwitch_ActuallyRumbleJoystick(ctx, low_frequency_rumble, high_frequency_rumble);
}

static int
HIDAPI_DriverSwitch_RumbleJoystickTriggers(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static SDL_bool
HIDAPI_DriverSwitch_HasJoystickLED(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    /* Doesn't have an RGB LED, so don't return true here */
    return SDL_FALSE;
}

static int
HIDAPI_DriverSwitch_SetJoystickLED(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
HIDAPI_DriverSwitch_SendJoystickEffect(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int
HIDAPI_DriverSwitch_SetJoystickSensorsEnabled(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, SDL_bool enabled)
{
    SDL_DriverSwitch_Context* ctx = (SDL_DriverSwitch_Context*)device->context;
    
    if (!ctx->m_bHasSensors) {
        return SDL_Unsupported();
    }

    SetIMUEnabled(ctx, enabled);
    ctx->m_bReportSensors = enabled;

    return 0;
}

static float
HIDAPI_DriverSwitch_ScaleGyro(Sint16 value)
{
    float result = (value / SWITCH_GYRO_SCALE) * (float)M_PI / 180.0f;
    return result;
}

static float
HIDAPI_DriverSwitch_ScaleAccel(Sint16 value)
{
    float result = (value / SWITCH_ACCEL_SCALE) * SDL_STANDARD_GRAVITY;
    return result;
}

static void HandleInputOnlyControllerState(SDL_Joystick *joystick, SDL_DriverSwitch_Context *ctx, SwitchInputOnlyControllerStatePacket_t *packet)
{
    Sint16 axis;

    if (packet->rgucButtons[0] != ctx->m_lastInputOnlyState.rgucButtons[0]) {
        Uint8 data = packet->rgucButtons[0];
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_A), (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_B), (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_X), (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_Y), (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);

        axis = (data & 0x40) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT, axis);

        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, axis);
    }

    if (packet->rgucButtons[1] != ctx->m_lastInputOnlyState.rgucButtons[1]) {
        Uint8 data = packet->rgucButtons[1];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_BACK, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_START, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSTICK, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSTICK, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_MISC1, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);
    }

    if (packet->ucStickHat != ctx->m_lastInputOnlyState.ucStickHat) {
        SDL_bool dpad_up = SDL_FALSE;
        SDL_bool dpad_down = SDL_FALSE;
        SDL_bool dpad_left = SDL_FALSE;
        SDL_bool dpad_right = SDL_FALSE;

        switch (packet->ucStickHat) {
        case 0:
            dpad_up = SDL_TRUE;
            break;
        case 1:
            dpad_up = SDL_TRUE;
            dpad_right = SDL_TRUE;
            break;
        case 2:
            dpad_right = SDL_TRUE;
            break;
        case 3:
            dpad_right = SDL_TRUE;
            dpad_down = SDL_TRUE;
            break;
        case 4:
            dpad_down = SDL_TRUE;
            break;
        case 5:
            dpad_left = SDL_TRUE;
            dpad_down = SDL_TRUE;
            break;
        case 6:
            dpad_left = SDL_TRUE;
            break;
        case 7:
            dpad_up = SDL_TRUE;
            dpad_left = SDL_TRUE;
            break;
        default:
            break;
        }
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN, dpad_down);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP, dpad_up);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, dpad_right);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT, dpad_left);
    }

    if (packet->rgucJoystickLeft[0] != ctx->m_lastInputOnlyState.rgucJoystickLeft[0]) {
        axis = (Sint16)HIDAPI_RemapVal(packet->rgucJoystickLeft[0], SDL_MIN_UINT8, SDL_MAX_UINT8, SDL_MIN_SINT16, SDL_MAX_SINT16);
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, axis);
    }

    if (packet->rgucJoystickLeft[1] != ctx->m_lastInputOnlyState.rgucJoystickLeft[1]) {
        axis = (Sint16)HIDAPI_RemapVal(packet->rgucJoystickLeft[1], SDL_MIN_UINT8, SDL_MAX_UINT8, SDL_MIN_SINT16, SDL_MAX_SINT16);
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, axis);
    }

    if (packet->rgucJoystickRight[0] != ctx->m_lastInputOnlyState.rgucJoystickRight[0]) {
        axis = (Sint16)HIDAPI_RemapVal(packet->rgucJoystickRight[0], SDL_MIN_UINT8, SDL_MAX_UINT8, SDL_MIN_SINT16, SDL_MAX_SINT16);
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTX, axis);
    }

    if (packet->rgucJoystickRight[1] != ctx->m_lastInputOnlyState.rgucJoystickRight[1]) {
        axis = (Sint16)HIDAPI_RemapVal(packet->rgucJoystickRight[1], SDL_MIN_UINT8, SDL_MAX_UINT8, SDL_MIN_SINT16, SDL_MAX_SINT16);
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTY, axis);
    }

    ctx->m_lastInputOnlyState = *packet;
}

static void HandleSimpleControllerState(SDL_Joystick *joystick, SDL_DriverSwitch_Context *ctx, SwitchSimpleStatePacket_t *packet)
{
    /* 0x8000 is the neutral value for all joystick axes */
    const Uint16 usJoystickCenter = 0x8000;
    Sint16 axis;

    if (packet->rgucButtons[0] != ctx->m_lastSimpleState.rgucButtons[0]) {
        Uint8 data = packet->rgucButtons[0];
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_A), (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_B), (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_X), (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_Y), (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);

        axis = (data & 0x40) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT, axis);

        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, axis);
    }

    if (packet->rgucButtons[1] != ctx->m_lastSimpleState.rgucButtons[1]) {
        Uint8 data = packet->rgucButtons[1];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_BACK, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_START, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSTICK, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSTICK, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_MISC1, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);
    }

    if (packet->ucStickHat != ctx->m_lastSimpleState.ucStickHat) {
        SDL_bool dpad_up = SDL_FALSE;
        SDL_bool dpad_down = SDL_FALSE;
        SDL_bool dpad_left = SDL_FALSE;
        SDL_bool dpad_right = SDL_FALSE;

        switch (packet->ucStickHat) {
        case 0:
            dpad_up = SDL_TRUE;
            break;
        case 1:
            dpad_up = SDL_TRUE;
            dpad_right = SDL_TRUE;
            break;
        case 2:
            dpad_right = SDL_TRUE;
            break;
        case 3:
            dpad_right = SDL_TRUE;
            dpad_down = SDL_TRUE;
            break;
        case 4:
            dpad_down = SDL_TRUE;
            break;
        case 5:
            dpad_left = SDL_TRUE;
            dpad_down = SDL_TRUE;
            break;
        case 6:
            dpad_left = SDL_TRUE;
            break;
        case 7:
            dpad_up = SDL_TRUE;
            dpad_left = SDL_TRUE;
            break;
        default:
            break;
        }
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN, dpad_down);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP, dpad_up);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, dpad_right);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT, dpad_left);
    }

    axis = ApplyStickCalibrationCentered(ctx, 0, 0, packet->sJoystickLeft[0], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, axis);

    axis = ApplyStickCalibrationCentered(ctx, 0, 1, packet->sJoystickLeft[1], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, axis);

    axis = ApplyStickCalibrationCentered(ctx, 1, 0, packet->sJoystickRight[0], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTX, axis);

    axis = ApplyStickCalibrationCentered(ctx, 1, 1, packet->sJoystickRight[1], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTY, axis);

    ctx->m_lastSimpleState = *packet;
}

static void SendSensorUpdate(SDL_Joystick *joystick, SDL_DriverSwitch_Context *ctx, SDL_SensorType type, Sint16 *values)
{
    float data[3];

    /* Note the order of components has been shuffled to match PlayStation controllers,
     * since that's our de facto standard from already supporting those controllers, and
     * users will want consistent axis mappings across devices.
     */
    if (type == SDL_SENSOR_GYRO) {
        data[0] = -HIDAPI_DriverSwitch_ScaleGyro(values[1]);
        data[1] = HIDAPI_DriverSwitch_ScaleGyro(values[2]);
        data[2] = -HIDAPI_DriverSwitch_ScaleGyro(values[0]);
    } else {
        data[0] = -HIDAPI_DriverSwitch_ScaleAccel(values[1]);
        data[1] = HIDAPI_DriverSwitch_ScaleAccel(values[2]);
        data[2] = -HIDAPI_DriverSwitch_ScaleAccel(values[0]);
    }

    /* Right Joy-Con flips some axes, so let's flip them back for consistency */
    if (ctx->m_eControllerType == k_eSwitchDeviceInfoControllerType_JoyConRight) {
        data[0] = -data[0];
        data[1] = -data[1];
    }

    SDL_PrivateJoystickSensor(joystick, type, data, 3);
}

static void HandleFullControllerState(SDL_Joystick *joystick, SDL_DriverSwitch_Context *ctx, SwitchStatePacket_t *packet)
{
    Sint16 axis;

    if (packet->controllerState.rgucButtons[0] != ctx->m_lastFullState.controllerState.rgucButtons[0]) {
        Uint8 data = packet->controllerState.rgucButtons[0];
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_A), (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_B), (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_X), (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, RemapButton(ctx, SDL_CONTROLLER_BUTTON_Y), (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        if (ctx->m_eControllerType == k_eSwitchDeviceInfoControllerType_JoyConRight) {
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_PADDLE1, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_PADDLE3, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);
        }
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, (data & 0x40) ? SDL_PRESSED : SDL_RELEASED);
        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, axis);
    }

    if (packet->controllerState.rgucButtons[1] != ctx->m_lastFullState.controllerState.rgucButtons[1]) {
        Uint8 data = packet->controllerState.rgucButtons[1];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_BACK, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_START, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSTICK, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSTICK, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);

        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_MISC1, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);
    }

    if (packet->controllerState.rgucButtons[2] != ctx->m_lastFullState.controllerState.rgucButtons[2]) {
        Uint8 data = packet->controllerState.rgucButtons[2];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        if (ctx->m_eControllerType == k_eSwitchDeviceInfoControllerType_JoyConLeft) {
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_PADDLE4, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_PADDLE2, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);
        }
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, (data & 0x40) ? SDL_PRESSED : SDL_RELEASED);
        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT, axis);
    }

    axis = packet->controllerState.rgucJoystickLeft[0] | ((packet->controllerState.rgucJoystickLeft[1] & 0xF) << 8);
    axis = ApplyStickCalibration(ctx, 0, 0, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, axis);

    axis = ((packet->controllerState.rgucJoystickLeft[1] & 0xF0) >> 4) | (packet->controllerState.rgucJoystickLeft[2] << 4);
    axis = ApplyStickCalibration(ctx, 0, 1, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, ~axis);

    axis = packet->controllerState.rgucJoystickRight[0] | ((packet->controllerState.rgucJoystickRight[1] & 0xF) << 8);
    axis = ApplyStickCalibration(ctx, 1, 0, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTX, axis);

    axis = ((packet->controllerState.rgucJoystickRight[1] & 0xF0) >> 4) | (packet->controllerState.rgucJoystickRight[2] << 4);
    axis = ApplyStickCalibration(ctx, 1, 1, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTY, ~axis);

    /* High nibble of battery/connection byte is battery level, low nibble is connection status
     * LSB of connection nibble is USB/Switch connection status
     */
    if (packet->controllerState.ucBatteryAndConnection & 0x1) {
        joystick->epowerlevel = SDL_JOYSTICK_POWER_WIRED;
    } else {
        /* LSB of the battery nibble is used to report charging.
         * The battery level is reported from 0(empty)-8(full)
         */
        int level = (packet->controllerState.ucBatteryAndConnection & 0xE0) >> 4;
        if (level == 0) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_EMPTY;
        } else if (level <= 2) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_LOW;
        } else if (level <= 6) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_MEDIUM;
        } else {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_FULL;
        }
    }

    if (ctx->m_bReportSensors) {
        SendSensorUpdate(joystick, ctx, SDL_SENSOR_GYRO, &packet->imuState[2].sGyroX);
        SendSensorUpdate(joystick, ctx, SDL_SENSOR_GYRO, &packet->imuState[1].sGyroX);
        SendSensorUpdate(joystick, ctx, SDL_SENSOR_GYRO, &packet->imuState[0].sGyroX);

        SendSensorUpdate(joystick, ctx, SDL_SENSOR_ACCEL, &packet->imuState[2].sAccelX);
        SendSensorUpdate(joystick, ctx, SDL_SENSOR_ACCEL, &packet->imuState[1].sAccelX);
        SendSensorUpdate(joystick, ctx, SDL_SENSOR_ACCEL, &packet->imuState[0].sAccelX);
    }

    ctx->m_lastFullState = *packet;
}

static SDL_bool
HIDAPI_DriverSwitch_UpdateDevice(SDL_HIDAPI_Device *device)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)device->context;
    SDL_Joystick *joystick = NULL;
    int size;

    if (device->num_joysticks > 0) {
        joystick = SDL_JoystickFromInstanceID(device->joysticks[0]);
    }
    if (!joystick) {
        return SDL_FALSE;
    }

    while ((size = ReadInput(ctx)) > 0) {
#ifdef DEBUG_SWITCH_PROTOCOL
        HIDAPI_DumpPacket("Nintendo Switch packet: size = %d", ctx->m_rgucReadBuffer, size);
#endif
        if (ctx->m_bInputOnly) {
            HandleInputOnlyControllerState(joystick, ctx, (SwitchInputOnlyControllerStatePacket_t *)&ctx->m_rgucReadBuffer[0]);
        } else {
            switch (ctx->m_rgucReadBuffer[0]) {
            case k_eSwitchInputReportIDs_SimpleControllerState:
                HandleSimpleControllerState(joystick, ctx, (SwitchSimpleStatePacket_t *)&ctx->m_rgucReadBuffer[1]);
                break;
            case k_eSwitchInputReportIDs_FullControllerState:
                HandleFullControllerState(joystick, ctx, (SwitchStatePacket_t *)&ctx->m_rgucReadBuffer[1]);
                break;
            default:
                break;
            }
        }
    }

    if (ctx->m_bRumblePending || ctx->m_bRumbleZeroPending) {
        HIDAPI_DriverSwitch_SendPendingRumble(ctx);
    } else if (ctx->m_bRumbleActive &&
               SDL_TICKS_PASSED(SDL_GetTicks(), ctx->m_unRumbleSent + RUMBLE_REFRESH_FREQUENCY_MS)) {
#ifdef DEBUG_RUMBLE
        SDL_Log("Sent continuing rumble, %d ms after previous rumble\n", SDL_GetTicks() - ctx->m_unRumbleSent);
#endif
        WriteRumble(ctx);
    }

    if (size < 0) {
        /* Read error, device is disconnected */
        HIDAPI_JoystickDisconnected(device, joystick->instance_id);
    }
    return (size >= 0);
}

static void
HIDAPI_DriverSwitch_CloseJoystick(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)device->context;

    if (!ctx->m_bInputOnly) {
        /* Restore simple input mode for other applications */
        SetInputMode(ctx, k_eSwitchInputReportIDs_SimpleControllerState);
    }

    SDL_DelHintCallback(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS,
                        SDL_GameControllerButtonReportingHintChanged, ctx);

    SDL_LockMutex(device->dev_lock);
    {
        hid_close(device->dev);
        device->dev = NULL;

        SDL_free(device->context);
        device->context = NULL;
    }
    SDL_UnlockMutex(device->dev_lock);
}

static void
HIDAPI_DriverSwitch_FreeDevice(SDL_HIDAPI_Device *device)
{
}

SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverSwitch =
{
    SDL_HINT_JOYSTICK_HIDAPI_SWITCH,
    SDL_TRUE,
    HIDAPI_DriverSwitch_IsSupportedDevice,
    HIDAPI_DriverSwitch_GetDeviceName,
    HIDAPI_DriverSwitch_InitDevice,
    HIDAPI_DriverSwitch_GetDevicePlayerIndex,
    HIDAPI_DriverSwitch_SetDevicePlayerIndex,
    HIDAPI_DriverSwitch_UpdateDevice,
    HIDAPI_DriverSwitch_OpenJoystick,
    HIDAPI_DriverSwitch_RumbleJoystick,
    HIDAPI_DriverSwitch_RumbleJoystickTriggers,
    HIDAPI_DriverSwitch_HasJoystickLED,
    HIDAPI_DriverSwitch_SetJoystickLED,
    HIDAPI_DriverSwitch_SendJoystickEffect,
    HIDAPI_DriverSwitch_SetJoystickSensorsEnabled,
    HIDAPI_DriverSwitch_CloseJoystick,
    HIDAPI_DriverSwitch_FreeDevice,
};

#endif /* SDL_JOYSTICK_HIDAPI_SWITCH */

#endif /* SDL_JOYSTICK_HIDAPI */

/* vi: set ts=4 sw=4 expandtab: */
