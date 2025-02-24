# ANGLE Settings UI

## Introduction

ANGLE Settings UI is a UI shipped as part of the ANGLE apk. The ANGLE apk can ship with a list rules
of applications using different OpenGL ES driver. The UI provides two functionalities:

1) Enable to show a message when ANGLE is loaded into the launched application;
2) Allow to select driver choice in the UI for applications.

## The rule file

Currently the ANGLE apk supports two rules: ANGLE and native OpenGL ES driver.

The rule file is a file that contains a JSON string, the format is shown below:

```
{
    "rules":[
        {
            "description": "Applications in this list will use ANGLE",
            "choice": "angle",
            "apps": [
                {
                    "packageName": "com.android.example.a"
                },
                {
                    "packageName": "com.android.example.b"
                }
            ]
        },
        {
            "description": "Applications in this list will not use ANGLE",
            "choice": "native",
            "apps":[
                {
                    "packageName": "com.android.example.c"
                }
            ]
        }
    ]
}
```

The ANGLE JSON rules are parsed only when `ACTION_BOOT_COMPLETED` or `ACTION_MY_PACKAGE_REPLACED` is
received. After the JSON rules are parsed, the result will be stored in `SharedPreferences` as
key-value pair, with the key being the package name and the value being the driver selection choice.
The JSON parsing code is in `AngleRuleHelper`.

After parsing, the rules are converted to global settings variables and applied to the system. This
is done in `Receiver`.

The UI logic is mainly in `MainFragment`, and the `GlobalSettings` is merely for manipulating
settings global variables and updating `SharedPreferences`. When a user changes the driver choice
of an application, the update will go into `GlobalSettings` and `SharedPreferences` respectively.

The `SharedPreferences` is the source of truth and the code should always query the driver choice
from it with the package name. The `SharedPreferences` should only be updated within
`GlobalSettings` and `AngleRuleHelper`.

The settings global variables may also be changed via `adb` command by the users, often time ANGLE
for Android developers. Note that every time a boot event happens, it is expected that all previous
values in settings global variables will be cleared and only values from the ANGLE JSON rule file
will take effect.

## Developer options

The ANGLE Settings UI is registered as a dynamic setting entry in the development component via

```
<intent-filter>
    <action android:name="com.android.settings.action.IA_SETTINGS" />
</intent-filter>
<meta-data android:name="com.android.settings.category"
        android:value="com.android.settings.category.ia.development" />
```

And hence the UI shows up in Developer options. If the Developer options are disabled, all settings
global variables will be cleared.
