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

package com.google.android.filament.tungsten.properties;

import javax.swing.JColorChooser;
import javax.swing.JComponent;

public class Float3Property extends NodeProperty {

    private JColorChooser mColorChooser;

    public Float3Property(float x, float y, float z) {
        mColorChooser = new JColorChooser();
        mColorChooser.getSelectionModel().addChangeListener(e -> {
            mPresenter.propertyChanged();
        });
    }

    @Override
    public JComponent getComponent() {
        return mColorChooser;
    }

    public float getX() {
        return mColorChooser.getColor().getRed() / 255.f;
    }

    public float getY() {
        return mColorChooser.getColor().getGreen() / 255.f;
    }

    public float getZ() {
        return mColorChooser.getColor().getBlue() / 255.f;
    }
}
