/*
 * Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#pragma once

#include "binding.h"

namespace vkt {

template <typename VertexT>
Buffer VertexBuffer(const Device &dev, const std::vector<VertexT> &vertices) {
    vkt::Buffer vertex_buffer(dev, vertices.size() * sizeof(VertexT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    auto *vertex_buffer_ptr = static_cast<VertexT *>(vertex_buffer.Memory().Map());
    std::copy(vertices.data(), vertices.data() + vertices.size(), vertex_buffer_ptr);
    vertex_buffer.Memory().Unmap();
    return vertex_buffer;
}

template <typename IndexT>
Buffer IndexBuffer(const Device &dev, const std::vector<IndexT> &indices) {
    vkt::Buffer index_buffer(dev, indices.size() * sizeof(IndexT), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    auto *index_buffer_ptr = static_cast<IndexT *>(index_buffer.Memory().Map());
    std::copy(indices.data(), indices.data() + indices.size(), index_buffer_ptr);
    index_buffer.Memory().Unmap();
    return index_buffer;
}

// stride == sizeof(IndirectCmdT)
template <typename IndirectCmdT>
Buffer IndirectBuffer(const Device &dev, const std::vector<IndirectCmdT> &indirect_cmds) {
    vkt::Buffer indirect_buffer(dev, indirect_cmds.size() * sizeof(IndirectCmdT), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto *indirect_buffer_ptr = static_cast<IndirectCmdT *>(indirect_buffer.Memory().Map());
    std::copy(indirect_cmds.data(), indirect_cmds.data() + indirect_cmds.size(), indirect_buffer_ptr);
    indirect_buffer.Memory().Unmap();
    return indirect_buffer;
}

}  // namespace vkt
