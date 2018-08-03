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

import com.google.android.filament.Engine;
import com.google.android.filament.IndexBuffer;
import com.google.android.filament.VertexBuffer;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.Channels;
import java.nio.channels.ReadableByteChannel;

public class FilameshLoader {

    public static final String FILAMESH_FILE_IDENTIFIER = "FILAMESH";

    private static class Float3 {
        float x, y, z;

        public Float3(float x, float y, float z) {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    private static class FilameshHeader {
        int versionNumber;
        int numberOfParts;
        Float3 totalBoundingBoxCenter;
        Float3 totalBoundingBoxHalfExtent;
        int interleaved;
        int positionAttributeOffset;
        int positionAttributeStride;
        int tangentAttributeOffset;
        int tangentAttributeStride;
        int colorAttributeOffset;
        int colorAttributeStride;
        int uv0AttributeOffset;
        int uv0AttributeStride;
        int uv1AttributeOffset;
        int uv1AttributeStride;
        int totalVertices;
        int verticesSizeInBytes;
        int indices16Bit;       // 0 if indices are stored as int, 1 if stored as uint16
        int totalIndices;
        int indicesSizeInBytes;
    };

    private static boolean readMagicNumber(InputStream in) throws IOException {
        final byte[] temp = new byte[FILAMESH_FILE_IDENTIFIER.length()];
        in.read(temp);
        String tempS = new String(temp, "UTF-8");
        return tempS.equals(FILAMESH_FILE_IDENTIFIER);
    }

    private static FilameshHeader readHeader(InputStream in) throws IOException {
        FilameshHeader header = new FilameshHeader();
        if (!readMagicNumber(in)) {
            System.err.print("Invalid filamesh file.");
            System.exit(1);
        }
        header.versionNumber = IOUtils.readIntLE(in);
        header.numberOfParts = IOUtils.readIntLE(in);
        header.totalBoundingBoxCenter =
                new Float3(IOUtils.readFloat32LE(in), IOUtils.readFloat32LE(in), IOUtils.readFloat32LE(in));
        header.totalBoundingBoxHalfExtent =
                new Float3(IOUtils.readFloat32LE(in), IOUtils.readFloat32LE(in), IOUtils.readFloat32LE(in));
        header.interleaved = IOUtils.readIntLE(in);
        header.positionAttributeOffset = IOUtils.readIntLE(in);
        header.positionAttributeStride = IOUtils.readIntLE(in);
        header.tangentAttributeOffset = IOUtils.readIntLE(in);
        header.tangentAttributeStride = IOUtils.readIntLE(in);
        header.colorAttributeOffset = IOUtils.readIntLE(in);
        header.colorAttributeStride = IOUtils.readIntLE(in);
        header.uv0AttributeOffset = IOUtils.readIntLE(in);
        header.uv0AttributeStride = IOUtils.readIntLE(in);
        header.uv1AttributeOffset = IOUtils.readIntLE(in);
        header.uv1AttributeStride = IOUtils.readIntLE(in);
        header.totalVertices = IOUtils.readIntLE(in);
        header.verticesSizeInBytes = IOUtils.readIntLE(in);
        header.indices16Bit = IOUtils.readIntLE(in);
        header.totalIndices = IOUtils.readIntLE(in);
        header.indicesSizeInBytes = IOUtils.readIntLE(in);
        return header;
    }

    public static Filamesh load(String name, InputStream in, Engine engine) {
        try {
            FilameshHeader header = readHeader(in);

            if (header.numberOfParts > 1) {
               System.out.println("Mesh " + name + " has " + header.numberOfParts + " parts.");
               System.out.println("Currently, only 1 part supported.");
            }

            ReadableByteChannel channel = Channels.newChannel(in);

            ByteBuffer vertexBuffer = ByteBuffer.allocateDirect(header.verticesSizeInBytes);
            vertexBuffer.order(ByteOrder.LITTLE_ENDIAN);
            channel.read(vertexBuffer);
            vertexBuffer.flip();

            ByteBuffer indexBuffer = ByteBuffer.allocateDirect(header.indicesSizeInBytes);
            indexBuffer.order(ByteOrder.LITTLE_ENDIAN);
            channel.read(indexBuffer);
            indexBuffer.flip();

            VertexBuffer vb = new VertexBuffer.Builder()
                    .bufferCount(1)
                    .vertexCount(header.totalVertices)
                    .attribute(VertexBuffer.VertexAttribute.POSITION, 0, VertexBuffer.AttributeType.HALF4,
                            header.positionAttributeOffset, header.positionAttributeStride)
                    .attribute(VertexBuffer.VertexAttribute.TANGENTS, 0, VertexBuffer.AttributeType.SHORT4,
                            header.tangentAttributeOffset, header.tangentAttributeStride)
                    .attribute(VertexBuffer.VertexAttribute.COLOR, 0, VertexBuffer.AttributeType.UBYTE4,
                            header.colorAttributeOffset, header.colorAttributeStride)
                    .attribute(VertexBuffer.VertexAttribute.UV0, 0, VertexBuffer.AttributeType.HALF2,
                            header.uv0AttributeOffset, header.uv0AttributeStride)
                    .build(engine);
            vb.setBufferAt(engine, 0, vertexBuffer);

            IndexBuffer ib = new IndexBuffer.Builder()
                    .bufferType(IndexBuffer.Builder.IndexType.USHORT)
                    .indexCount(header.totalIndices)
                    .build(engine);
            ib.setBuffer(engine, indexBuffer);

            return new Filamesh(name, vb, ib);
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println("Error loading mesh: " + name);
        }

        return null;
    }
}
