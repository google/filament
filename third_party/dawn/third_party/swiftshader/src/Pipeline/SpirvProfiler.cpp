// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#include "SpirvProfiler.hpp"

#include "System/Debug.hpp"

#include "spirv-tools/libspirv.h"

#include <string.h>
#include <atomic>
#include <fstream>

namespace {
std::string GetSpvOpName(const spv::Op &op)
{
	return std::string("Op") + spvOpcodeString(static_cast<uint32_t>(op));
}

std::string ConcatPath(const std::string &base, const std::string &file)
{
	if(base.size() == 0)
	{
		return file;
	}
	char lastChar = *base.rend();
	if(lastChar == '\\' || lastChar == '/')
	{
		return base + file;
	}
	return base + "/" + file;
}
}  // namespace

namespace sw {

SpirvProfiler::SpirvProfiler(const Configuration &config)
    : cfg(config)
{
	reportFilePath = ConcatPath(cfg.spvProfilingReportDir, "spirv_profile.txt");

	reportThreadStop = false;
	reportThread = std::thread{ [this] {
		while(!reportThreadStop.load(std::memory_order_acquire))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(cfg.spvProfilingReportPeriodMs));
			ReportSnapshot();
		}
	} };
}

SpirvProfiler::~SpirvProfiler()
{
	reportThreadStop.store(true, std::memory_order_release);
	reportThread.join();
}

void SpirvProfiler::ReportSnapshot()
{
	std::ofstream f{ reportFilePath };

	if(!f)
	{
		warn("Error writing SPIR-V profile to file %s: %s\n", reportFilePath.c_str(), strerror(errno));
		return;
	}

	auto profiles = GetRegisteredProfilesSnapshot();
	for(const auto &[shaderId, profileData] : profiles)
	{
		f << "[Shader " << shaderId << "]" << std::endl;

		f << "[SPIR-V operand execution count]" << std::endl;
		for(const auto &[spvOp, execCount] : profileData->spvOpExecutionCount)
		{
			f << GetSpvOpName(spvOp) << ": " << execCount << std::endl;
		}

		f << std::endl;
	}

	f.close();
}

void SpirvProfiler::RegisterShaderForProfiling(std::string shaderId, std::unique_ptr<SpirvProfileData> profData)
{
	marl::lock lock{ profileMux };
	shaderProfiles[shaderId] = std::move(profData);
}

std::unordered_map<std::string, SpirvProfileData *> SpirvProfiler::GetRegisteredProfilesSnapshot()
{
	marl::lock lock{ profileMux };
	std::unordered_map<std::string, SpirvProfileData *> snapshot;
	for(const auto &[shaderId, profileData] : shaderProfiles)
	{
		snapshot.emplace(shaderId, profileData.get());
	}
	return snapshot;
}
}  // namespace sw