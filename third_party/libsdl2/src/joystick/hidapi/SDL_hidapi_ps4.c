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
/* This driver supports both simplified reports and the extended input reports enabled by Steam.
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


#ifdef SDL_JOYSTICK_HIDAPI_PS4

/* Define this if you want to log all packets from the controller */
/*#define DEBUG_PS4_PROTOCOL*/

/* Define this if you want to log calibration data */
/*#define DEBUG_PS4_CALIBRATION*/

#define GYRO_RES_PER_DEGREE 1024.0f
#define ACCEL_RES_PER_G     8192.0f
#define BLUETOOTH_DISCONNECT_TIMEOUT_MS 500

#define LOAD16(A, B)  (Sint16)((Uint16)(A) | (((Uint16)(B)) << 8))

typedef enum
{
    k_EPS4ReportIdUsbState = 1,
    k_EPS4ReportIdUsbEffects = 5,
    k_EPS4ReportIdBluetoothState1 = 17,
    k_EPS4ReportIdBluetoothState2 = 18,
    k_EPS4ReportIdBluetoothState3 = 19,
    k_EPS4ReportIdBluetoothState4 = 20,
    k_EPS4ReportIdBluetoothState5 = 21,
    k_EPS4ReportIdBluetoothState6 = 22,
    k_EPS4ReportIdBluetoothState7 = 23,
    k_EPS4ReportIdBluetoothState8 = 24,
    k_EPS4ReportIdBluetoothState9 = 25,
    k_EPS4ReportIdBluetoothEffects = 17,
    k_EPS4ReportIdDisconnectMessage = 226,
} EPS4ReportId;

typedef enum 
{
    k_ePS4FeatureReportIdGyroCalibration_USB = 0x02,
    k_ePS4FeatureReportIdGyroCalibration_BT = 0x05,
    k_ePS4FeatureReportIdSerialNumber = 0x12,
} EPS4FeatureReportID;

typedef struct
{
    Uint8 ucLeftJoystickX;
    Uint8 ucLeftJoystickY;
    Uint8 ucRightJoystickX;
    Uint8 ucRightJoystickY;
    Uint8 rgucButtonsHatAndCounter[ 3 ];
    Uint8 ucTriggerLeft;
    Uint8 ucTriggerRight;
    Uint8 _rgucPad0[ 3 ];
    Uint8 rgucGyroX[2];
    Uint8 rgucGyroY[2];
    Uint8 rgucGyroZ[2];
    Uint8 rgucAccelX[2];
    Uint8 rgucAccelY[2];
    Uint8 rgucAccelZ[2];
    Uint8 _rgucPad1[ 5 ];
    Uint8 ucBatteryLevel;
    Uint8 _rgucPad2[ 4 ];
    Uint8 ucTouchpadCounter1;
    Uint8 rgucTouchpadData1[ 3 ];
    Uint8 ucTouchpadCounter2;
    Uint8 rgucTouchpadData2[ 3 ];
} PS4StatePacket_t;

typedef struct
{
    Uint8 ucRumbleRight;
    Uint8 ucRumbleLeft;
    Uint8 ucLedRed;
    Uint8 ucLedGreen;
    Uint8 ucLedBlue;
    Uint8 ucLedDelayOn;
    Uint8 ucLedDelayOff;
    Uint8 _rgucPad0[ 8 ];
    Uint8 ucVolumeLeft;
    Uint8 ucVolumeRight;
    Uint8 ucVolumeMic;
    Uint8 ucVolumeSpeaker;
} DS4EffectsState_t;

typedef struct {
    Sint16 bias;
    float sensitivity;
} IMUCalibrationData;

typedef struct {
    SDL_HIDAPI_Device *device;
    SDL_Joystick *joystick;
    SDL_bool is_dongle;
    SDL_bool is_bluetooth;
    SDL_bool official_controller;
    SDL_bool audio_supported;
    SDL_bool effects_supported;
    SDL_bool enhanced_mode;
    SDL_bool report_sensors;
    SDL_bool hardware_calibration;
    IMUCalibrationData calibration[6];
    Uint32 last_packet;
    int player_index;
    Uint8 rumble_left;
    Uint8 rumble_right;
    SDL_bool color_set;
    Uint8 led_red;
    Uint8 led_green;
    Uint8 led_blue;
    Uint8 volume;
    Uint32 last_volume_check;
    PS4StatePacket_t last_state;
} SDL_DriverPS4_Context;


static int HIDAPI_DriverPS4_SendJoystickEffect(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, const void *effect, int size);

static SDL_bool
HIDAPI_DriverPS4_IsSupportedDevice(const char *name, SDL_GameControllerType type, Uint16 vendor_id, Uint16 product_id, Uint16 version, int interface_number, int interface_class, int interface_subclass, int interface_protocol)
{
    return (type == SDL_CONTROLLER_TYPE_PS4) ? SDL_TRUE : SDL_FALSE;
}

static const char *
HIDAPI_DriverPS4_GetDeviceName(Uint16 vendor_id, Uint16 product_id)
{
    if (vendor_id == USB_VENDOR_SONY) {
        return "PS4 Controller";
    }
    return NULL;
}

static int ReadFeatureReport(hid_device *dev, Uint8 report_id, Uint8 *report, size_t length)
{
    SDL_memset(report, 0, length);
    report[0] = report_id;
    return hid_get_feature_report(dev, report, length);
}

static SDL_bool HIDAPI_DriverPS4_CanRumble(Uint16 vendor_id, Uint16 product_id)
{
    /* The Razer Panthera fight stick hangs when trying to rumble */
    if (vendor_id == USB_VENDOR_RAZER &&
        (product_id == USB_PRODUCT_RAZER_PANTHERA || product_id == USB_PRODUCT_RAZER_PANTHERA_EVO)) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static void
SetLedsForPlayerIndex(DS4EffectsState_t *effects, int player_index)
{
    /* This list is the same as what hid-sony.c uses in the Linux kernel.
       The first 4 values correspond to what the PS4 assigns.
    */
    static const Uint8 colors[7][3] = {
        { 0x00, 0x00, 0x40 }, /* Blue */
        { 0x40, 0x00, 0x00 }, /* Red */
        { 0x00, 0x40, 0x00 }, /* Green */
        { 0x20, 0x00, 0x20 }, /* Pink */
        { 0x02, 0x01, 0x00 }, /* Orange */
        { 0x00, 0x01, 0x01 }, /* Teal */
        { 0x01, 0x01, 0x01 }  /* White */
    };

    if (player_index >= 0) {
        player_index %= SDL_arraysize(colors);
    } else {
        player_index = 0;
    }

    effects->ucLedRed = colors[player_index][0];
    effects->ucLedGreen = colors[player_index][1];
    effects->ucLedBlue = colors[player_index][2];
}

static SDL_bool
HIDAPI_DriverPS4_InitDevice(SDL_HIDAPI_Device *device)
{
    return HIDAPI_JoystickConnected(device, NULL);
}

static int
HIDAPI_DriverPS4_GetDevicePlayerIndex(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id)
{
    return -1;
}

static void
HIDAPI_DriverPS4_LoadCalibrationData(SDL_HIDAPI_Device *device)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;
    int i, tries, size;
    SDL_bool have_data = SDL_FALSE;
    Uint8 data[USB_PACKET_LENGTH];

    if (!ctx->official_controller) {
#ifdef DEBUG_PS4_CALIBRATION
        SDL_Log("Not an official controller, ignoring calibration\n");
#endif
        return;
    }

    for( tries = 0; tries < 5; ++tries ) {
        /* For Bluetooth controllers, this report switches them into advanced report mode */
        size = ReadFeatureReport(device->dev, k_ePS4FeatureReportIdGyroCalibration_USB, data, sizeof(data));
        if (size < 35) {
#ifdef DEBUG_PS4_CALIBRATION
            SDL_Log("Short read of calibration data: %d, ignoring calibration\n", size);
#endif
            return;
        }

        if (ctx->is_bluetooth) {
            size = ReadFeatureReport(device->dev, k_ePS4FeatureReportIdGyroCalibration_BT, data, sizeof(data));
            if (size < 35) {
#ifdef DEBUG_PS4_CALIBRATION
                SDL_Log("Short read of calibration data: %d, ignoring calibration\n", size);
#endif
                return;
            }
        }

        /* In some cases this report returns all zeros. Usually immediately after connection with the PS4 Dongle */
        for (i = 0; i < size; ++i) {
            if (data[i]) {
                have_data = SDL_TRUE;
                break;
            }
        }
        if (have_data) {
            break;
        }

        SDL_Delay(2);
    }

    if (have_data) {
        Sint16 sGyroPitchBias, sGyroYawBias, sGyroRollBias;
        Sint16 sGyroPitchPlus, sGyroPitchMinus;
        Sint16 sGyroYawPlus, sGyroYawMinus;
        Sint16 sGyroRollPlus, sGyroRollMinus;
        Sint16 sGyroSpeedPlus, sGyroSpeedMinus;

        Sint16 sAccXPlus, sAccXMinus;
        Sint16 sAccYPlus, sAccYMinus;
        Sint16 sAccZPlus, sAccZMinus;

        float flNumerator;
        Sint16 sRange2g;

#ifdef DEBUG_PS4_CALIBRATION
        HIDAPI_DumpPacket("PS4 calibration packet: size = %d", data, size);
#endif

        sGyroPitchBias = LOAD16(data[1], data[2]);
        sGyroYawBias = LOAD16(data[3], data[4]);
        sGyroRollBias = LOAD16(data[5], data[6]);

        if (ctx->is_bluetooth || ctx->is_dongle) {
            sGyroPitchPlus = LOAD16(data[7], data[8]);
            sGyroYawPlus = LOAD16(data[9], data[10]);
            sGyroRollPlus = LOAD16(data[11], data[12]);
            sGyroPitchMinus = LOAD16(data[13], data[14]);
            sGyroYawMinus = LOAD16(data[15], data[16]);
            sGyroRollMinus = LOAD16(data[17], data[18]);
        } else {
            sGyroPitchPlus = LOAD16(data[7], data[8]);
            sGyroPitchMinus = LOAD16(data[9], data[10]);
            sGyroYawPlus = LOAD16(data[11], data[12]);
            sGyroYawMinus = LOAD16(data[13], data[14]);
            sGyroRollPlus = LOAD16(data[15], data[16]);
            sGyroRollMinus = LOAD16(data[17], data[18]);
        }

        sGyroSpeedPlus = LOAD16(data[19], data[20]);
        sGyroSpeedMinus = LOAD16(data[21], data[22]);

        sAccXPlus = LOAD16(data[23], data[24]);
        sAccXMinus = LOAD16(data[25], data[26]);
        sAccYPlus = LOAD16(data[27], data[28]);
        sAccYMinus = LOAD16(data[29], data[30]);
        sAccZPlus = LOAD16(data[31], data[32]);
        sAccZMinus = LOAD16(data[33], data[34]);

        flNumerator = (sGyroSpeedPlus + sGyroSpeedMinus) * GYRO_RES_PER_DEGREE;
        ctx->calibration[0].bias = sGyroPitchBias;
        ctx->calibration[0].sensitivity = flNumerator / (sGyroPitchPlus - sGyroPitchMinus);

        ctx->calibration[1].bias = sGyroYawBias;
        ctx->calibration[1].sensitivity = flNumerator / (sGyroYawPlus - sGyroYawMinus);

        ctx->calibration[2].bias = sGyroRollBias;
        ctx->calibration[2].sensitivity = flNumerator / (sGyroRollPlus - sGyroRollMinus);

        sRange2g = sAccXPlus - sAccXMinus;
        ctx->calibration[3].bias = sAccXPlus - sRange2g / 2;
        ctx->calibration[3].sensitivity = 2.0f * ACCEL_RES_PER_G / (float)sRange2g;

        sRange2g = sAccYPlus - sAccYMinus;
        ctx->calibration[4].bias = sAccYPlus - sRange2g / 2;
        ctx->calibration[4].sensitivity = 2.0f * ACCEL_RES_PER_G / (float)sRange2g;

        sRange2g = sAccZPlus - sAccZMinus;
        ctx->calibration[5].bias = sAccZPlus - sRange2g / 2;
        ctx->calibration[5].sensitivity = 2.0f * ACCEL_RES_PER_G / (float)sRange2g;

        ctx->hardware_calibration = SDL_TRUE;
        for (i = 0; i < 6; ++i) {
            float divisor = (i < 3 ? 64.0f : 1.0f);
#ifdef DEBUG_PS4_CALIBRATION
            SDL_Log("calibration[%d] bias = %d, sensitivity = %f\n", i, ctx->calibration[i].bias, ctx->calibration[i].sensitivity);
#endif
            /* Some controllers have a bad calibration */
            if ((SDL_abs(ctx->calibration[i].bias) > 1024) || (SDL_fabs(1.0f - ctx->calibration[i].sensitivity / divisor) > 0.5f)) {
#ifdef DEBUG_PS4_CALIBRATION
                SDL_Log("invalid calibration, ignoring\n");
#endif
                ctx->hardware_calibration = SDL_FALSE;
            }
        }
    } else {
#ifdef DEBUG_PS4_CALIBRATION
        SDL_Log("Calibration data not available\n");
#endif
    }
}

static float
HIDAPI_DriverPS4_ApplyCalibrationData(SDL_DriverPS4_Context *ctx, int index, Sint16 value)
{
    float result;

    if (ctx->hardware_calibration) {
        IMUCalibrationData *calibration = &ctx->calibration[index];

        result = (value - calibration->bias) * calibration->sensitivity;
    } else if (index < 3) {
        result = value * 64.f;
    } else {
        result = value;
    }

    /* Convert the raw data to the units expected by SDL */
    if (index < 3) {
        result = (result / GYRO_RES_PER_DEGREE) * (float)M_PI / 180.0f;
    } else {
        result = (result / ACCEL_RES_PER_G) * SDL_STANDARD_GRAVITY;
    }
    return result;
}

static int
HIDAPI_DriverPS4_UpdateEffects(SDL_HIDAPI_Device *device)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;
    DS4EffectsState_t effects;

    if (!ctx->enhanced_mode) {
        return SDL_Unsupported();
    }

    SDL_zero(effects);

    effects.ucRumbleLeft = ctx->rumble_left;
    effects.ucRumbleRight = ctx->rumble_right;

    /* Populate the LED state with the appropriate color from our lookup table */
    if (ctx->color_set) {
        effects.ucLedRed = ctx->led_red;
        effects.ucLedGreen = ctx->led_green;
        effects.ucLedBlue = ctx->led_blue;
    } else {
        SetLedsForPlayerIndex(&effects, ctx->player_index);
    }
    return HIDAPI_DriverPS4_SendJoystickEffect(device, ctx->joystick, &effects, sizeof(effects));
}

static void
HIDAPI_DriverPS4_TickleBluetooth(SDL_HIDAPI_Device *device)
{
    /* This is just a dummy packet that should have no effect, since we don't set the CRC */
    Uint8 data[78];

    SDL_zero(data);

    data[0] = k_EPS4ReportIdBluetoothEffects;
    data[1] = 0xC0;  /* Magic value HID + CRC */

    SDL_HIDAPI_SendRumble(device, data, sizeof(data));
}

static void
HIDAPI_DriverPS4_SetEnhancedMode(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;

    if (!ctx->enhanced_mode) {
        ctx->enhanced_mode = SDL_TRUE;

        SDL_PrivateJoystickAddTouchpad(joystick, 2);
        SDL_PrivateJoystickAddSensor(joystick, SDL_SENSOR_GYRO, 250.0f);
        SDL_PrivateJoystickAddSensor(joystick, SDL_SENSOR_ACCEL, 250.0f);

        HIDAPI_DriverPS4_UpdateEffects(device);
    }
}

static void SDLCALL SDL_PS4RumbleHintChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)userdata;

    /* This is a one-way trip, you can't switch the controller back to simple report mode */
    if (SDL_GetStringBoolean(hint, SDL_FALSE)) {
        HIDAPI_DriverPS4_SetEnhancedMode(ctx->device, ctx->joystick);
    }
}

static void
HIDAPI_DriverPS4_SetDevicePlayerIndex(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id, int player_index)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;

    if (!ctx) {
        return;
    }

    ctx->player_index = player_index;

    /* This will set the new LED state based on the new player index */
    HIDAPI_DriverPS4_UpdateEffects(device);
}

static SDL_bool
HIDAPI_DriverPS4_OpenJoystick(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    SDL_DriverPS4_Context *ctx;
    SDL_bool enhanced_mode = SDL_FALSE;

    ctx = (SDL_DriverPS4_Context *)SDL_calloc(1, sizeof(*ctx));
    if (!ctx) {
        SDL_OutOfMemory();
        return SDL_FALSE;
    }
    ctx->device = device;
    ctx->joystick = joystick;
    ctx->last_packet = SDL_GetTicks();

    device->dev = hid_open_path(device->path, 0);
    if (!device->dev) {
        SDL_free(ctx);
        SDL_SetError("Couldn't open %s", device->path);
        return SDL_FALSE;
    }
    device->context = ctx;

    /* Check for type of connection */
    ctx->is_dongle = (device->vendor_id == USB_VENDOR_SONY && device->product_id == USB_PRODUCT_SONY_DS4_DONGLE);
    if (ctx->is_dongle) {
        ctx->is_bluetooth = SDL_FALSE;
        ctx->official_controller = SDL_TRUE;
        enhanced_mode = SDL_TRUE;
    } else if (device->vendor_id == USB_VENDOR_SONY) {
        Uint8 data[USB_PACKET_LENGTH];
        int size;

        /* This will fail if we're on Bluetooth */
        size = ReadFeatureReport(device->dev, k_ePS4FeatureReportIdSerialNumber, data, sizeof(data));
        if (size >= 7) {
            char serial[18];

            SDL_snprintf(serial, sizeof(serial), "%.2x-%.2x-%.2x-%.2x-%.2x-%.2x",
                data[6], data[5], data[4], data[3], data[2], data[1]);
            joystick->serial = SDL_strdup(serial);
            ctx->is_bluetooth = SDL_FALSE;
            enhanced_mode = SDL_TRUE;
        } else {
            ctx->is_bluetooth = SDL_TRUE;

            /* Read a report to see if we're in enhanced mode */
            size = hid_read_timeout(device->dev, data, sizeof(data), 16);
#ifdef DEBUG_PS4_PROTOCOL
            if (size > 0) {
                HIDAPI_DumpPacket("PS4 first packet: size = %d", data, size);
            } else {
                SDL_Log("PS4 first packet: size = %d\n", size);
            }
#endif
            if (size > 0 &&
                data[0] >= k_EPS4ReportIdBluetoothState1 &&
                data[0] <= k_EPS4ReportIdBluetoothState9) {
                enhanced_mode = SDL_TRUE;
            }
        }
        ctx->official_controller = SDL_TRUE;
    } else {
        /* Third party controllers appear to all be wired */
        ctx->is_bluetooth = SDL_FALSE;
        enhanced_mode = SDL_TRUE;
    }
#ifdef DEBUG_PS4
    SDL_Log("PS4 dongle = %s, bluetooth = %s\n", ctx->is_dongle ? "TRUE" : "FALSE", ctx->is_bluetooth ? "TRUE" : "FALSE");
#endif

    /* Check to see if audio is supported */
    if (device->vendor_id == USB_VENDOR_SONY &&
        (device->product_id == USB_PRODUCT_SONY_DS4_SLIM || device->product_id == USB_PRODUCT_SONY_DS4_DONGLE)) {
        ctx->audio_supported = SDL_TRUE;
    }

    if (HIDAPI_DriverPS4_CanRumble(device->vendor_id, device->product_id)) {
        ctx->effects_supported = SDL_TRUE;
    }

    if (!joystick->serial && device->serial && SDL_strlen(device->serial) == 12) {
        int i, j;
        char serial[18];

        j = -1;
        for (i = 0; i < 12; i += 2) {
            j += 1;
            SDL_memcpy(&serial[j], &device->serial[i], 2);
            j += 2;
            serial[j] = '-';
        }
        serial[j] = '\0';

        joystick->serial = SDL_strdup(serial);
    }

    /* Initialize player index (needed for setting LEDs) */
    ctx->player_index = SDL_JoystickGetPlayerIndex(joystick);

    /* Initialize the joystick capabilities
     *
     * We can't dynamically add the touchpad button, so always report it here
     */
    joystick->nbuttons = 16;
    joystick->naxes = SDL_CONTROLLER_AXIS_MAX;
    joystick->epowerlevel = ctx->is_bluetooth ? SDL_JOYSTICK_POWER_UNKNOWN : SDL_JOYSTICK_POWER_WIRED;

    if (enhanced_mode) {
        HIDAPI_DriverPS4_SetEnhancedMode(device, joystick);
    } else {
        SDL_AddHintCallback(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE,
                            SDL_PS4RumbleHintChanged, ctx);
    }
    return SDL_TRUE;
}

static int
HIDAPI_DriverPS4_RumbleJoystick(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;

    ctx->rumble_left = (low_frequency_rumble >> 8);
    ctx->rumble_right = (high_frequency_rumble >> 8);

    return HIDAPI_DriverPS4_UpdateEffects(device);
}

static int
HIDAPI_DriverPS4_RumbleJoystickTriggers(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static SDL_bool
HIDAPI_DriverPS4_HasJoystickLED(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    return SDL_TRUE;
}

static int
HIDAPI_DriverPS4_SetJoystickLED(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;

    ctx->color_set = SDL_TRUE;
    ctx->led_red = red;
    ctx->led_green = green;
    ctx->led_blue = blue;

    return HIDAPI_DriverPS4_UpdateEffects(device);
}

static int
HIDAPI_DriverPS4_SendJoystickEffect(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, const void *effect, int size)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;
    Uint8 data[78];
    int report_size, offset;

    if (!ctx->effects_supported) {
        return SDL_Unsupported();
    }

    if (!ctx->enhanced_mode) {
        HIDAPI_DriverPS4_SetEnhancedMode(device, joystick);
    }

    SDL_zero(data);

    if (ctx->is_bluetooth) {
        data[0] = k_EPS4ReportIdBluetoothEffects;
        data[1] = 0xC0 | 0x04;  /* Magic value HID + CRC, also sets interval to 4ms for samples */
        data[3] = 0x03;  /* 0x1 is rumble, 0x2 is lightbar, 0x4 is the blink interval */

        report_size = 78;
        offset = 6;
    } else {
        data[0] = k_EPS4ReportIdUsbEffects;
        data[1] = 0x07;  /* Magic value */

        report_size = 32;
        offset = 4;
    }

    SDL_memcpy(&data[offset], effect, SDL_min((sizeof(data) - offset), (size_t)size));

    if (ctx->is_bluetooth) {
        /* Bluetooth reports need a CRC at the end of the packet (at least on Linux) */
        Uint8 ubHdr = 0xA2; /* hidp header is part of the CRC calculation */
        Uint32 unCRC;
        unCRC = SDL_crc32(0, &ubHdr, 1);
        unCRC = SDL_crc32(unCRC, data, (size_t)(report_size - sizeof(unCRC)));
        SDL_memcpy(&data[report_size - sizeof(unCRC)], &unCRC, sizeof(unCRC));
    }

    if (SDL_HIDAPI_SendRumble(device, data, report_size) != report_size) {
        return SDL_SetError("Couldn't send rumble packet");
    }
    return 0;
}

static int
HIDAPI_DriverPS4_SetJoystickSensorsEnabled(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, SDL_bool enabled)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;

    if (!ctx->enhanced_mode) {
        return SDL_Unsupported();
    }

    if (enabled) {
        HIDAPI_DriverPS4_LoadCalibrationData(device);
    }
    ctx->report_sensors = enabled;

    return 0;
}

static void
HIDAPI_DriverPS4_HandleStatePacket(SDL_Joystick *joystick, hid_device *dev, SDL_DriverPS4_Context *ctx, PS4StatePacket_t *packet)
{
    static const float TOUCHPAD_SCALEX = 1.0f / 1920;
    static const float TOUCHPAD_SCALEY = 1.0f / 920;    /* This is noted as being 944 resolution, but 920 feels better */
    Sint16 axis;
    Uint8 touchpad_state;
    int touchpad_x, touchpad_y;

    if (ctx->last_state.rgucButtonsHatAndCounter[0] != packet->rgucButtonsHatAndCounter[0]) {
        {
            Uint8 data = (packet->rgucButtonsHatAndCounter[0] >> 4);

            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_X, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_A, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_B, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
            SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_Y, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        }
        {
            Uint8 data = (packet->rgucButtonsHatAndCounter[0] & 0x0F);
            SDL_bool dpad_up = SDL_FALSE;
            SDL_bool dpad_down = SDL_FALSE;
            SDL_bool dpad_left = SDL_FALSE;
            SDL_bool dpad_right = SDL_FALSE;

            switch (data) {
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
    }

    if (ctx->last_state.rgucButtonsHatAndCounter[1] != packet->rgucButtonsHatAndCounter[1]) {
        Uint8 data = packet->rgucButtonsHatAndCounter[1];

        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_BACK, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_START, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSTICK, (data & 0x40) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSTICK, (data & 0x80) ? SDL_PRESSED : SDL_RELEASED);
    }

    /* Some fightsticks, ex: Victrix FS Pro will only this these digital trigger bits and not the analog values so this needs to run whenever the
       trigger is evaluated
    */
    if ((packet->rgucButtonsHatAndCounter[1] & 0x0C) != 0) {
        Uint8 data = packet->rgucButtonsHatAndCounter[1];
        packet->ucTriggerLeft = (data & 0x04) && packet->ucTriggerLeft == 0 ? 255 : packet->ucTriggerLeft;
        packet->ucTriggerRight = (data & 0x08) && packet->ucTriggerRight == 0 ? 255 : packet->ucTriggerRight;
    }

    if (ctx->last_state.rgucButtonsHatAndCounter[2] != packet->rgucButtonsHatAndCounter[2]) {
        Uint8 data = (packet->rgucButtonsHatAndCounter[2] & 0x03);

        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, 15, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
    }

    axis = ((int)packet->ucTriggerLeft * 257) - 32768;
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT, axis);
    axis = ((int)packet->ucTriggerRight * 257) - 32768;
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, axis);
    axis = ((int)packet->ucLeftJoystickX * 257) - 32768;
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, axis);
    axis = ((int)packet->ucLeftJoystickY * 257) - 32768;
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, axis);
    axis = ((int)packet->ucRightJoystickX * 257) - 32768;
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTX, axis);
    axis = ((int)packet->ucRightJoystickY * 257) - 32768;
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTY, axis);

    if (packet->ucBatteryLevel & 0x10) {
        joystick->epowerlevel = SDL_JOYSTICK_POWER_WIRED;
    } else {
        /* Battery level ranges from 0 to 10 */
        int level = (packet->ucBatteryLevel & 0xF);
        if (level == 0) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_EMPTY;
        } else if (level <= 2) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_LOW;
        } else if (level <= 7) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_MEDIUM;
        } else {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_FULL;
        }
    }

    touchpad_state = ((packet->ucTouchpadCounter1 & 0x80) == 0) ? SDL_PRESSED : SDL_RELEASED;
    touchpad_x = packet->rgucTouchpadData1[0] | (((int)packet->rgucTouchpadData1[1] & 0x0F) << 8);
    touchpad_y = (packet->rgucTouchpadData1[1] >> 4) | ((int)packet->rgucTouchpadData1[2] << 4);
    SDL_PrivateJoystickTouchpad(joystick, 0, 0, touchpad_state, touchpad_x * TOUCHPAD_SCALEX, touchpad_y * TOUCHPAD_SCALEY, touchpad_state ? 1.0f : 0.0f);

    touchpad_state = ((packet->ucTouchpadCounter2 & 0x80) == 0) ? SDL_PRESSED : SDL_RELEASED;
    touchpad_x = packet->rgucTouchpadData2[0] | (((int)packet->rgucTouchpadData2[1] & 0x0F) << 8);
    touchpad_y = (packet->rgucTouchpadData2[1] >> 4) | ((int)packet->rgucTouchpadData2[2] << 4);
    SDL_PrivateJoystickTouchpad(joystick, 0, 1, touchpad_state, touchpad_x * TOUCHPAD_SCALEX, touchpad_y * TOUCHPAD_SCALEY, touchpad_state ? 1.0f : 0.0f);

    if (ctx->report_sensors) {
        float data[3];

        data[0] = HIDAPI_DriverPS4_ApplyCalibrationData(ctx, 0, LOAD16(packet->rgucGyroX[0], packet->rgucGyroX[1]));
        data[1] = HIDAPI_DriverPS4_ApplyCalibrationData(ctx, 1, LOAD16(packet->rgucGyroY[0], packet->rgucGyroY[1]));
        data[2] = HIDAPI_DriverPS4_ApplyCalibrationData(ctx, 2, LOAD16(packet->rgucGyroZ[0], packet->rgucGyroZ[1]));
        SDL_PrivateJoystickSensor(joystick, SDL_SENSOR_GYRO, data, 3);

        data[0] = HIDAPI_DriverPS4_ApplyCalibrationData(ctx, 3, LOAD16(packet->rgucAccelX[0], packet->rgucAccelX[1]));
        data[1] = HIDAPI_DriverPS4_ApplyCalibrationData(ctx, 4, LOAD16(packet->rgucAccelY[0], packet->rgucAccelY[1]));
        data[2] = HIDAPI_DriverPS4_ApplyCalibrationData(ctx, 5, LOAD16(packet->rgucAccelZ[0], packet->rgucAccelZ[1]));
        SDL_PrivateJoystickSensor(joystick, SDL_SENSOR_ACCEL, data, 3);
    }

    SDL_memcpy(&ctx->last_state, packet, sizeof(ctx->last_state));
}

static SDL_bool
HIDAPI_DriverPS4_UpdateDevice(SDL_HIDAPI_Device *device)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;
    SDL_Joystick *joystick = NULL;
    Uint8 data[USB_PACKET_LENGTH*2];
    int size;
    int packet_count = 0;

    if (device->num_joysticks > 0) {
        joystick = SDL_JoystickFromInstanceID(device->joysticks[0]);
    }
    if (!joystick) {
        return SDL_FALSE;
    }

    while ((size = hid_read_timeout(device->dev, data, sizeof(data), 0)) > 0) {
#ifdef DEBUG_PS4_PROTOCOL
        HIDAPI_DumpPacket("PS4 packet: size = %d", data, size);
#endif
        ++packet_count;
        ctx->last_packet = SDL_GetTicks();

        switch (data[0]) {
        case k_EPS4ReportIdUsbState:
            HIDAPI_DriverPS4_HandleStatePacket(joystick, device->dev, ctx, (PS4StatePacket_t *)&data[1]);
            break;
        case k_EPS4ReportIdBluetoothState1:
        case k_EPS4ReportIdBluetoothState2:
        case k_EPS4ReportIdBluetoothState3:
        case k_EPS4ReportIdBluetoothState4:
        case k_EPS4ReportIdBluetoothState5:
        case k_EPS4ReportIdBluetoothState6:
        case k_EPS4ReportIdBluetoothState7:
        case k_EPS4ReportIdBluetoothState8:
        case k_EPS4ReportIdBluetoothState9:
            if (!ctx->enhanced_mode) {
                /* This is the extended report, we can enable effects now */
                HIDAPI_DriverPS4_SetEnhancedMode(device, joystick);
            }
            /* Bluetooth state packets have two additional bytes at the beginning, the first notes if HID is present */
            if (data[1] & 0x80) {
                HIDAPI_DriverPS4_HandleStatePacket(joystick, device->dev, ctx, (PS4StatePacket_t*)&data[3]);
            }
            break;
        default:
#ifdef DEBUG_JOYSTICK
            SDL_Log("Unknown PS4 packet: 0x%.2x\n", data[0]);
#endif
            break;
        }
    }

    if (ctx->is_bluetooth && packet_count == 0) {
        /* Check to see if it looks like the device disconnected */
        if (SDL_TICKS_PASSED(SDL_GetTicks(), ctx->last_packet + BLUETOOTH_DISCONNECT_TIMEOUT_MS)) {
            /* Send an empty output report to tickle the Bluetooth stack */
            HIDAPI_DriverPS4_TickleBluetooth(device);
        }
    }

    if (size < 0) {
        /* Read error, device is disconnected */
        HIDAPI_JoystickDisconnected(device, joystick->instance_id);
    }
    return (size >= 0);
}

static void
HIDAPI_DriverPS4_CloseJoystick(SDL_HIDAPI_Device *device, SDL_Joystick *joystick)
{
    SDL_DriverPS4_Context *ctx = (SDL_DriverPS4_Context *)device->context;

    SDL_DelHintCallback(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE,
                        SDL_PS4RumbleHintChanged, ctx);

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
HIDAPI_DriverPS4_FreeDevice(SDL_HIDAPI_Device *device)
{
}

SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverPS4 =
{
    SDL_HINT_JOYSTICK_HIDAPI_PS4,
    SDL_TRUE,
    HIDAPI_DriverPS4_IsSupportedDevice,
    HIDAPI_DriverPS4_GetDeviceName,
    HIDAPI_DriverPS4_InitDevice,
    HIDAPI_DriverPS4_GetDevicePlayerIndex,
    HIDAPI_DriverPS4_SetDevicePlayerIndex,
    HIDAPI_DriverPS4_UpdateDevice,
    HIDAPI_DriverPS4_OpenJoystick,
    HIDAPI_DriverPS4_RumbleJoystick,
    HIDAPI_DriverPS4_RumbleJoystickTriggers,
    HIDAPI_DriverPS4_HasJoystickLED,
    HIDAPI_DriverPS4_SetJoystickLED,
    HIDAPI_DriverPS4_SendJoystickEffect,
    HIDAPI_DriverPS4_SetJoystickSensorsEnabled,
    HIDAPI_DriverPS4_CloseJoystick,
    HIDAPI_DriverPS4_FreeDevice,
};

#endif /* SDL_JOYSTICK_HIDAPI_PS4 */

#endif /* SDL_JOYSTICK_HIDAPI */

/* vi: set ts=4 sw=4 expandtab: */
