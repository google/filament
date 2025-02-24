# Vulkan Validation Layer Test ICD (VVL_Test_ICD)

This null driver is desigend with the sole intentions to run the test suite against.

The original designed is based off the [MockICD](https://github.com/KhronosGroup/Vulkan-Tools/tree/main/icd).

The 2 main reasons for having a custom null driver instead of using the MockICD are:

1. Reduces one more dependency to build when working on non-released extensions that have a new Vulkan-Headers
2. We have things we do purely for the sake of getting tests to work (ex. Forcing a `VK_ERROR_DEVICE_LOST`)
