# Performance Analysis

## Android

### Prerequisites

- Download and install Android GPU Inspector (AGI). See https://developer.android.com/agi.

### Android with the Vulkan Backend

---

#### Profiling

1. Build a release build of Filament with the Vulkan backend profiling enabled _(debug builds for this are useless)_

    ```shell
    # the following command assumes you are in the root filament directory
    # and ANDROID_HOME is exported (and possibly also CC and CXX on linux as needed)
    #
    # Note that you can speed this up a bit (and reduce disk space usage) by limiting the target to just 
    # the ABI you plan on testing with the -q flag, e.g. -q arm64-v8a. If you do this, you
    # will need to update the android/gradle.properties file to specify the ABI(s) you are targeting
    # with the com.google.android.filament.abis property.
    ./build.sh -p android,desktop -i release
    ```
1. Connect your Android device to your computer via a USB cord with USB debugging enabled and configure
   the system property that will default Filament's backend to Vulkan, e.g.:
    ```shell
    adb shell setprop debug.filament.backend 2
    ```
1. Build and run a sample, e.g. sample-gltf-viewer, on your Android device
   _([Android Studio](https://developer.android.com/studio) recommended)_.
1. Run [AGI](https://developer.android.com/agi) and follow instructions to profile the app/system
   with trace capture(s). See https://developer.android.com/agi/start for more details to
   get started. Generally, we are most interested in the performance of the FEngine::loop thread and
   the GPU timeline as it relates to that.

### Android with the WebGPU Backend

---

#### Profiling

1. Build a release build of Filament with the WebGPU backend profiling enabled _(debug builds for this are useless)_

    ```shell
    # the following command assumes you are in the root filament directory
    # and ANDROID_HOME is exported (and possibly also CC and CXX on linux as needed).
    #
    # Note that you can speed this up a bit (and reduce disk space usage) by limiting the target to just 
    # the ABI you plan on testing with the -q flag, e.g. -q arm64-v8a. If you do this, you
    # will need to update the android/gradle.properties file to specify the ABI(s) you are targeting
    # with the com.google.android.filament.abis property.
    ./build.sh -W -p android,desktop -i release
    ```
1. Connect your Android device to your computer via a USB cord with USB debugging enabled and configure
   the system property that will default Filament's backend to WebGPU, e.g.:
    ```shell
    adb shell setprop debug.filament.backend 4
    ```
1. Build and run a sample, e.g. sample-gltf-viewer, on your Android device
   _([Android Studio](https://developer.android.com/studio) recommended)_.
   Note that you will need to define the `com.google.android.filament.include-webgpu`
   property in the [android/gradle.properties)](android/gradle.properties) file
   _(uncomment the line that has it commented out)_ to ensure the WebGPU backend is
   supported when building the sample.
1. Run [AGI](https://developer.android.com/agi) and follow instructions to profile the app/system
   with trace capture(s). See https://developer.android.com/agi/start for more details to
   get started. Generally, we are most interested in the performance of the FEngine::loop thread and
   the GPU timeline as it relates to that.
