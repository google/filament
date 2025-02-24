// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ReactorDebugInfo.hpp"
#include "Print.hpp"

#ifdef ENABLE_RR_DEBUG_INFO

#	include "boost/stacktrace.hpp"

#	include <algorithm>
#	include <cctype>
#	include <unordered_map>

namespace rr {

namespace {
std::string to_lower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(),
	               [](unsigned char c) { return std::tolower(c); });
	return str;
}

bool endswith_lower(const std::string &str, const std::string &suffix)
{
	size_t strLen = str.size();
	size_t suffixLen = suffix.size();

	if(strLen < suffixLen)
	{
		return false;
	}

	return to_lower(str).substr(strLen - suffixLen) == to_lower(suffix);
}
}  // namespace

Backtrace getCallerBacktrace(size_t limit /* = 0 */)
{
	namespace bs = boost::stacktrace;

	auto shouldSkipFile = [](const std::string &fileSR) {
		return fileSR.empty() ||
		       endswith_lower(fileSR, "ReactorDebugInfo.cpp") ||
		       endswith_lower(fileSR, "Reactor.cpp") ||
		       endswith_lower(fileSR, "Reactor.hpp") ||
		       endswith_lower(fileSR, "Traits.hpp") ||
		       endswith_lower(fileSR, "stacktrace.hpp");
	};

	auto offsetStackFrames = [](const bs::stacktrace &st) {
		// Return a stack trace with every stack frame address offset by -1. We do this so that we get
		// back the location of the function call, and not the location following it. We need this since
		// all debug info emits are the result of a function call. Note that technically we shouldn't
		// perform this offsetting on the top-most stack frame, but it doesn't matter as we discard it
		// anyway (see shouldSkipFile).

		std::vector<bs::frame> result;
		result.reserve(st.size());

		for(bs::frame frame : st)
		{
			result.emplace_back(reinterpret_cast<void *>(reinterpret_cast<size_t>(frame.address()) - 1));
		}

		return result;
	};

	std::vector<Location> locations;

	// Cache to avoid expensive stacktrace lookups, especially since our use-case results in looking up the
	// same call stack addresses many times.
	static std::unordered_map<bs::frame::native_frame_ptr_t, Location> cache;

	for(bs::frame frame : offsetStackFrames(bs::stacktrace()))
	{
		Location location;

		auto iter = cache.find(frame.address());
		if(iter == cache.end())
		{
			location.function.file = frame.source_file();
			location.function.name = frame.name();
			location.line = frame.source_line();
			cache[frame.address()] = location;
		}
		else
		{
			location = iter->second;
		}

		if(shouldSkipFile(location.function.file))
		{
			continue;
		}

		locations.push_back(std::move(location));

		if(limit > 0 && locations.size() >= limit)
		{
			break;
		}
	}

	std::reverse(locations.begin(), locations.end());

	return locations;
}

void emitPrintLocation(const Backtrace &backtrace)
{
#	ifdef ENABLE_RR_EMIT_PRINT_LOCATION
	static Location lastLocation;
	if(backtrace.size() == 0)
	{
		return;
	}
	Location currLocation = backtrace[backtrace.size() - 1];
	if(currLocation != lastLocation)
	{
		rr::Print("rr> {0} [{1}:{2}]\n", currLocation.function.name.c_str(), currLocation.function.file.c_str(), currLocation.line);
		lastLocation = std::move(currLocation);
	}
#	endif
}

}  // namespace rr

#endif  // ENABLE_RR_DEBUG_INFO
