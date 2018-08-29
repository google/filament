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

package com.google.android.filament.tungsten.model

import com.google.android.filament.MaterialInstance

sealed class PropertyValue {

    abstract fun applyToMaterialInstance(materialInstance: MaterialInstance, name: String)

    /**
     * Serialize this value into a Kotlin List, Map, String, or Number
     */
    abstract fun serialize(): Any

    /**
     * Deserialize into a new PropertyValue
     */
    abstract fun deserialize(value: Any): PropertyValue
}

data class Float3(val x: Float = 0.0f, val y: Float = 0.0f, val z: Float = 0.0f) : PropertyValue() {

    override fun applyToMaterialInstance(materialInstance: MaterialInstance, name: String) {
        materialInstance.setParameter(name, x, y, z)
    }

    override fun serialize() = listOf(x, y, z)

    override fun deserialize(value: Any): PropertyValue {
        if (value !is List<*> || value.size < 3) return this
        val (x, y, z) = value
        if (x !is Number || y !is Number || z !is Number) return this
        return Float3(x.toFloat(), y.toFloat(), z.toFloat())
    }
}

enum class PropertyType {
    GRAPH_PROPERTY,
    MATERIAL_PARAMETER
}

data class Property(
    val name: String,
    val value: PropertyValue,
    val type: PropertyType = PropertyType.GRAPH_PROPERTY
)
