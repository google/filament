// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "VkSpecializationInfo.hpp"

#include "System/Memory.hpp"

#include <cstring>

namespace vk {

SpecializationInfo::SpecializationInfo(const VkSpecializationInfo *specializationInfo)
{
	if(specializationInfo && specializationInfo->mapEntryCount > 0)
	{
		info.mapEntryCount = specializationInfo->mapEntryCount;
		size_t entriesSize = specializationInfo->mapEntryCount * sizeof(VkSpecializationMapEntry);
		void *mapEntries = sw::allocate(entriesSize);
		memcpy(mapEntries, specializationInfo->pMapEntries, entriesSize);
		info.pMapEntries = reinterpret_cast<VkSpecializationMapEntry *>(mapEntries);

		info.dataSize = specializationInfo->dataSize;
		void *data = sw::allocate(specializationInfo->dataSize);
		memcpy(data, specializationInfo->pData, specializationInfo->dataSize);
		info.pData = data;
	}
}

SpecializationInfo::SpecializationInfo(const SpecializationInfo &copy)
    : SpecializationInfo(&copy.info)
{
}

SpecializationInfo::~SpecializationInfo()
{
	sw::freeMemory(const_cast<VkSpecializationMapEntry *>(info.pMapEntries));
	sw::freeMemory(const_cast<void *>(info.pData));
}

bool SpecializationInfo::operator<(const SpecializationInfo &rhs) const
{
	if(info.mapEntryCount != rhs.info.mapEntryCount)
	{
		return info.mapEntryCount < rhs.info.mapEntryCount;
	}

	if(info.dataSize != rhs.info.dataSize)
	{
		return info.dataSize < rhs.info.dataSize;
	}

	if(info.mapEntryCount > 0)
	{
		int cmp = memcmp(info.pMapEntries, rhs.info.pMapEntries, info.mapEntryCount * sizeof(VkSpecializationMapEntry));
		if(cmp != 0)
		{
			return cmp < 0;
		}
	}

	if(info.dataSize > 0)
	{
		int cmp = memcmp(info.pData, rhs.info.pData, info.dataSize);
		if(cmp != 0)
		{
			return cmp < 0;
		}
	}

	return false;
}

}  // namespace vk
