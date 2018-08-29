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

package com.google.android.filament.tungsten.ui

import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.Slot

/**
 * Create a node view (MaterialNode) for this Node.
 * slotMap maps from a Slot in the graph to its corresponding view, and needs to be updated.
 */
internal fun renderNode(node: Node, slotMap: MutableMap<Slot, NodeSlot>): MaterialNode {

    val nodeView = MaterialNode(node)

    for (slotName in node.inputSlots) {
        val s = node.getInputSlot(slotName)
        val slotView = nodeView.addSlot(s, slotName)
        slotMap[s] = slotView
    }
    for (slotName in node.outputSlots) {
        val s = node.getOutputSlot(slotName)
        val slotView = nodeView.addSlot(s, slotName)
        slotMap[s] = slotView
    }

    nodeView.setBounds(node.x.toInt(), node.y.toInt(), 200, 100)

    return nodeView
}