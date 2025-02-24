# Android Mock

When dealing with AndroidHardwareBuffers (`VK_ANDROID_external_memory_android_hardware_buffer`) there are API calls that are only found when deploying on Android.

In order to easily and properly do some testing for extensions that don't have devices yet (`VK_ANDROID_external_format_resolve`) we need a way to have a MockICD driver.

Instead of trying to deploy a MockICD driver for android, we are leaving the MockICD alone and instead just mocking the bare minimum AHB calls in order to run the tests locally.

This allows both developers and GitHub CI to test the AHB logic against something instead of having to rely on a phyiscal Android device.

This is not a **replacement** for the Android devices as the same way MockICD is not a replacement for phyiscal hardware