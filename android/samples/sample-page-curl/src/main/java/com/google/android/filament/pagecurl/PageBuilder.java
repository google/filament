/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.pagecurl;

import com.google.android.filament.Engine;
import com.google.android.filament.EntityManager;
import com.google.android.filament.IndexBuffer;
import com.google.android.filament.RenderableManager;
import com.google.android.filament.RenderableManager.PrimitiveType;
import com.google.android.filament.VertexBuffer;
import com.google.android.filament.VertexBuffer.VertexAttribute;

import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

@SuppressWarnings({"WeakerAccess", "unused"})
public class PageBuilder {
    private PageMaterials mMaterials;
    private final int[] mMeshResolution = new int[] { 20, 20 };

    public PageBuilder materials(PageMaterials materials) {
        this.mMaterials = materials;
        return this;
    }

    public PageBuilder resolution(int columnCount, int rowCount) {
        mMeshResolution[0] = columnCount;
        mMeshResolution[1] = rowCount;
        return this;
    }

    Page build(Engine engine, EntityManager entityManager) {
        final int numColumns = mMeshResolution[0];
        final int numRows = mMeshResolution[1];
        final int numCells = numColumns * numRows;
        final int numIndices = numCells * 6;
        final int numVertices = (numColumns + 1) * (numRows + 1);

        if (mMaterials == null || numVertices >= Short.MAX_VALUE) {
            return null;
        }

        FloatBuffer positions = FloatBuffer.allocate(numVertices * 3);
        FloatBuffer uvs = FloatBuffer.allocate(numVertices * 2);
        FloatBuffer tangents = FloatBuffer.allocate(numVertices * 4);
        ShortBuffer indices = ShortBuffer.allocate(numIndices);

        Page page = new Page();

        page.indexBuffer = new IndexBuffer.Builder()
                .indexCount(numIndices)
                .bufferType(IndexBuffer.Builder.IndexType.USHORT)
                .build(engine);

        final int vertsPerRow = numColumns + 1;
        for (int row = 0; row < numRows; row++) {
            for (int col = 0; col < numColumns; col++) {
                final int a = col + row * vertsPerRow;
                final int b = (col + 1) + row * vertsPerRow;
                final int c = col + (row + 1) * vertsPerRow;
                final int d = (col + 1) + (row + 1) * vertsPerRow;
                indices.put((short)a);
                indices.put((short)b);
                indices.put((short)d);
                indices.put((short)d);
                indices.put((short)c);
                indices.put((short)a);
            }
        }
        indices.rewind();
        page.indexBuffer.setBuffer(engine, indices);

        page.vertexBuffer = new VertexBuffer.Builder()
                .bufferCount(3)
                .vertexCount(numVertices)
                .attribute(VertexAttribute.POSITION, 0, VertexBuffer.AttributeType.FLOAT3)
                .attribute(VertexAttribute.UV0, 1, VertexBuffer.AttributeType.FLOAT2)
                .attribute(VertexAttribute.TANGENTS, 2, VertexBuffer.AttributeType.FLOAT4)
                .build(engine);

        for (int row = 0; row <= numRows; row++) {
            for (int col = 0; col <= numColumns; col++) {
                uvs.put((float) col / numColumns);
                uvs.put((float) row / numRows);
            }
        }

        uvs.flip();

        page.vertexBuffer.setBufferAt(engine, 1, uvs);

        page.positions = positions;
        page.uvs = uvs;
        page.tangents = tangents;
        page.material = mMaterials.createInstance();
        page.renderable = entityManager.create();

        new RenderableManager.Builder(1)
                .culling(false)
                .material(0, page.material)
                .geometry(0, PrimitiveType.TRIANGLES, page.vertexBuffer, page.indexBuffer)
                .castShadows(false)
                .receiveShadows(false)
                .build(engine, page.renderable);

        page.updateVertices(engine, 0f, 0f, Page.CurlStyle.BREEZE);

        return page;
    }
}