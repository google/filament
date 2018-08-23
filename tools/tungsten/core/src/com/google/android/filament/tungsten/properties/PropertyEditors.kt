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

package com.google.android.filament.tungsten.properties

import com.google.android.filament.tungsten.model.Float3
import com.google.android.filament.tungsten.model.PropertyValue
import java.awt.Color
import javax.swing.JColorChooser
import javax.swing.JComponent
import kotlin.math.roundToInt

sealed class PropertyEditor {

    abstract val component: JComponent
    var valueChanged: (newValue: Float3) -> Unit = { }

    abstract fun setValue(v: PropertyValue)
}

internal class ColorChooser(value: Float3) : PropertyEditor() {

    override val component: JColorChooser

    override fun setValue(v: PropertyValue) {
        val value = v as Float3
        component.setColor((value.x * 255.0f).roundToInt(), (value.y * 255.0f).roundToInt(),
                (value.z * 255.0f).roundToInt())
    }

    init {
        val initialColor = Color(value.x, value.y, value.z)
        component = JColorChooser(initialColor)
        component.selectionModel.addChangeListener {
            val newValue = value.copy(
                    x = component.color.red / 255.0f,
                    y = component.color.green / 255.0f,
                    z = component.color.blue / 255.0f)
            valueChanged(newValue)
        }
    }
}