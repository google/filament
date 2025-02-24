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

#ifndef rr_ReactorDebugInfo_hpp
#define rr_ReactorDebugInfo_hpp

#ifdef ENABLE_RR_DEBUG_INFO

#	include <vector>
#	include <string>

namespace rr {

struct FunctionLocation
{
	std::string name;
	std::string file;

	bool operator==(const FunctionLocation &rhs) const { return name == rhs.name && file == rhs.file; }
	bool operator!=(const FunctionLocation &rhs) const { return !(*this == rhs); }

	struct Hash
	{
		std::size_t operator()(const FunctionLocation &l) const noexcept
		{
			return std::hash<std::string>()(l.file) * 31 +
			       std::hash<std::string>()(l.name);
		}
	};
};

struct Location
{
	FunctionLocation function;
	unsigned int line = 0;

	bool operator==(const Location &rhs) const { return function == rhs.function && line == rhs.line; }
	bool operator!=(const Location &rhs) const { return !(*this == rhs); }

	struct Hash
	{
		std::size_t operator()(const Location &l) const noexcept
		{
			return FunctionLocation::Hash()(l.function) * 31 +
			       std::hash<unsigned int>()(l.line);
		}
	};
};

using Backtrace = std::vector<Location>;

// Returns the backtrace for the callstack, starting at the first
// non-Reactor file. If limit is non-zero, then a maximum of limit
// frames will be returned.
Backtrace getCallerBacktrace(size_t limit = 0);

// Emits a print location for the top of the input backtrace.
void emitPrintLocation(const Backtrace &backtrace);

}  // namespace rr

#endif  // ENABLE_RR_DEBUG_INFO

#endif  // rr_ReactorDebugInfo_hpp
