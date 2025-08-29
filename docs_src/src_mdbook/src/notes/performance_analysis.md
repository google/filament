# Performance Analysis

## Android

### Prerequisites

- Download and install Android GPU Inspector (AGI). See https://developer.android.com/agi.

### Android with the Vulkan Backend

---

#### Profiling

1. Build a release build of Filament with the applicable backend(s) enabled
   _(+ any special flags for enabling sys strace. Nothing special is needed for Vulkan or WebGPU aside from
   building a release build with no flags)_ _(debug builds for this are useless)_

    ```shell
    # the following command assumes you are in the root filament directory
    # and ANDROID_HOME is exported (and possibly also CC and CXX on linux as needed)
    #
    # NOTE: to build with WebGPU support you need to explicitly include the -W flag
    # (it doesn't get compiled in by default), e.g.:
    # ./build.sh -W -p android,desktop -i release
    #
    # Note that you can speed this up a bit (and reduce disk space usage) by limiting the target to just 
    # the ABI you plan on testing with the -q flag, e.g. -q arm64-v8a. If you do this, you
    # will need to update the android/gradle.properties file to specify the ABI(s) you are targeting
    # with the com.google.android.filament.abis property.
    # Thus, a build command that would target BOTH Vulkan AND WebGPU AND only target the ARM64 ABI would look something
    # like:
    # ./build.sh -W -q arm64-v8a -p android,desktop -i release
    ./build.sh -p android,desktop -i release
    ```
1. Connect your Android device to your computer via a USB cord with USB debugging enabled and configure
   the system property to default to the desired backend, e.g.
   _(to determine how these numbers map to the backends, see the `enum class Backend`
   definition in [filament/filament/backend/include/backend/DriverEnums.h](../../../../filament/backend/include/backend/DriverEnums.h))_:
    ```shell
    # to set the backend to Vulkan: 
    adb shell setprop debug.filament.backend 2
    # to set the backend to WebGPU:
    adb shell setprop debug.filament.backend 4
    # to view the current property:
    adb shell getprop debug.filament.backend
    ```
1. Build and run a sample, e.g. sample-gltf-viewer, on your Android device
   _([Android Studio](https://developer.android.com/studio) recommended)_.
1. Run [AGI](https://developer.android.com/agi) and follow instructions to profile the app/system
   with trace capture(s). See https://developer.android.com/agi/start for more details to
   get started.
   - We typically only run "Capture System Profiler trace" _(not necessarily "Capture Frame Profiler trace")_
   - When configuring the trace:
     - For both the WebGPU and Vulkan backends configure the profiler for the Vulkan API
       _(since WebGPU should be using Vulkan under the hood as well)_
     - Running for ~1 seconds should suffice
     - Hit the "Configure" button in Trace objects, select "Switch to advanced mode" and add:
       ```
       data_sources {
         config {
           name: "track_event"
           track_event_config {
             disabled_categories: "*"
             enabled_categories: "filament/filament"
             enabled_categories: "filament/jobsystem"
             enabled_categories: "filament/gltfio"
           }
         }
       }
       ```
   - One you open the trace, zoom into a series of frames to get a sense of generally how long they typically take
    _(use `W`, `S`, `A` and `D` keys and mouse wheel for navigation)_ and find a representative one.
     We are most interested in the performance of the
     `FEngine::loop` thread, how long it takes, overlap in activities/processes/commands, reduction in queue submissions,
     etc. Similarly, we can view GPU timeline as it relates to that. We want to see overlapping
     shader invocations and non-interrupted fragment shader runs.
