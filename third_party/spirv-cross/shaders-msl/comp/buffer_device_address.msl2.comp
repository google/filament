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

layout(local_size_x = 8, local_size_y = 8) in;

// If we mark a buffer as buffer_reference, this is treated as a pointer type.
// A variable with the type Position is a 64-bit pointer to the data within.
// We can freely cast between pointer types if we wish, but that is not necessary in this sample.
// buffer_reference_align is used to let the underlying implementation know which alignment to expect.
// The pointer can have scalar alignment, which is something the compiler cannot know unless you tell it.
// It is best to use vector alignment when you can for optimal performance, but scalar alignment is sometimes useful.
// With SSBOs, the API has a minimum offset alignment which guarantees a minimum level of alignment from API side.

// It is possible to forward reference a pointer, so you can contain a pointer to yourself inside a struct.
// Useful if you need something like a linked list on the GPU.
// Here it's not particularly useful, but something to know about.
layout(buffer_reference) buffer Position;

layout(std430, buffer_reference, buffer_reference_align = 8) writeonly buffer Position
{
    vec2 positions[];
};

layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer PositionReferences
{
    // This buffer contains an array of pointers to other buffers.
    Position buffers[];
};

// In push constant we place a pointer to VBO pointers, spicy!
// This way we don't need any descriptor sets, but there's nothing wrong with combining use of descriptor sets and buffer device addresses.
// It is mostly done for convenience here.
layout(push_constant) uniform Registers
{
    PositionReferences references;
    // A buffer reference is 64-bit, so offset of fract_time is 8 bytes.
    float fract_time;
} registers;

void main()
{
    // Every slice is a 8x8 grid of vertices which we update here in compute.
    uvec2 local_offset = gl_GlobalInvocationID.xy;
    uint local_index = local_offset.y * gl_WorkGroupSize.x * gl_NumWorkGroups.x + local_offset.x;
    uint slice = gl_WorkGroupID.z;

    restrict Position positions = registers.references.buffers[slice];

    // This is a trivial wave-like function. Arbitrary for demonstration purposes.
    const float TWO_PI = 3.1415628 * 2.0;
    float offset = TWO_PI * fract(registers.fract_time + float(slice) * 0.1);

    // Simple grid.
    vec2 pos = vec2(local_offset);

    // Wobble, wobble.
    pos.x += 0.2 * sin(2.2 * pos.x + offset);
    pos.y += 0.2 * sin(2.25 * pos.y + 2.0 * offset);
    pos.x += 0.2 * cos(1.8 * pos.y + 3.0 * offset);
    pos.y += 0.2 * cos(2.85 * pos.x + 4.0 * offset);
    pos.x += 0.5 * sin(offset);
    pos.y += 0.5 * sin(offset + 0.3);

    // Center the mesh in [-0.5, 0.5] range.
    // Here we write to a raw pointer.
    // Be aware, there is no robustness support for buffer_device_address since we don't have a complete descriptor!
    positions.positions[local_index] = pos / (vec2(gl_WorkGroupSize.xy) * vec2(gl_NumWorkGroups.xy) - 1.0) - 0.5;
}
