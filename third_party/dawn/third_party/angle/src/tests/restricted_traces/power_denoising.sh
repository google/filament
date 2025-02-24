#!/bin/bash
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

declare -a Packages=("com.android.chrome" "com.android.phone" "com.google.android.apps.gcs" "com.google.android.apps.internal.betterbug" "com.google.android.apps.nbu.files" "com.google.android.apps.scone" "com.google.android.apps.work.clouddpc" "com.google.android.as" "com.google.android.contacts" "com.google.android.dialer" "com.google.android.googlequicksearchbox" "com.google.android.ims" "com.google.android.tts" "com.google.android.videos" "com.google.android.GoogleCamera" "com.google.android.apps.maps" "com.google.android.apps.photos" "com.google.android.apps.youtube.music" "com.google.android.youtube" "com.google.android.apps.docs" "com.google.android.apps.gcs" "com.google.android.apps.messaging" "com.google.android.apps.turbo" "com.google.android.apps.tycho" "com.google.android.calculator" "com.google.android.calendar" "com.google.android.configupdater" "com.google.android.contacts" "com.google.android.deskclock" "com.google.android.gm" "com.google.android.inputmethod.latin" "com.google.android.marvin.talkback" "com.google.android.videos" "com.android.ramdump" "com.google.android.volta" "com.google.android.settings.intelligence" "com.google.android.partnersetup" "com.google.android.apps.tips" "com.android.vending")

SetGservicesFlag() (
  echo "Setting $1 to $2 -START"
  adb shell am broadcast -a com.google.gservices.intent.action.GSERVICES_OVERRIDE \
      -e "${1}" "${2}"
  wanted="${1}=${2}"
  i=1
  while [[ $i -le 20 ]]
  do
    echo "Try #$i:"
    if adb shell dumpsys activity provider GservicesProvider | grep "$wanted" &>/dev/null;
    then
      echo "Succeeded"
      break
    else
      echo "Failed"
      sleep 0.05
    fi
    (( i++ ))
  done
  echo "Setting $1 to $2 -Done"
)

adb root

# Disable unnecessary packages
for val in ${StringArray[@]}; do
   adb shell pm disable $val
done

# Disable modem logging, ambient EQ, and adaptive brightness.
adb shell setprop vendor.sys.modem.diag.mdlog false
adb shell settings put secure display_white_balance_enabled 0
adb shell settings put system screen_brightness_mode 0

# Disable bg dex2oat
adb shell setprop pm.dexopt.disable_bg_dexopt true

# Disable moisture detection
contaminant_detection_node=$(adb shell find /sys -name contaminant_detection)
adb shell "echo 0 > ${contaminant_detection_node}"
adb shell setprop vendor.usb.contaminantdisable true

# Direct Phenotype to a local Heterodyne server.
SetGservicesFlag gms:phenotype:debug_allow_http "true"
SetGservicesFlag gms:phenotype:configurator:service_url "http://localhost:9879"

# Remove any production configurations already downloaded.
# Restarts GMS Core.
adb shell pm clear com.google.android.gms

adb shell stop
adb shell start
sleep 40s
adb shell input keyevent 82
