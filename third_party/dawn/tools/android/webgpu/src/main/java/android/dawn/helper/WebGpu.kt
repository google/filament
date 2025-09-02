package android.dawn.helper

import android.dawn.Adapter
import android.dawn.BackendType
import android.dawn.CallbackMode.Companion.AllowSpontaneous
import android.dawn.Device
import android.dawn.DeviceDescriptor
import android.dawn.DeviceLostCallback
import android.dawn.DeviceLostCallbackInfo
import android.dawn.DeviceLostReason
import android.dawn.ErrorType
import android.dawn.FeatureName
import android.dawn.Instance
import android.dawn.InstanceDescriptor
import android.dawn.RequestAdapterOptions
import android.dawn.RequestAdapterStatus
import android.dawn.RequestDeviceStatus
import android.dawn.SurfaceDescriptor
import android.dawn.SurfaceSourceAndroidNativeWindow
import android.dawn.UncapturedErrorCallbackInfo
import android.dawn.createInstance
import android.dawn.helper.Util.windowFromSurface
import android.dawn.requestAdapter
import android.dawn.requestDevice
import android.os.Handler
import android.os.Looper
import android.view.Surface

public class DeviceLostException(
    public val device: Device, public val reason: DeviceLostReason, message: String
) : Exception(message)

public class UncapturedErrorException(
    public val device: Device, public val type: ErrorType, message: String
) : Exception(message)

private const val POLLING_DELAY_MS = 100L

public abstract class WebGpu : AutoCloseable {
    public abstract val instance: Instance
    public abstract val webgpuSurface: android.dawn.Surface
    public abstract val device: Device
}

public suspend fun createWebGpu(
    surface: Surface? = null,
    instanceDescriptor: InstanceDescriptor = InstanceDescriptor(),
    requestAdapterOptions: RequestAdapterOptions = RequestAdapterOptions(),
    requiredFeatures: Array<FeatureName> = arrayOf()
): WebGpu {
    initLibrary()

    val instance = createInstance(instanceDescriptor)
    val webgpuSurface =
        surface?.let {
            instance.createSurface(
                SurfaceDescriptor(
                    surfaceSourceAndroidNativeWindow =
                        SurfaceSourceAndroidNativeWindow(windowFromSurface(it))
                )
            )
        }

    val adapter = requestAdapter(instance, requestAdapterOptions)
    val device = requestDevice(adapter, requiredFeatures)

    val isClosing = false
    // Long-running event poller for async methods. Can be removed when
    // https://issues.chromium.org/issues/323983633 is fixed.
    val handler = Handler(Looper.getMainLooper())
    fun nextProcess() {
        handler.postDelayed({
            if (isClosing) {
                return@postDelayed
            }
            instance.processEvents()
            nextProcess()
        }, POLLING_DELAY_MS)
    }
    nextProcess()

    return object : WebGpu() {
        override val instance = instance
        override val webgpuSurface
            get() = checkNotNull(webgpuSurface)
        override val device = device

        override fun close() {
            device.close()
            device.destroy();
//          webgpuSurface.close()  // TODO(b/428866400): Remove when fixed
//          instance.close()  // TODO(b/428866400): Remove when fixed
        }
    }
}

private suspend fun requestAdapter(
    instance: Instance,
    options: RequestAdapterOptions = RequestAdapterOptions(backendType = BackendType.Vulkan),
): Adapter {
    val (status, adapter, message) = instance.requestAdapter(options)
    check(status == RequestAdapterStatus.Success && adapter != null) {
        message.ifEmpty { "Error requesting the adapter: $status" }
    }
    return adapter
}

private suspend fun requestDevice(adapter: Adapter, requiredFeatures: Array<FeatureName>): Device {
    val (status, device, message) =
        adapter.requestDevice(
            DeviceDescriptor(
                requiredFeatures = requiredFeatures,
                deviceLostCallbackInfo =
                    DeviceLostCallbackInfo(
                        callback =
                            DeviceLostCallback { device, reason, message ->
                                throw DeviceLostException(device, reason, message)
                            },
                        mode = AllowSpontaneous,
                    ),
                uncapturedErrorCallbackInfo =
                    UncapturedErrorCallbackInfo { device, type, message ->
                        throw UncapturedErrorException(device, type, message)
                    },
            )
        )
    check(status == RequestDeviceStatus.Success && device != null) {
        message.ifEmpty { "Error requesting the device: $status" }
    }
    return device
}

/** Initializes the native library. This method should be called before making and WebGPU calls. */
public fun initLibrary() {
    System.loadLibrary("webgpu_c_bundled")
}
