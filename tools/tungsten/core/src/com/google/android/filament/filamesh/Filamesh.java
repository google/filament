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

package com.google.android.filament.filamesh;

import com.google.android.filament.IndexBuffer;
import com.google.android.filament.VertexBuffer;

public class Filamesh {
    private final String mName;
    private final VertexBuffer mVertexBuffer;
    private final IndexBuffer mIndexBuffer;

    Filamesh(String name, VertexBuffer vertexBuffer, IndexBuffer indexBuffer) {
        mName = name;
        mVertexBuffer = vertexBuffer;
        mIndexBuffer = indexBuffer;
    }

    String getName() {
        return mName;
    }

    public VertexBuffer getVertexBuffer() {
        return mVertexBuffer;
    }

    public IndexBuffer getIndexBuffer() {
        return mIndexBuffer;
    }
}
