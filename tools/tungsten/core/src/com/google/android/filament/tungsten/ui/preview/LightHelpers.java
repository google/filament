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

import com.google.android.filament.Engine;
import com.google.android.filament.EntityManager;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.LightManager;
import com.google.android.filament.Scene;

public final class LightHelpers {

    private LightHelpers() { }

    private static final float[] DEBUG_IRRADIANCE = {
            0.7545545f,
            0.74854296f,
            0.7909215f,
            -0.083856545f,
            0.0925335f,
            0.32276466f,
            0.3081527f,
            0.36679634f,
            0.46669817f,
            -0.18888493f,
            -0.27740255f,
            -0.3778442f,
            -0.25278875f,
            -0.3160564f,
            -0.39614528f,
            0.0713582f,
            0.15978426f,
            0.29059005f,
            -0.031043747f,
            -0.031144021f,
            -0.031046612f,
            -0.16099837f,
            -0.2036487f,
            -0.24664281f,
            0.045710605f,
            0.048120886f,
            0.046324715f
    };

    static int addSun(Engine engine, Scene scene) {
        int light = EntityManager.get().create();
        LightManager.Builder lightBuilder = new LightManager.Builder(LightManager.Type.SUN);
        lightBuilder.build(engine, light);
        scene.addEntity(light);
        return light;
    }

    static int addPointLight(Engine engine, Scene scene) {
        int light = EntityManager.get().create();
        LightManager.Builder lightBuilder = new LightManager.Builder(LightManager.Type.POINT)
                .position(0.0f, 0.0f, 2.0f)
                .direction(0.0f, 0.0f, -1.0f)
                .intensity(440000.0f)
                .castShadows(false)
                .falloff(10.0f)
                .color(1.0f, 1.0f, 1.0f);
        lightBuilder.build(engine, light);
        scene.addEntity(light);
        return light;
    }

    static IndirectLight addIndirectLight(Engine engine, Scene scene) {
        IndirectLight ibl =
                new IndirectLight.Builder()
                .irradiance(3, DEBUG_IRRADIANCE)
                .intensity(30000.0f)
                .build(engine);
        scene.setIndirectLight(ibl);
        return ibl;
    }
}
