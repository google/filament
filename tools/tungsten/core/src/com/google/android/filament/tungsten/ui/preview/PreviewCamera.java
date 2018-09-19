/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.google.android.filament.tungsten.ui.preview;

import com.google.android.filament.Camera;

class PreviewCamera {

    private static final double FOV = 65.0;
    private static final double NEAR_PLANE = 0.1;
    private static final double FAR_PLANE = 200.0;

    private final Camera mCamera;

    public PreviewCamera(Camera camera) {
        mCamera = camera;
    }

    public void setProjection(int width, int height) {
        float displayRatio = (float) width / (float) height;
        Camera.Fov axis;
        if (width < height) {
            axis = Camera.Fov.VERTICAL;
        } else {
            axis = Camera.Fov.HORIZONTAL;
        }
        mCamera.setProjection(FOV, displayRatio, NEAR_PLANE, FAR_PLANE, axis);
    }
}
