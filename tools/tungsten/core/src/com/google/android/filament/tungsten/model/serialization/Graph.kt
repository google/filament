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

package com.google.android.filament.tungsten.model.serialization

import com.google.android.filament.tungsten.model.NodeId
import java.awt.Point

data class NodeSchema(
    val type: String,
    val id: NodeId,
    val position: Point? = Point(0, 0),
    val properties: Map<String, Any>?
)

data class SlotSchema(val id: NodeId, val name: String)

data class ConnectionSchema(val from: SlotSchema, val to: SlotSchema) {

    constructor(fromId: NodeId, fromSlot: String, toId: NodeId, toSlot: String):
            this(SlotSchema(fromId, fromSlot), SlotSchema(toId, toSlot))
}

data class GraphSchema(
    val nodes: List<NodeSchema>,
    val rootNode: NodeId?,
    val connections: List<ConnectionSchema>,
    val version: String,
    val editor: Map<String, Any> = emptyMap()
)
