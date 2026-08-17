package com.google.android.filament.cppviewer

import android.app.Activity
import android.os.Build
import android.os.Bundle
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Spinner


class MainActivity : Activity() {

    private lateinit var surfaceView: SurfaceView
    private lateinit var choreographer: Choreographer
    private lateinit var frameCallback: Choreographer.FrameCallback
    private var nativeViewer: NativeViewer? = null
    private var displayManager: AndroidDisplayManager? = null
    private var nativeAssetLoader: Long = 0L

    private lateinit var samples: List<String>

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            window.attributes.layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        }

        setContentView(R.layout.activity_main)
        hideSystemUI()
        
        samples = SampleAppDispatcher.getSampleNames().toList()

        surfaceView = findViewById(R.id.surface_view)
        
        val spinner: Spinner = findViewById(R.id.sample_spinner)
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, samples)
        adapter.setDropDownViewResource(R.layout.spinner_dropdown_item)
        spinner.adapter = adapter
        
        spinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>, view: View?, position: Int, id: Long) {
                loadSample(samples[position])
            }
            override fun onNothingSelected(parent: AdapterView<*>) {}
        }

        surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                nativeViewer?.onSurfaceCreated(holder.surface)
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                nativeViewer?.onSurfaceChanged(width, height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                nativeViewer?.onSurfaceDestroyed()
            }
            })
        
        surfaceView.setOnTouchListener { _, event ->
            nativeViewer?.onTouchEvent(event) ?: false
        }
        choreographer = Choreographer.getInstance()
        frameCallback = Choreographer.FrameCallback {
            nativeViewer?.doFrame()
            choreographer.postFrameCallback(frameCallback)
        }
    }
    
    private fun loadSample(name: String) {
        // Cleanup old
        nativeViewer?.onSurfaceDestroyed()
        nativeViewer?.destroy()
        nativeViewer = null
        displayManager?.destroy()
        displayManager = null
        if (nativeAssetLoader != 0L) {
            SampleAppDispatcher.destroyAssetLoader(nativeAssetLoader)
            nativeAssetLoader = 0L
        }
        
        // AndroidDisplayManager handles Android lifecycle events natively
        displayManager = AndroidDisplayManager(surfaceView)
        
        val nativeDm = displayManager?.nativeDm ?: 0L
        if (nativeAssetLoader == 0L) {
            nativeAssetLoader = SampleAppDispatcher.createAssetLoader(assets)
        }
        
        nativeViewer = SampleAppDispatcher.createSampleApp(name, nativeDm, nativeAssetLoader)
        
        // If the surface is already valid, we must trigger onSurfaceCreated so the new app creates its swapchain
        val surface = surfaceView.holder.surface
        if (surface.isValid) {
            nativeViewer?.onSurfaceCreated(surface)
            //nativeViewer?.onSurfaceChanged(surfaceView.width, surfaceView.height)
        }
    }

    override fun onResume() {
        super.onResume()
        choreographer.postFrameCallback(frameCallback)
    }

    override fun onPause() {
        super.onPause()
        choreographer.removeFrameCallback(frameCallback)
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUI()
        }
    }

    @Suppress("DEPRECATION")
    private fun hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.setDecorFitsSystemWindows(false)
            val controller = window.insetsController ?: window.decorView.windowInsetsController
            controller?.let {
                it.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                it.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN
            )
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        nativeViewer?.onSurfaceDestroyed()
        nativeViewer?.destroy()
        nativeViewer = null
        displayManager?.destroy()
        displayManager = null
        if (nativeAssetLoader != 0L) {
            SampleAppDispatcher.destroyAssetLoader(nativeAssetLoader)
            nativeAssetLoader = 0L
        }
    }
}
