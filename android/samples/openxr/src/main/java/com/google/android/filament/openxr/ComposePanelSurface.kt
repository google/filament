package com.google.android.filament.openxr

import android.app.Activity
import android.app.Presentation
import android.content.Context
import android.hardware.display.DisplayManager
import android.hardware.display.VirtualDisplay
import android.os.SystemClock
import android.util.Log
import android.view.MotionEvent
import android.view.Surface
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface as MaterialSurface
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.ViewCompositionStrategy
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.LifecycleRegistry
import androidx.lifecycle.ViewModelStore
import androidx.lifecycle.ViewModelStoreOwner
import androidx.lifecycle.setViewTreeLifecycleOwner
import androidx.lifecycle.setViewTreeViewModelStoreOwner
import androidx.savedstate.SavedStateRegistry
import androidx.savedstate.SavedStateRegistryController
import androidx.savedstate.SavedStateRegistryOwner
import androidx.savedstate.setViewTreeSavedStateRegistryOwner

object ComposePanelSurface {
    private var virtualDisplay: VirtualDisplay? = null
    private var presentation: Presentation? = null
    private var owner: PanelOwner? = null
    private var composeView: ComposeView? = null
    private var touchDownTime = 0L

    @JvmStatic
    fun attach(activity: Activity, surface: Surface, width: Int, height: Int) {
        activity.runOnUiThread {
            release()
            try {
                val displayManager =
                    activity.getSystemService(Context.DISPLAY_SERVICE) as DisplayManager
                virtualDisplay = displayManager.createVirtualDisplay(
                    "FilamentJetpackUi",
                    width,
                    height,
                    activity.resources.displayMetrics.densityDpi,
                    surface,
                    DisplayManager.VIRTUAL_DISPLAY_FLAG_PRESENTATION or
                        DisplayManager.VIRTUAL_DISPLAY_FLAG_OWN_CONTENT_ONLY,
                )

                val panelOwner = PanelOwner()
                val panelPresentation = Presentation(activity, virtualDisplay!!.display)
                val composeView = ComposeView(panelPresentation.context).apply {
                    setViewTreeLifecycleOwner(panelOwner)
                    setViewTreeViewModelStoreOwner(panelOwner)
                    setViewTreeSavedStateRegistryOwner(panelOwner)
                    setViewCompositionStrategy(
                        ViewCompositionStrategy.DisposeOnViewTreeLifecycleDestroyed,
                    )
                    setContent { PanelContent() }
                }
                panelPresentation.setContentView(composeView)
                panelPresentation.show()
                owner = panelOwner
                presentation = panelPresentation
                this.composeView = composeView
                Log.i(TAG, "Jetpack UI attached to ${width}x$height XR surface")
            } catch (throwable: Throwable) {
                Log.e(TAG, "Could not attach Jetpack UI", throwable)
                release()
            }
        }
    }

    @JvmStatic
    fun detach(activity: Activity) {
        activity.runOnUiThread { release() }
    }

    @JvmStatic
    fun injectTouch(activity: Activity, x: Float, y: Float, action: Int) {
        activity.runOnUiThread {
            val target = composeView ?: return@runOnUiThread
            val now = SystemClock.uptimeMillis()
            if (action == MotionEvent.ACTION_DOWN) {
                touchDownTime = now
            }
            val event = MotionEvent.obtain(touchDownTime, now, action, x, y, 0)
            target.dispatchTouchEvent(event)
            event.recycle()
        }
    }

    private fun release() {
        composeView = null
        presentation?.dismiss()
        presentation = null
        owner?.close()
        owner = null
        virtualDisplay?.release()
        virtualDisplay = null
    }

    private const val TAG = "FilamentJetpackUi"
}

@Composable
private fun PanelContent() {
    MaterialTheme(
        colorScheme = darkColorScheme(
            primary = Color(0xFF4DD6B0),
            secondary = Color(0xFFFFC857),
            tertiary = Color(0xFFFF6B5E),
            background = Color(0xFF101614),
            surface = Color(0xFF1A2320),
            onBackground = Color(0xFFF2F5F0),
            onSurface = Color(0xFFF2F5F0),
        ),
    ) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background),
        ) {
            Box(
                modifier = Modifier
                    .fillMaxHeight()
                    .width(10.dp)
                    .background(MaterialTheme.colorScheme.primary),
            )
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 30.dp, vertical = 24.dp),
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    Column {
                        Text(
                            text = "FILAMENT XR",
                            color = MaterialTheme.colorScheme.onBackground,
                            fontFamily = FontFamily.Monospace,
                            fontWeight = FontWeight.Bold,
                            fontSize = 30.sp,
                            letterSpacing = 0.sp,
                        )
                        Text(
                            text = "COMPOSITOR CONTROL",
                            color = Color(0xFF9AA8A2),
                            fontFamily = FontFamily.Monospace,
                            fontSize = 12.sp,
                            letterSpacing = 0.sp,
                        )
                    }
                    var tapCount by remember { mutableIntStateOf(0) }
                    Button(onClick = { tapCount++ }) {
                        Text("TRIGGER $tapCount")
                    }
                }

                Spacer(Modifier.height(20.dp))
                HorizontalDivider(color = Color(0xFF33413C))
                Spacer(Modifier.height(20.dp))

                Row(
                    modifier = Modifier.fillMaxSize(),
                    horizontalArrangement = Arrangement.spacedBy(16.dp),
                ) {
                    StatusTile(
                        modifier = Modifier.weight(1f),
                        label = "RENDERER",
                        value = "VULKAN",
                        accent = MaterialTheme.colorScheme.primary,
                    )
                    StatusTile(
                        modifier = Modifier.weight(1f),
                        label = "STEREO",
                        value = "MULTIVIEW",
                        accent = MaterialTheme.colorScheme.secondary,
                    )
                    StatusTile(
                        modifier = Modifier.weight(1f),
                        label = "LAYER",
                        value = "JETPACK UI",
                        accent = MaterialTheme.colorScheme.tertiary,
                    )
                }
            }
        }
    }
}

@Composable
private fun StatusTile(modifier: Modifier, label: String, value: String, accent: Color) {
    MaterialSurface(
        modifier = modifier.fillMaxHeight(),
        color = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(6.dp),
    ) {
        Column(
            modifier = Modifier.padding(18.dp),
            verticalArrangement = Arrangement.SpaceBetween,
        ) {
            Text(
                text = label,
                color = Color(0xFF9AA8A2),
                fontFamily = FontFamily.Monospace,
                fontSize = 11.sp,
                letterSpacing = 0.sp,
            )
            Column {
                Box(
                    modifier = Modifier
                        .width(36.dp)
                        .height(4.dp)
                        .background(accent),
                )
                Spacer(Modifier.height(12.dp))
                Text(
                    text = value,
                    color = MaterialTheme.colorScheme.onSurface,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    fontSize = 17.sp,
                    letterSpacing = 0.sp,
                )
            }
        }
    }
}

private class PanelOwner : LifecycleOwner, ViewModelStoreOwner, SavedStateRegistryOwner {
    private val lifecycleRegistry = LifecycleRegistry(this)
    private val store = ViewModelStore()
    private val savedStateController = SavedStateRegistryController.create(this)

    override val lifecycle: Lifecycle get() = lifecycleRegistry
    override val viewModelStore: ViewModelStore get() = store
    override val savedStateRegistry: SavedStateRegistry
        get() = savedStateController.savedStateRegistry

    init {
        savedStateController.performAttach()
        savedStateController.performRestore(null)
        lifecycleRegistry.currentState = Lifecycle.State.RESUMED
    }

    fun close() {
        lifecycleRegistry.currentState = Lifecycle.State.DESTROYED
        store.clear()
    }
}
