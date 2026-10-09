/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.google.android.filament.cppviewer

import android.app.Activity
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.ViewGroup
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.ProgressBar
import android.widget.Spinner


class MainActivity : Activity() {

    companion object {
        private const val TAG = "CppViewer"
        private const val EXTRA_SAMPLE = "sample"
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var loadingIndicator: ProgressBar
    private lateinit var renderThread: SampleRenderThread

    // The generation of the most recent loadSample() request. The loading indicator is hidden
    // only when this request finishes, so results from superseded requests are ignored.
    private var currentGeneration = 0

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
        loadingIndicator = findViewById(R.id.loading_indicator)

        renderThread = SampleRenderThread(
            assets,
            onFirstFrame = { gen -> onSampleReady(gen) },
            onLoadFailed = { gen ->
                Log.e(TAG, "Sample failed to load")
                onSampleReady(gen)
            },
        )

        val spinner: Spinner = findViewById(R.id.sample_spinner)
        // The spinner reserves room for the dropdown arrow at its end. Padding the start of the
        // selected item by the same amount centers the sample name across the whole bar. The
        // spinner's own padding is left alone, because it also positions the dropdown list.
        val arrowSpace = spinner.paddingEnd - spinner.paddingStart
        val adapter = object : ArrayAdapter<String>(
            this, android.R.layout.simple_spinner_item, samples) {
            override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
                val view = super.getView(position, convertView, parent)
                // Recycled views already carry the extra padding.
                if (convertView == null) {
                    view.setPaddingRelative(view.paddingStart + arrowSpace, view.paddingTop,
                        view.paddingEnd, view.paddingBottom)
                }
                return view
            }
        }
        adapter.setDropDownViewResource(R.layout.spinner_dropdown_item)
        spinner.adapter = adapter

        spinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>, view: View?, position: Int, id: Long) {
                loadSample(samples[position])
            }
            override fun onNothingSelected(parent: AdapterView<*>) {}
        }

        // Allows launching a specific sample, for example:
        //   adb shell am start -n com.google.android.filament.cppviewer/.MainActivity \
        //       --es sample suzanne
        intent.getStringExtra(EXTRA_SAMPLE)?.let { name ->
            val index = samples.indexOf(name)
            if (index >= 0) {
                spinner.setSelection(index)
            } else {
                Log.w(TAG, "Unknown sample '$name'; available samples: $samples")
            }
        }

        surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                renderThread.surfaceCreated(holder.surface)
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                renderThread.surfaceChanged(width, height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                // Blocks until the render thread has stopped using the surface.
                renderThread.surfaceDestroyed()
            }
        })

        surfaceView.setOnTouchListener { _, event ->
            renderThread.touchEvent(event.actionMasked, event.x, event.y)
            true
        }
    }

    private fun loadSample(name: String) {
        currentGeneration = renderThread.loadSample(name)
        loadingIndicator.visibility = View.VISIBLE
    }

    private fun onSampleReady(generation: Int) {
        if (!isDestroyed && generation == currentGeneration) {
            loadingIndicator.visibility = View.GONE
        }
    }

    override fun onResume() {
        super.onResume()
        renderThread.resume()
    }

    override fun onPause() {
        super.onPause()
        renderThread.pause()
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
        // Destroys the sample on the render thread without blocking the UI thread.
        renderThread.quit()
    }
}
