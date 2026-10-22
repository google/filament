/* Copyright (c) 2021, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#version 450

// Allows buffer_reference.
#extension GL_EXT_buffer_reference : require

// Since we did not enable vertexPipelineStoresAndAtomics, we must mark everything readonly.
layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer Position
{
    vec2 positions[];
};

layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer PositionReferences
{
    // Represents an array of pointers, where each pointer points to its own VBO (Position).
    // The size of a pointer (VkDeviceAddress) is always 8 in Vulkan.
    Position buffers[];
};

layout(push_constant) uniform Registers
{
    mat4 view_projection;

    // This is a pointer to an array of pointers, essentially:
    // const VBO * const *vbos
    PositionReferences references;
} registers;

// Flat shading looks a little cooler here :)
layout(location = 0) flat out vec4 out_color;

void main()
{
    int slice = gl_InstanceIndex;

    // One VBO per instance, load the VBO pointer.
    // The cool thing here is that a compute shader could hypothetically
    // write the pointer list where vertices are stored.
    // With vertex attributes we do not have the luxury to modify VBO bindings on the GPU.
    // The best we can do is to just modify the vertexOffset in an indirect draw call,
    // but that's not always flexible enough, and enforces a very specific engine design to work.
    // We can even modify the attribute layout per slice here, since we can just cast the pointer
    // to something else if we want.
    restrict Position positions = registers.references.buffers[slice];

    // Load the vertex based on VertexIndex instead of an attribute. Fully flexible.
    // Only downside is that we do not get format conversion for free like we do with normal vertex attributes.
    vec2 pos = positions.positions[gl_VertexIndex] * 2.5;

    // Place the quad meshes on screen and center it.
    pos += 3.0 * (vec2(slice % 8, slice / 8) - 3.5);

    // Normal projection.
    gl_Position = registers.view_projection * vec4(pos, 0.0, 1.0);

    // Color the vertex. Use a combination of a wave and checkerboard, completely arbitrary.
    int index_x = gl_VertexIndex % 16;
    int index_y = gl_VertexIndex / 16;

    float r = 0.5 + 0.3 * sin(float(index_x));
    float g = 0.5 + 0.3 * sin(float(index_y));

    int checkerboard = (index_x ^ index_y) & 1;
    r *= float(checkerboard) * 0.8 + 0.2;
    g *= float(checkerboard) * 0.8 + 0.2;

    out_color = vec4(r, g, 0.15, 1.0);
}
