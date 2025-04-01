/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.android.filament.externalimg

import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.SurfaceTexture
import android.hardware.camera2.*
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.util.Size
import android.view.Surface
import androidx.core.content.ContextCompat

import android.Manifest
import android.graphics.ImageFormat
import android.hardware.HardwareBuffer
import android.media.ImageReader
import android.opengl.Matrix
import android.os.Build
import android.os.Looper
import androidx.annotation.RequiresApi

import com.google.android.filament.*

import java.util.concurrent.Semaphore
import java.util.concurrent.TimeUnit

/**
 * Toy class that handles all interaction with the Android camera2 API.
 * Sets the "textureTransform" and "videoTexture" parameters on the given Filament material.
 */
class CameraHelper(val activity: Activity, private val filamentEngine: Engine, private val filamentMaterial: MaterialInstance) {
    private lateinit var cameraId: String
    private lateinit var captureRequest: CaptureRequest

    private val cameraOpenCloseLock = Semaphore(1)
    private var backgroundHandler: Handler? = null
    private var backgroundThread: HandlerThread? = null
    private var cameraDevice: CameraDevice? = null
    private var captureSession: CameraCaptureSession? = null
    private var resolution = Size(640, 480)
    private var filamentTexture: Texture? = null
    private var filamentStream: Stream? = null
    private val imageReader = ImageReader.newInstance(
            resolution.width,
            resolution.height,
            ImageFormat.PRIVATE,
            kImageReaderMaxImages,
            HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE)

    @Suppress("deprecation")
    private val display = if (Build.VERSION.SDK_INT >= 30) {
        Api30Impl.getDisplay(activity)
    } else {
        activity.windowManager.defaultDisplay!!
    }

    @RequiresApi(30)
    class Api30Impl {
        companion object {
            fun getDisplay(context: Context) = context.display!!
        }
    }

    private val cameraCallback = object : CameraDevice.StateCallback() {
        override fun onOpened(cameraDevice: CameraDevice) {
            cameraOpenCloseLock.release()
            this@CameraHelper.cameraDevice = cameraDevice
            createCaptureSession()
        }
        override fun onDisconnected(cameraDevice: CameraDevice) {
            cameraOpenCloseLock.release()
            cameraDevice.close()
            this@CameraHelper.cameraDevice = null
        }
        override fun onError(cameraDevice: CameraDevice, error: Int) {
            onDisconnected(cameraDevice)
            this@CameraHelper.activity.finish()
        }
    }

    /**
     * Fetches the latest image (if any) from ImageReader and passes its HardwareBuffer to Filament.
     */
    fun pushExternalImageToFilament() {
        val stream = filamentStream
        if (stream != null) {
            imageReader.acquireLatestImage()?.also {
                stream.setAcquiredImage(it.hardwareBuffer, Handler(Looper.getMainLooper())) {
                    it.close()
                }
            }
        }
    }

    /**
     * Finds the front-facing Android camera, requests permission, and sets up a listener that will
     * start a capture session as soon as the camera is ready.
     */
    fun openCamera() {
        val manager = activity.getSystemService(Context.CAMERA_SERVICE) as CameraManager
        try {
            for (cameraId in manager.cameraIdList) {
                val characteristics = manager.getCameraCharacteristics(cameraId)
                val cameraDirection = characteristics.get(CameraCharacteristics.LENS_FACING)
                if (cameraDirection != null && cameraDirection == CameraCharacteristics.LENS_FACING_FRONT) {
                    continue
                }

                this.cameraId = cameraId
                Log.i(kLogTag, "Selected camera $cameraId.")

                val map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP) ?: continue
                resolution = map.getOutputSizes(SurfaceTexture::class.java)[0]
                Log.i(kLogTag, "Highest resolution is $resolution.")
            }
        } catch (e: CameraAccessException) {
            Log.e(kLogTag, e.toString())
        } catch (e: NullPointerException) {
            Log.e(kLogTag, "Camera2 API is not supported on this device.")
        }

        val permission = ContextCompat.checkSelfPermission(this.activity, Manifest.permission.CAMERA)
        if (permission != PackageManager.PERMISSION_GRANTED) {
            activity.requestPermissions(arrayOf(Manifest.permission.CAMERA), kRequestCameraPermission)
            return
        }
        if (!cameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
            throw RuntimeException("Time out waiting to lock camera opening.")
        }
        manager.openCamera(cameraId, cameraCallback, backgroundHandler)
    }

    fun onResume() {
        backgroundThread = HandlerThread("CameraBackground").also { it.start() }
        backgroundHandler = Handler(backgroundThread?.looper!!)
    }

    fun onPause() {
        backgroundThread?.quitSafely()
        try {
            backgroundThread?.join()
            backgroundThread = null
            backgroundHandler = null
        } catch (e: InterruptedException) {
            Log.e(kLogTag, e.toString())
        }
    }

    fun onRequestPermissionsResult(requestCode: Int, grantResults: IntArray): Boolean {
        if (requestCode == kRequestCameraPermission) {
            if (grantResults.size != 1 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                Log.e(kLogTag, "Unable to obtain camera position.")
            }
            return true
        }
        return false
    }

    private fun createCaptureSession() {
        filamentStream?.apply { filamentEngine.destroyStream(this) }

        // [Re]create the Filament Stream object that gets bound to the Texture.
        filamentStream = Stream.Builder().build(filamentEngine)

        // Create the Filament Texture object if we haven't done so already.
        if (filamentTexture == null) {
            filamentTexture = Texture.Builder()
                    .sampler(Texture.Sampler.SAMPLER_EXTERNAL)
                    .format(Texture.InternalFormat.RGB8)
                    .build(filamentEngine)
        }

        // We are texturing a front-facing square shape so we need to generate a matrix that transforms (u, v, 0, 1)
        // into a new UV coordinate according to the screen rotation and the aspect ratio of the camera image.
        val aspectRatio = resolution.width.toFloat() / resolution.height.toFloat()
        val textureTransform = FloatArray(16)
        Matrix.setIdentityM(textureTransform, 0)
        when (display.rotation) {
            Surface.ROTATION_0 -> {
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.rotateM(textureTransform, 0, 90.0f, 0.0f, 0.0f, 1.0f)
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.scaleM(textureTransform, 0, -1.0f, 1.0f / aspectRatio, 1.0f)
            }
            Surface.ROTATION_90 -> {
                Matrix.translateM(textureTransform, 0, 1.0f, 1.0f, 0.0f)
                Matrix.rotateM(textureTransform, 0, 180.0f, 0.0f, 0.0f, 1.0f)
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.scaleM(textureTransform, 0, -1.0f / aspectRatio, 1.0f, 1.0f)
            }
            Surface.ROTATION_270 -> {
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.scaleM(textureTransform, 0, -1.0f / aspectRatio, 1.0f, 1.0f)
            }
        }

        // Connect the Stream to the Texture and the Texture to the MaterialInstance.
        val sampler = TextureSampler(TextureSampler.MinFilter.LINEAR, TextureSampler.MagFilter.LINEAR, TextureSampler.WrapMode.CLAMP_TO_EDGE)
        filamentTexture!!.setExternalStream(filamentEngine, filamentStream!!)
        filamentMaterial.setParameter("videoTexture", filamentTexture!!, sampler)
        filamentMaterial.setParameter("textureTransform", MaterialInstance.FloatElement.MAT4, textureTransform, 0, 1)

        // Start the capture session. You could also use TEMPLATE_PREVIEW here.
        val captureRequestBuilder = cameraDevice!!.createCaptureRequest(CameraDevice.TEMPLATE_RECORD)
        captureRequestBuilder.addTarget(imageReader.surface)

        cameraDevice?.createCaptureSession(listOf(imageReader.surface),
                object : CameraCaptureSession.StateCallback() {
                    override fun onConfigured(cameraCaptureSession: CameraCaptureSession) {
                        if (cameraDevice == null) return
                        captureSession = cameraCaptureSession
                        captureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                        captureRequest = captureRequestBuilder.build()
                        captureSession!!.setRepeatingRequest(captureRequest, null, backgroundHandler)
                        Log.i(kLogTag, "Created CaptureRequest.")
                    }
                    override fun onConfigureFailed(session: CameraCaptureSession) {
                        Log.e(kLogTag, "onConfigureFailed")
                    }
                }, null)
    }

    companion object {
        private const val kLogTag = "CameraHelper"
        private const val kRequestCameraPermission = 1
        private const val kImageReaderMaxImages = 7
    }

}
