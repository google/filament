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

#ifndef sw_SwiftConfig_hpp
#define sw_SwiftConfig_hpp

#include "Reactor/Nucleus.hpp"
#include "marl/scheduler.h"

#include <stdint.h>

namespace sw {

struct Configuration
{
	enum class AffinityPolicy : int
	{
		// A thread has affinity with any core in the affinity mask.
		AnyOf = 0,
		// A thread has affinity with a single core in the affinity mask.
		OneOf = 1,
	};

	// -------- [Processor] --------
	// Number of threads used by the scheduler. A thread count of 0 is
	// interpreted as min(cpu_cores_available, 16).
	uint32_t threadCount = 0;

	// Core affinity and affinity policy used by the scheduler.
	uint64_t affinityMask = 0xFFFFFFFFFFFFFFFFu;
	AffinityPolicy affinityPolicy = AffinityPolicy::AnyOf;

	// -------- [Profiler] --------
	// Whether SPIR-V profiling is enabled.
	bool enableSpirvProfiling = false;
	// Period controlling how often SPIR-V profiles are reported.
	uint64_t spvProfilingReportPeriodMs = 1000;
	// Directory where SPIR-V profile reports will be written.
	std::string spvProfilingReportDir = "";
};

// Get the configuration as parsed from a configuration file.
const Configuration &getConfiguration();

// Get the scheduler configuration given a configuration.
marl::Scheduler::Config getSchedulerConfiguration(const Configuration &config);

}  // namespace sw

#endif
