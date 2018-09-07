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

import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.NodeId
import com.google.android.filament.tungsten.model.copyPropertyWithValue
import javax.swing.JPanel

class PropertiesPanel : JPanel() {

    var presenter: IPropertiesPresenter? = null

    private val editorCache: MutableMap<NodeId, List<PropertyEditor>> = mutableMapOf()

    fun showNone() {
        removeAll()
        repaint()
        revalidate()
    }

    fun showPropertiesForNode(node: Node) {
        val p = presenter ?: return
        // TODO: every time a node is selected, we recreate the property panel for its
        // properties. We could probably cache this somewhere and show / hide it to avoid
        // re-adding components each time.

        removeAll()
        createEditorsForNode(node)
        val editors = editorCache[node.id] ?: return
        for ((property, editor) in node.properties.zip(editors)) {
            editor.setValue(property.value)
            editor.valueChanged = { newValue ->
                p.propertyChanged(node.getPropertyHandle(property.name),
                        copyPropertyWithValue(property, newValue))
            }
            add(editor.component)
        }
        repaint()
        revalidate()
    }

    /**
     * editorCache caches a list of property editors for each node in the graph. This is done so
     * property editors don't have to be re-allocated each time showPropertiesForNode is called.
     */
    private fun createEditorsForNode(node: Node) {
        editorCache.computeIfAbsent(node.id) { _ ->
            node.properties.map { it.callEditorFactory() }
        }
    }
}
