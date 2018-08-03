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

import com.google.gson.GsonBuilder
import com.google.gson.JsonArray
import com.google.gson.JsonDeserializer
import com.google.gson.JsonSerializer

interface ISerializer {

    fun serialize(graph: Graph): String
}

interface IDeserializer {

    fun deserialize(data: String): Graph
}

// Serialize Slot as an array: [5, "foobar"]
val slotSerializer: JsonSerializer<Slot> = JsonSerializer { src, _, _ ->
    val pairArray = JsonArray()
    pairArray.add(src.id)
    pairArray.add(src.name)
    pairArray
}

// Deserialize Slot from an array: [5, "foobar"]
val slotDeserializer: JsonDeserializer<Slot> = JsonDeserializer { element, _, _ ->
    val o = element.asJsonArray
    val id = o.get(0).asInt
    val slot = o.get(1).asString
    Slot(id, slot)
}

class JsonSerializer : ISerializer {

    override fun serialize(graph: Graph): String {
        val gsonBuilder = GsonBuilder()
        gsonBuilder.registerTypeAdapter(Slot::class.java, slotSerializer)
        val gson = gsonBuilder.create()
        return gson.toJson(graph)
    }
}

class JsonDeserializer : IDeserializer {

    override fun deserialize(data: String): Graph {
        val gsonBuilder = GsonBuilder()
        gsonBuilder.registerTypeAdapter(Slot::class.java, slotDeserializer)
        val gson = gsonBuilder.create()
        return gson.fromJson(data, Graph::class.java)
    }
}
