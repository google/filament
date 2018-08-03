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

import java.awt.Point

data class Node(val type: String, val id: Int, val position: Point? = Point(0, 0))

data class Slot(val id: Int, val name: String)

data class Connection(val from: Slot, val to: Slot) {

    constructor(fromId: Int, fromSlot: String, toId: Int, toSlot: String):
            this(Slot(fromId, fromSlot), Slot(toId, toSlot))
}

data class Graph(
    val nodes: List<Node>,
    val connections: List<Connection>,
    val version: String,
    val editor: Map<String, Any> = emptyMap()
)
