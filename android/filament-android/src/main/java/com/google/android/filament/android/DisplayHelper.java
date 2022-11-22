/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.android;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.os.Handler;
import android.view.Display;

import com.google.android.filament.Renderer;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/**
 * DisplayHelper is here to help managing a Display, for instance being notified when its
 * resolution or refresh rate changes.
 */
public class DisplayHelper {

    private Handler mHandler = null;
    private DisplayManager mDisplayManager;
    private Display mDisplay;
    private Renderer mRenderer;
    private DisplayManager.DisplayListener mListener;

    /**
     * Creates a DisplayHelper which helps managing a {@link Display}.
     *
     * The {@link Display} to manage is specified with {@link #attach}
     *
     * @param context a {@link Context} to used to retrieve the {@link DisplayManager}
     */
    public DisplayHelper(@NonNull Context context) {
        mDisplayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
    }

    /**
     * Creates a DisplayHelper which helps manage a {@link Display} and provides a Handler
     * where callbacks can execute filament code. Use this method if filament is executing
     * on another thread.
     *
     * @param context a {@link Context} to used to retrieve teh {@link DisplayManager}
     * @param handler a {@link Handler} used to run callbacks accessing filament
     */
    public DisplayHelper(@NonNull Context context, @NonNull Handler handler) {
        this(context);
        mHandler = handler;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            // just for safety
            detach();
        } finally {
            super.finalize();
        }
    }

    /**
     * Sets the filament {@link Renderer} associated to the {@link Display}, from this point
     * on, {@link Renderer.DisplayInfo} will be automatically updated when the {@link Display}
     * properties change.
     *
     * This is typically called from {@link UiHelper.RendererCallback#onNativeWindowChanged}.
     *
     * @param renderer a filament {@link Renderer} instance
     * @param display a {@link Display} to be associated with the {@link Renderer}
     */
    public void attach(@NonNull Renderer renderer, @NonNull Display display) {
        if (renderer == mRenderer && display == mDisplay) {
            return;
        }
        mRenderer = renderer;
        mDisplay = display;
        mListener = new DisplayManager.DisplayListener() {
            @Override
            public void onDisplayAdded(int displayId) {
            }
            @Override
            public void onDisplayRemoved(int displayId) {
            }
            @Override
            public void onDisplayChanged(int displayId) {
                if (displayId == display.getDisplayId()) {
                    updateDisplayInfo();
                }
            }
        };
        mDisplayManager.registerDisplayListener(mListener, mHandler);

        // always invoke the callback when it's registered
        if (mHandler != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    updateDisplayInfo();
                }
            });
        } else {
            updateDisplayInfo();
        }
    }

    /**
     * Disconnect the previously set {@link Renderer} from {@link Display}
     * This is typically called from {@link UiHelper.RendererCallback#onDetachedFromSurface}.
     */
    public void detach() {
        if (mListener != null) {
            mDisplayManager.unregisterDisplayListener(mListener);
            mListener = null;
            mDisplay = null;
            mRenderer = null;
        }
    }

    private void updateDisplayInfo() {
        mRenderer.setDisplayInfo(
                DisplayHelper.getDisplayInfo(mDisplay, mRenderer.getDisplayInfo()));
    }

    /**
     * Returns the {@link Display} currently monitored
     * @return the {@link Display} set in {@link #attach} or null
     */
    public Display getDisplay() {
        return mDisplay;
    }

    /**
     * Populate a {@link Renderer.DisplayInfo} with properties from the given {@link Display}
     *
     * @param display   {@link Display} to get {@link Renderer.DisplayInfo} from
     * @param info      an instance of {@link Renderer.DisplayInfo} or null
     * @return          an populated instance of {@link Renderer.DisplayInfo}
     */
    @NonNull
    public static Renderer.DisplayInfo getDisplayInfo(@NonNull Display display, @Nullable Renderer.DisplayInfo info) {
        if (info == null) {
            info = new Renderer.DisplayInfo();
        }
        info.refreshRate = DisplayHelper.getRefreshRate(display);
        return info;
    }

    /**
     * @return the {@link Display} application vsync offset 0 if not supported
     * @see Display#getAppVsyncOffsetNanos
     */
    public static long getAppVsyncOffsetNanos(@NonNull Display display) {
        if (Build.VERSION.SDK_INT >= 29) {
            return display.getAppVsyncOffsetNanos();
        }
        return 0;
    }

    /**
     * @return the {@link Display} presentation deadline before the h/w vsync event in nanoseconds
     * @see Display#getPresentationDeadlineNanos
     */
    public static long getPresentationDeadlineNanos(@NonNull Display display) {
        if (Build.VERSION.SDK_INT >= 29) {
            return display.getPresentationDeadlineNanos();
        }
        // not supported, pick something reasonable
        return 11_600_000;
    }

    /**
     * @return the {@link Display} refresh rate in Hz
     * @see Display#getRefreshRate
     */
    public static float getRefreshRate(@NonNull Display display) {
        return display.getRefreshRate();
    }

    /**
     * Returns a {@link Display}'s refresh period in nanoseconds
     * @param display the {@link Display} to get the refresh period from
     * @return the {@link Display} refresh period in nanoseconds
     */
    public static long getRefreshPeriodNanos(@NonNull Display display) {
        return (long) (1_000_000_000.0 / display.getRefreshRate());
    }
}
