/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple test of the SDL sensor code */

#include "SDL.h"

static const char *GetSensorTypeString(SDL_SensorType type)
{
    static char unknown_type[64];

    switch (type)
    {
    case SDL_SENSOR_INVALID:
        return "SDL_SENSOR_INVALID";
    case SDL_SENSOR_UNKNOWN:
        return "SDL_SENSOR_UNKNOWN";
    case SDL_SENSOR_ACCEL:
        return "SDL_SENSOR_ACCEL";
    case SDL_SENSOR_GYRO:
        return "SDL_SENSOR_GYRO";
    default:
        SDL_snprintf(unknown_type, sizeof(unknown_type), "UNKNOWN (%d)", type);
        return unknown_type;
    }
}

static void HandleSensorEvent(SDL_SensorEvent *event)
{
    SDL_Sensor *sensor = SDL_SensorFromInstanceID(event->which);
    if (!sensor) {
        SDL_Log("Couldn't get sensor for sensor event\n");
        return;
    }

    switch (SDL_SensorGetType(sensor)) {
    case SDL_SENSOR_ACCEL:
        SDL_Log("Accelerometer update: %.2f, %.2f, %.2f\n", event->data[0], event->data[1], event->data[2]);
        break;
    case SDL_SENSOR_GYRO:
        SDL_Log("Gyro update: %.2f, %.2f, %.2f\n", event->data[0], event->data[1], event->data[2]);
        break;
    default:
        SDL_Log("Sensor update for sensor type %s\n", GetSensorTypeString(SDL_SensorGetType(sensor)));
        break;
    }
}

int
main(int argc, char **argv)
{
    int i;
    int num_sensors, num_opened;

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_SENSOR) < 0) {
        SDL_Log("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

    num_sensors = SDL_NumSensors();
    num_opened = 0;

    SDL_Log("There are %d sensors available\n", num_sensors);
    for (i = 0; i < num_sensors; ++i) {
        SDL_Log("Sensor %d: %s, type %s, platform type %d\n",
            SDL_SensorGetDeviceInstanceID(i),
            SDL_SensorGetDeviceName(i),
            GetSensorTypeString(SDL_SensorGetDeviceType(i)),
            SDL_SensorGetDeviceNonPortableType(i));

        if (SDL_SensorGetDeviceType(i) != SDL_SENSOR_UNKNOWN) {
            SDL_Sensor *sensor = SDL_SensorOpen(i);
            if (sensor == NULL) {
                SDL_Log("Couldn't open sensor %d: %s\n", SDL_SensorGetDeviceInstanceID(i), SDL_GetError());
            } else {
                ++num_opened;
            }
        }
    }
    SDL_Log("Opened %d sensors\n", num_opened);

    if (num_opened > 0) {
        SDL_bool done = SDL_FALSE;
        SDL_Event event;

        SDL_CreateWindow("Sensor Test", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
        while (!done) {
            /* Update to get the current event state */
            SDL_PumpEvents();

            /* Process all currently pending events */
            while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 1) {
                switch (event.type) {
                case SDL_SENSORUPDATE:
                    HandleSensorEvent(&event.sensor);
                    break;
                case SDL_MOUSEBUTTONUP:
                case SDL_KEYUP:
                case SDL_QUIT:
                    done = SDL_TRUE;
                    break;
                default:
                    break;
                }
            }
        }
    }

    SDL_Quit();
    return (0);
}
