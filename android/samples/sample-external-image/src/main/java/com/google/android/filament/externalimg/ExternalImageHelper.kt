import android.graphics.*
import android.hardware.HardwareBuffer
import android.media.Image
import android.media.ImageReader
import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.view.Surface
import androidx.annotation.RequiresApi
import java.util.concurrent.CompletableFuture
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeoutException

@RequiresApi(Build.VERSION_CODES.O)
object CanvasToHardwareBufferUtil {
    private const val TAG = "CanvasToHardwareBufferKt"
    private const val IMAGE_READER_TIMEOUT_MS = 3000L // Timeout for waiting buffer

    fun drawToHardwareBuffer(
        width: Int,
        height: Int,
    ): HardwareBuffer? {
        if (width <= 0 || height <= 0) {
            Log.e(TAG, "Invalid dimensions: width=$width, height=$height")
            return null
        }

        var handlerThread: HandlerThread? = null
        var imageReader: ImageReader? = null
        var surface: Surface? = null // Keep track for logging/debugging if needed
        // Use var as it's assigned within the try block after future completion
        var receivedHardwareBuffer: HardwareBuffer? = null

        try {
            // 1. Setup HandlerThread for ImageReader callbacks
            handlerThread = HandlerThread("ImageReaderThreadKt").apply { start() }
            val imageReaderHandler = Handler(handlerThread.looper)

            // 2. Use CompletableFuture to wait for the buffer from the listener
            val bufferFuture = CompletableFuture<HardwareBuffer>()

            // 3. Create ImageReader
            val usageFlags =
                HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE or
                    HardwareBuffer.USAGE_GPU_COLOR_OUTPUT or
                    HardwareBuffer.USAGE_CPU_READ_RARELY // Adjust as needed

            imageReader =
                ImageReader.newInstance(width, height, PixelFormat.RGBA_8888, 1, usageFlags)

            // 4. Set Listener to capture the buffer
            imageReader.setOnImageAvailableListener({ reader ->
                var image: Image? = null
                var hardwareBuffer: HardwareBuffer? = null
                try {
                    // Use `use` block for automatic image.close()
                    image = reader.acquireLatestImage()
                    if (image == null) {
                        Log.w(TAG, "ImageReader listener fired but no image available.")
                        // Complete exceptionally if buffer wasn't already completed.
                        bufferFuture.completeExceptionally(
                            RuntimeException("ImageReader listener fired but no image available"),
                        )
                        return@setOnImageAvailableListener
                    }

                    hardwareBuffer = image.hardwareBuffer
                    if (hardwareBuffer != null) {
                        // IMPORTANT: Don't close the HardwareBuffer here!
                        // Transfer ownership via the CompletableFuture.
                        if (!bufferFuture.isDone) { // Avoid completing more than once
                            bufferFuture.complete(hardwareBuffer)
                        } else {
                            // Future was already completed (maybe exceptionally), close this buffer
                            Log.w(TAG, "Future already done, closing redundant HardwareBuffer")
                            hardwareBuffer.close()
                        }
                    } else {
                        Log.e(TAG, "Failed to get HardwareBuffer from Image.")
                        if (!bufferFuture.isDone) {
                            bufferFuture.completeExceptionally(
                                RuntimeException("Failed to get HardwareBuffer from Image"),
                            )
                        }
                    }
                } catch (e: Exception) {
                    Log.e(TAG, "Error in ImageReader listener", e)
                    if (!bufferFuture.isDone) {
                        bufferFuture.completeExceptionally(e) // Propagate error
                    }
                    // If we got the buffer but failed elsewhere, ensure it's closed
                    hardwareBuffer?.takeUnless { it.isClosed }?.close()
                } finally {
                    // image?.close() // Handled by acquiring reader itself or image.use{} if used
                    image?.close() // Close image if not using `use` or if error before `use` finishes
                }
            }, imageReaderHandler)

            // 5. Get the Surface to draw onto
            surface =
                imageReader.surface
                    ?: throw RuntimeException("Failed to get Surface from ImageReader")

            // 6. Lock Canvas and Draw
            val canvas: Canvas? = surface.lockHardwareCanvas() // Use hardware accelerated canvas
            if (canvas != null) {
                try {
                    // --- Your Drawing Code Here ---
                    val paint =
                        Paint().apply {
                            isAntiAlias = true // Good practice
                        }

                    // Blue background
                    paint.color = Color.BLUE
                    canvas.drawRect(0f, 0f, width.toFloat(), height.toFloat(), paint)

                    // White text
                    paint.color = Color.WHITE
                    paint.textSize = 40f
                    paint.textAlign = Paint.Align.CENTER
                    canvas.drawText(
                        "Hello HardwareBuffer! (Kotlin)",
                        width / 2f,
                        height / 2f,
                        paint,
                    )
                    // --- End Drawing Code ---
                } finally {
                    // 7. Unlock Canvas and Post
                    surface.unlockCanvasAndPost(canvas)
                }
            } else {
                throw RuntimeException("Failed to lock Hardware Canvas")
            }

            // 8. Wait for the listener to provide the HardwareBuffer
            try {
                // Wait for the buffer; this blocks the current thread.
                receivedHardwareBuffer =
                    bufferFuture.get(IMAGE_READER_TIMEOUT_MS, TimeUnit.MILLISECONDS)
                // Ownership of receivedHardwareBuffer is now transferred to the caller
            } catch (timeout: TimeoutException) {
                Log.e(TAG, "Timeout waiting for HardwareBuffer from ImageReader listener")
                bufferFuture.cancel(true) // Attempt to cancel listener processing
                throw timeout // Re-throw
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to draw to HardwareBuffer", e)
            // Ensure buffer is closed if acquired but an error occurred before returning it
            receivedHardwareBuffer?.takeUnless { it.isClosed }?.close()
            return null // Indicate failure
        } finally {
            // 9. Cleanup
            try {
                imageReader?.close() // Also releases the Surface implicitly
            } catch (e: Exception) {
                Log.e(TAG, "Error closing ImageReader", e)
            }
            try {
                handlerThread?.quitSafely()
            } catch (e: Exception) {
                Log.e(TAG, "Error quitting HandlerThread", e)
            }
            // Note: Do NOT close receivedHardwareBuffer here if returning successfully.
            // The caller is responsible for closing the returned buffer.
        }

        // Return the buffer; caller MUST close it.
        return receivedHardwareBuffer
    }

    // --- Example Usage (must be called from appropriate context/thread, like a coroutine) ---
    /*
    @RequiresApi(Build.VERSION_CODES.O)
    suspend fun exampleUsage() = withContext(Dispatchers.IO) { // Run blocking code off main thread
        val myBuffer: HardwareBuffer? = CanvasToHardwareBufferUtil.drawToHardwareBuffer(640, 480)

        // Use the 'use' extension function for automatic closing
        myBuffer?.use { buffer ->
            // ... Use the buffer (e.g., create an EGLImage, pass to Vulkan, etc.) ...
            Log.d(TAG, "Successfully created HardwareBuffer: $buffer. Now using it...")
            // buffer.close() // 'use' handles this automatically

        } ?: run {
             Log.e(TAG, "Failed to create HardwareBuffer.")
        }

         Log.d(TAG,"HardwareBuffer processing finished (buffer closed if obtained).")
    }
     */
}
