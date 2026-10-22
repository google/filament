# Performance Analysis

## Android

### Prerequisites

- Download and install Android GPU Inspector (AGI). See https://developer.android.com/agi.

---

#### Profiling

1. Before profiling the application or analyzing the performance in a consistent way, ideally
   the **GPU frequency on the target hardware should get locked**. In order to do this, you need to do the following:
   1. Ensure your device is OEM unlocked. If your phone is carrier-locked, you may need
      to wait a period, such as 60 or 90 days after activation to be eligible for unlocking.
      Some phones don't support this at all. You will need to enable developer options
      (e.g. Settings > About Phone and tap the Build number 7 times). You need to go to
      Settings > System > Developer options and toggle on "OEM unlocking".
   1. Next, you need to unlock the phone. You will need to install Android SDK Platform
      Tools on you computer, enable "USB debugging" in your phone's Settings > System > Developer options.
   1. Connect the phone to the computer via a USB cable and run from the command line:
      ```shell
      adb reboot bootloader
      # once the phone is in bootloader mode, run the following to begin the unlocking
      # process:
      fastboot flashing unlock
      ```
      A warning will appear on your phone's screen. Use the volume buttons to navigate and
      the power button to select the "Unlock the bootloader" option. The phone will perform
      a factory data reset and reboot with an unlocked bootloader.
   1. You would need to flash an image to the phone with root permissions, such as a *-userdebug or *-eng
      build. One way to do this is with the [Android Flash Tool](https://flash.android.com/). Connect the
      tool to your device and find a build to flash ending in `-userdebug` or `-eng`. Once you have that
      selected run `Install build`.
   1. Shell into your device as root and configure the gpu frequency to be locked, e.g.:
      ```shell
      adb shell
      su
      # navigate to the system GPU directory. this varies on different phones. One phone might have
      # it at /sys/class/kgsl/kgsl-3d0 and another might be in something similar, maybe with "mali" instead
      # of "kgsl". At the time of writing this for the device at hand it was /sys/devices/platform/1f000000.mali
      cd /sys/devices/platform/1f000000.mali
      # get the current available GPU governors and frequencies.
      # note that some systems may have these at gpu_available_governors and gpu_available_frequencies, but
      # the system at the time of writing this had available_governors and available_frequences
      cat available_governors
      cat available_frequencies
      # depending on the governors, you may want to set it prioritize performance over other things like
      # battery. Some systems allow you to do this with something like (although, for the device used at the
      # time of writing this did not have an equivalent option):
      echo performance > gpu_governor
      # finally, lock your frequency in, usually to something high like 897 MHz. Some systems may have you
      # pipe the value to gpu_min_freq and gpu_max_freq, but the system used at the time of writing this had:
      echo 940000 > hint_min_freq
      echo 940000 > hint_max_freq
      # you can typically verify the GPU is running at that frequency consistently by running something like
      # the following a few times over time, which should show the frequency you want to lock the device to:
      cat cur_freq
      ```
   1. You may need to re-apply the `hint_min_freq` just before starting the profiling trace and check before and after
      that the frequency remained at the value expected. Some systems may adjust the frequency on you, but you
      may want to ensure the frequency remains the same through the analysis.
   1. The GPU frequency settings should be undone after restarting the device, but after you have done your app profiling,
      you can revert the state of the device, such as the OS build image, back to the way you had it
      initially, as needed.
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
