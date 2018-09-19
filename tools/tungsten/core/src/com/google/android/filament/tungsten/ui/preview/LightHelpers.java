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
import com.google.android.filament.IndirectLight;
import com.google.android.filament.Scene;

public final class LightHelpers {

    private LightHelpers() { }

    static IndirectLight addIndirectLight(Engine engine, Scene scene) {
        Ibl i = new Ibl(engine, "ibls/venetian_crossroads_2k");
        IndirectLight ibl =
                new IndirectLight.Builder()
                .irradiance(3, i.getIrradiance())
                .reflections(i.getEnvironmentMap())
                .intensity(30000.0f)
                .build(engine);
        scene.setIndirectLight(ibl);
        scene.setSkybox(i.getSkybox());
        return ibl;
    }
}
