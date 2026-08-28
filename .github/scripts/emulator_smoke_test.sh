#!/usr/bin/env bash
set -euo pipefail

echo "Waiting for a device to appear..."
adb wait-for-device

echo "----- adb devices -l -----"
adb devices -l
echo "--------------------------"

# Count entries in the ready ('device') state, skipping the
# "List of devices attached" header and any offline/unauthorized rows.
device_count="$(adb devices -l | awk 'NR>1 && $2 == "device"' | wc -l | tr -d '[:space:]')"
echo "Devices in 'device' state: ${device_count}"

if [ "${device_count}" -eq 0 ]; then
  echo "::error::adb devices -l returned an empty device list"
  exit 1
fi

echo "Success: ${device_count} device(s) connected."

# Locate the built sample-render-validation APK
APK_PATH="out/sample-render-validation-debug.apk"
if [ ! -f "${APK_PATH}" ]; then
  APK_PATH="android/samples/sample-render-validation/build/outputs/apk/debug/sample-render-validation-debug.apk"
fi

if [ ! -f "${APK_PATH}" ]; then
  echo "::error::Render validation APK not found at ${APK_PATH}"
  exit 1
fi

echo "Installing render validation APK from ${APK_PATH}..."
adb install -r "${APK_PATH}"

PACKAGE="com.google.android.filament.validation"

echo "Launching ${PACKAGE} on emulator to generate goldens..."
adb shell am start -n "${PACKAGE}/.MainActivity" \
  --ez auto_run true \
  --ez generate_goldens true \
  --ez auto_export true

echo "Waiting for generated goldens zip archive..."
TIMEOUT=600 # 10 minutes timeout
ELAPSED=0
ZIP_FILE=""

mkdir -p out

while [ ${ELAPSED} -lt ${TIMEOUT} ]; do
  # Check app internal storage (files/)
  INT_FILES=$(adb shell "run-as ${PACKAGE} ls -1 files/" 2>/dev/null | grep '\.zip$' || true)
  # Check app external storage (/sdcard/Android/data/...)
  EXT_FILES=$(adb shell "ls -1 /sdcard/Android/data/${PACKAGE}/files/" 2>/dev/null | grep '\.zip$' || true)

  if [ -n "${INT_FILES}" ]; then
    ZIP_NAME=$(echo "${INT_FILES}" | head -n 1 | tr -d '\r')
    echo "Found exported zip in internal storage: ${ZIP_NAME}"
    adb shell "run-as ${PACKAGE} cat files/${ZIP_NAME}" > "out/${ZIP_NAME}"
    ZIP_FILE="out/${ZIP_NAME}"
    break
  elif [ -n "${EXT_FILES}" ]; then
    ZIP_NAME=$(echo "${EXT_FILES}" | head -n 1 | tr -d '\r')
    echo "Found exported zip in external storage: ${ZIP_NAME}"
    adb pull "/sdcard/Android/data/${PACKAGE}/files/${ZIP_NAME}" "out/${ZIP_NAME}"
    ZIP_FILE="out/${ZIP_NAME}"
    break
  fi

  sleep 5
  ELAPSED=$((ELAPSED + 5))
  if [ $((ELAPSED % 30)) -eq 0 ]; then
    echo "Waiting for render validation test execution... (${ELAPSED}s elapsed)"
  fi
done

if [ -z "${ZIP_FILE}" ] || [ ! -s "${ZIP_FILE}" ]; then
  echo "::error::Failed to retrieve generated goldens zip from emulator within ${TIMEOUT} seconds"
  exit 1
fi

echo "Success: Retrieved generated golden zip: ${ZIP_FILE} ($(wc -c < "${ZIP_FILE}") bytes)"
