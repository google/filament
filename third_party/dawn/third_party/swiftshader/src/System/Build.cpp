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

#if defined(_MSVC_LANG)
#	define CPP_VERSION _MSVC_LANG
#elif defined(__cplusplus)
// Note: This must come after checking for _MSVC_LANG.
// See https://developercommunity.visualstudio.com/content/problem/139261/msvc-incorrectly-defines-cplusplus.html
#	define CPP_VERSION __cplusplus
#endif

#if !defined(CPP_VERSION) || CPP_VERSION <= 1
#	error "Unable to identify C++ language version"
#endif

// The template and unused function below verifies the compiler is using at least
// C++17. It will print an error message containing the actual C++ version if
// the version is < 17.

namespace {

template<int version>
class cpp
{
	static_assert(version >= 2017, "SwiftShader requires at least C++17");
};

void check_cpp_version()
{
	cpp<CPP_VERSION / 100>();
	(void)&check_cpp_version;  // Unused reference to avoid unreferenced function warning.
}

}  // namespace
