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

package com.google.android.filament.tungsten.compiler

import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.Slot

data class CompiledGraph(
    // The Filament material definition that is fed into matc
    val materialDefinition: String,

    // Maps a Node's property to the associated material parameter it controls
    val parameterMap: Map<Node.PropertyHandle, Parameter>,

    // Maps a Slot to its corresponding Expression
    val expressionMap: Map<Slot, Expression>,

    /**
     * Nodes can change during compilation. This maps from Nodes in the graph pre-compilation to
     * equivalent nodes post-compilation.
     */
    val oldToNewNodeMap: Map<Node, Node>
)
