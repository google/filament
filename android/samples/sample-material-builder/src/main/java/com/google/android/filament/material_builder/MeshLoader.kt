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

package com.google.android.filament.material_builder

import android.content.res.AssetManager
import android.util.Log

import com.google.android.filament.*
import com.google.android.filament.VertexBuffer.AttributeType.*
import com.google.android.filament.VertexBuffer.VertexAttribute.*

import java.io.InputStream
import java.nio.charset.Charset
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.Channels
import java.nio.channels.ReadableByteChannel

data class Mesh(
        @Entity val renderable: Int,
        val indexBuffer: IndexBuffer,
        val vertexBuffer: VertexBuffer,
        val aabb: Box)

fun destroyMesh(engine: Engine, mesh: Mesh) {
    engine.destroyEntity(mesh.renderable)
    engine.destroyIndexBuffer(mesh.indexBuffer)
    engine.destroyVertexBuffer(mesh.vertexBuffer)
    EntityManager.get().destroy(mesh.renderable)
}

fun loadMesh(assets: AssetManager, name: String,
             materials: Map<String, MaterialInstance>, engine: Engine): Mesh {
    // See tools/filamesh/README.md for a description of the filamesh file format
    assets.open(name).use { input ->
        val header = readHeader(input)

        val channel = Channels.newChannel(input)
        val vertexBufferData = readSizedData(channel, header.verticesSizeInBytes)
        val indexBufferData = readSizedData(channel, header.indicesSizeInBytes)

        val parts = readParts(header, input)
        val definedMaterials = readMaterials(input)

        val indexBuffer = createIndexBuffer(engine, header, indexBufferData)
        val vertexBuffer = createVertexBuffer(engine, header, vertexBufferData)

        val renderable = createRenderable(
                engine, header, indexBuffer, vertexBuffer, parts, definedMaterials, materials)

        return Mesh(renderable, indexBuffer, vertexBuffer, header.aabb)
    }
}

private const val FILAMESH_FILE_IDENTIFIER = "FILAMESH"
private const val MAX_UINT32 = 4294967295

@Suppress("unused")
private const val HEADER_FLAG_INTERLEAVED = 0x1L
private const val HEADER_FLAG_SNORM16_UV  = 0x2L
@Suppress("unused")
private const val HEADER_FLAG_COMPRESSED  = 0x4L

private class Header {
    var valid = false
    var versionNumber = 0L
    var parts = 0L
    var aabb = Box()
    var flags = 0L
    var posOffset = 0L
    var positionStride = 0L
    var tangentOffset = 0L
    var tangentStride = 0L
    var colorOffset = 0L
    var colorStride = 0L
    var uv0Offset = 0L
    var uv0Stride = 0L
    var uv1Offset = 0L
    var uv1Stride = 0L
    var totalVertices = 0L
    var verticesSizeInBytes = 0L
    var indices16Bit = 0L
    var totalIndices = 0L
    var indicesSizeInBytes = 0L
}

private class Part {
    var offset = 0L
    var indexCount = 0L
    var minIndex = 0L
    var maxIndex = 0L
    var materialID = 0L
    var aabb = Box()
}

private fun readMagicNumber(input: InputStream): Boolean {
    val temp = ByteArray(FILAMESH_FILE_IDENTIFIER.length)
    input.read(temp)
    val tempS = String(temp, Charset.forName("UTF-8"))
    return tempS == FILAMESH_FILE_IDENTIFIER
}

private fun readHeader(input: InputStream): Header {
    val header = Header()

    if (!readMagicNumber(input)) {
        Log.e("Filament", "Invalid filamesh file.")
        return header
    }

    header.versionNumber = readUIntLE(input)
    header.parts = readUIntLE(input)
    header.aabb = Box(
            readFloat32LE(input), readFloat32LE(input), readFloat32LE(input),
            readFloat32LE(input), readFloat32LE(input), readFloat32LE(input))
    header.flags = readUIntLE(input)
    header.posOffset = readUIntLE(input)
    header.positionStride = readUIntLE(input)
    header.tangentOffset = readUIntLE(input)
    header.tangentStride = readUIntLE(input)
    header.colorOffset = readUIntLE(input)
    header.colorStride = readUIntLE(input)
    header.uv0Offset = readUIntLE(input)
    header.uv0Stride = readUIntLE(input)
    header.uv1Offset = readUIntLE(input)
    header.uv1Stride = readUIntLE(input)
    header.totalVertices = readUIntLE(input)
    header.verticesSizeInBytes = readUIntLE(input)
    header.indices16Bit = readUIntLE(input)
    header.totalIndices = readUIntLE(input)
    header.indicesSizeInBytes = readUIntLE(input)

    header.valid = true
    return header
}

private fun readSizedData(channel: ReadableByteChannel, sizeInBytes: Long): ByteBuffer {
    val buffer = ByteBuffer.allocateDirect(sizeInBytes.toInt())
    buffer.order(ByteOrder.LITTLE_ENDIAN)
    channel.read(buffer)
    buffer.flip()
    return buffer
}

private fun readParts(header: Header, input: InputStream): List<Part> {
    return List(header.parts.toInt()) {
        val p = Part()
        p.offset = readUIntLE(input)
        p.indexCount = readUIntLE(input)
        p.minIndex = readUIntLE(input)
        p.maxIndex = readUIntLE(input)
        p.materialID = readUIntLE(input)
        p.aabb = Box(
                readFloat32LE(input), readFloat32LE(input), readFloat32LE(input),
                readFloat32LE(input), readFloat32LE(input), readFloat32LE(input))
        p
    }
}

private fun readMaterials(input: InputStream): List<String> {
    return List(readUIntLE(input).toInt()) {
        val data = ByteArray(readUIntLE(input).toInt())
        input.read(data)
        // Skip null terminator
        input.skip(1)
        data.toString(Charset.forName("UTF-8"))
    }
}

private fun createIndexBuffer(engine: Engine, header: Header, data: ByteBuffer): IndexBuffer {
    val indexType = if (header.indices16Bit != 0L) {
        IndexBuffer.Builder.IndexType.USHORT
    } else {
        IndexBuffer.Builder.IndexType.UINT
    }

    return IndexBuffer.Builder()
            .bufferType(indexType)
            .indexCount(header.totalIndices.toInt())
            .build(engine)
            .apply { setBuffer(engine, data) }
}

private fun uvNormalized(header: Header) = header.flags and HEADER_FLAG_SNORM16_UV != 0L

private fun createVertexBuffer(engine: Engine, header: Header, data: ByteBuffer): VertexBuffer {
    val uvType = if (!uvNormalized(header)) {
        HALF2
    } else {
        SHORT2
    }

    val vertexBufferBuilder = VertexBuffer.Builder()
            .bufferCount(1)
            .vertexCount(header.totalVertices.toInt())
            // We store colors as unsigned bytes (0..255) but the shader wants values in the 0..1
            // range so we must mark this attribute normalized
            .normalized(COLOR)
            // The same goes for the tangent frame: we store it as a signed short, but we want
            // values in 0..1 in the shader
            .normalized(TANGENTS)
            .attribute(POSITION, 0, HALF4, header.posOffset.toInt(), header.positionStride.toInt())
            .attribute(TANGENTS, 0, SHORT4, header.tangentOffset.toInt(), header.tangentStride.toInt())
            .attribute(COLOR, 0, UBYTE4, header.colorOffset.toInt(), header.colorStride.toInt())
            // UV coordinates are stored as normalized 16-bit integers or half-floats depending on
            // the range they span. When stored as half-float, there is only enough precision for
            // sub-pixel addressing in textures that are <= 1024x1024
            .attribute(UV0, 0, uvType, header.uv0Offset.toInt(), header.uv0Stride.toInt())
            // When UV coordinates are stored as 16-bit integers we must normalize them (we want
            // values in the range -1..1)
            .normalized(UV0, uvNormalized(header))

    if (header.uv1Offset != MAX_UINT32 && header.uv1Stride != MAX_UINT32) {
        vertexBufferBuilder
                .attribute(UV1, 0, uvType, header.uv1Offset.toInt(), header.uv1Stride.toInt())
                .normalized(UV1, uvNormalized(header))
    }

    return vertexBufferBuilder.build(engine).apply { setBufferAt(engine, 0, data) }
}

private fun createRenderable(
        engine: Engine,
        header: Header,
        indexBuffer: IndexBuffer,
        vertexBuffer: VertexBuffer,
        parts: List<Part>,
        definedMaterials: List<String>,
        materials: Map<String, MaterialInstance>): Int {

    val builder = RenderableManager.Builder(header.parts.toInt()).boundingBox(header.aabb)

    repeat(header.parts.toInt()) { i ->
        builder.geometry(i,
                RenderableManager.PrimitiveType.TRIANGLES,
                vertexBuffer,
                indexBuffer,
                parts[i].offset.toInt(),
                parts[i].minIndex.toInt(),
                parts[i].maxIndex.toInt(),
                parts[i].indexCount.toInt())

        // Find a material in the supplied material map, otherwise we fall back to
        // the default material named "DefaultMaterial"
        val material = materials[definedMaterials[parts[i].materialID.toInt()]]
        material?.let {
            builder.material(i, material)
        } ?: builder.material(i, materials["DefaultMaterial"]!!)
    }

    return EntityManager.get().create().apply { builder.build(engine, this) }
}
