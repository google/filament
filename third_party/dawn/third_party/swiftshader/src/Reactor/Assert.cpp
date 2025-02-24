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

#include "Reactor.hpp"

#if defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#endif

#include <mutex>

namespace rr {

// Reports a failed Reactor Assert()
void failedAssert(const char *expression, const char *file, unsigned int line, const char *func)
{
	// Since multple threads could fail an Assert() simultaneously, enter a critical section.
	static std::mutex m;
	std::scoped_lock lock(m);

	fflush(stdout);

	char string[1024];
	snprintf(string, sizeof(string),
	         "Assertion failed: '%s'\n"
	         "At '%s:%d'\n"
	         "In '%s'\n\n",
	         expression, file, line, func);

	fprintf(stderr, "%s", string);
	fflush(stderr);

#if defined(_WIN32)
	OutputDebugStringA(string);
#endif

	assert(false && "Reactor Assert() failed");
}

}  // namespace rr
