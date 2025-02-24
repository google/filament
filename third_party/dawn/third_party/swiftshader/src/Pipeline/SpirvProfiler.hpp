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

#ifndef sw_SpirvProfiler_hpp
#define sw_SpirvProfiler_hpp

#include "System/SwiftConfig.hpp"

#define SPV_ENABLE_UTILITY_CODE
#include <spirv/unified1/spirv.hpp>

#include "marl/mutex.h"

#include <atomic>
#include <cstdint>
#include <thread>
#include <unordered_map>

namespace sw {

// SpirvProfileData contains bookkeeping data structures for SPIR-V
// instrumentation.
class SpirvProfileData
{
public:
	SpirvProfileData() = default;
	SpirvProfileData(const SpirvProfileData &) = delete;
	SpirvProfileData(SpirvProfileData &&) = delete;
	SpirvProfileData &operator=(const SpirvProfileData &) = delete;
	SpirvProfileData &operator=(SpirvProfileData &&) = delete;

	// SPIR-V instruction execution count, by opcode.
	std::unordered_map<spv::Op, int64_t> spvOpExecutionCount;
};

class SpirvProfiler
{
public:
	SpirvProfiler(const Configuration &config);
	~SpirvProfiler();

	void RegisterShaderForProfiling(std::string shaderId, std::unique_ptr<SpirvProfileData> profData);

private:
	void ReportSnapshot();
	std::unordered_map<std::string, SpirvProfileData *> GetRegisteredProfilesSnapshot();

	const Configuration &cfg;
	std::string reportFilePath;

	std::thread reportThread;
	std::atomic<bool> reportThreadStop;

	// Map from shader ID to associated profile data.
	std::unordered_map<std::string, std::unique_ptr<SpirvProfileData>> shaderProfiles GUARDED_BY(profileMux);
	marl::mutex profileMux;
};

}  // namespace sw

#endif  // sw_SpirvProfiler_hpp