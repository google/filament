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
import com.google.android.filament.tungsten.properties.PropertyEditor

sealed class PropertyValue {

    open fun applyToMaterialInstance(materialInstance: MaterialInstance, name: String) { }

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

    override fun deserialize(value: Any): Float3 {
        if (value !is List<*> || value.size < 3) return this
        val (x, y, z) = value
        if (x !is Number || y !is Number || z !is Number) return this
        return Float3(x.toFloat(), y.toFloat(), z.toFloat())
    }
}

data class StringValue(val value: String) : PropertyValue() {

    override fun serialize() = value

    override fun deserialize(value: Any): StringValue {
        if (value !is String) return this
        return StringValue(value)
    }
}

enum class PropertyType {
    GRAPH_PROPERTY,
    MATERIAL_PARAMETER
}

data class Property<T : PropertyValue>(
    val name: String,
    val value: T,
    val type: PropertyType = PropertyType.GRAPH_PROPERTY,

    // Construct an appropriate PropertyEditor for this property.
    val editorFactory: (T) -> PropertyEditor
) {

    fun callEditorFactory(): PropertyEditor = this.editorFactory.invoke(value)
}

fun <T : PropertyValue> copyPropertyWithValue(
    p: Property<T>,
    value: PropertyValue
): Property<*> =
        Property(name = p.name, value = value as T, type = p.type, editorFactory = p.editorFactory)
