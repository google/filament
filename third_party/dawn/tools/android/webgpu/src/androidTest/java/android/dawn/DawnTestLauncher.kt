package android.dawn

import android.dawn.CallbackMode.Companion.WaitAnyOnly
import android.dawn.helper.DawnException
import android.dawn.helper.Util
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking

class DeviceLostException(val device: Device, val reason: DeviceLostReason, message: String) :
    Exception(message)

class UncapturedErrorException(val device: Device, val type: ErrorType, message: String) : Exception(message)

fun dawnTestLauncher(
    requiredFeatures: Array<FeatureName> = arrayOf(),
    callback: suspend (device: Device) -> Unit
) {
    Util  // Hack to force library initialization.

    val instance = createInstance()

    runBlocking {
        val eventProcessor =
            launch {
                while (true) {
                    delay(200)
                    instance.processEvents()
                }
            }

        val adapter =
            instance.requestAdapter().adapter ?: throw DawnException("No adapter available")

        val device = adapter.requestDevice(
            DeviceDescriptor(
                requiredFeatures = requiredFeatures,
                deviceLostCallbackInfo2 = DeviceLostCallbackInfo2(
                    callback = DeviceLostCallback2
                    { device, reason, message ->
                        throw DeviceLostException(device, reason, message)
                    },
                    mode = WaitAnyOnly
                ),
                uncapturedErrorCallbackInfo2 = UncapturedErrorCallbackInfo2 { device, type, message ->
                    throw UncapturedErrorException(device, type, message)
                })
        ).device ?: throw DawnException("No device available")

        callback(device)

        device.close()
        device.destroy()
        adapter.close()

        eventProcessor.cancel()
        runBlocking {
            eventProcessor.join()
        }
        var caughtDeviceLostException = false;
        try {
            instance.close()
        } catch (ignored: DeviceLostException) {
            // For some reason we receive a device lost callback even though the only device has
            // been closed. b/381416258
            caughtDeviceLostException = true;
        }
        assert(caughtDeviceLostException) {
            "When this assert stops passing, it indicates that the DeviceLostException we're " +
                    "catching above is no longer being erroneously thrown, so we can safely " +
                    "remove the try/catch and treat the DLE as a genuine error."
        }
    }
}
