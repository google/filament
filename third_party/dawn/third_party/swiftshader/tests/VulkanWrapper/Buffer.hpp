// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BENCHMARKS_BUFFER_HPP_
#define BENCHMARKS_BUFFER_HPP_

#include "VulkanHeaders.hpp"

class Buffer
{
public:
	Buffer(vk::Device device, vk::DeviceSize size, vk::BufferUsageFlags usage);
	~Buffer();

	vk::Buffer getBuffer()
	{
		return buffer;
	}

	void *mapMemory()
	{
		return device.mapMemory(bufferMemory, 0, size);
	}

	void unmapMemory()
	{
		device.unmapMemory(bufferMemory);
	}

private:
	const vk::Device device;
	vk::DeviceSize size;
	vk::Buffer buffer;              // Owning handle
	vk::DeviceMemory bufferMemory;  // Owning handle
};

#endif  // BENCHMARKS_BUFFER_HPP_
