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

#ifndef rr_Assert_hpp
#define rr_Assert_hpp

#include "Reactor.hpp"

namespace rr {

void failedAssert(const char *expression, const char *file, unsigned int line, const char *func);

#if defined(_MSC_VER)
#	define FUNCSIG __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
#	define FUNCSIG __PRETTY_FUNCTION__
#else
#	define FUNCSIG __func__
#endif

#ifdef NDEBUG
#	define Assert(expression) ((void)0)
#else
#	define Assert(expression)                                                                                                        \
		If(!(expression))                                                                                                             \
		{                                                                                                                             \
			Call(failedAssert, ConstantPointer(#expression), ConstantPointer(__FILE__), rr::Int(__LINE__), ConstantPointer(FUNCSIG)); \
		};
#endif

}  // namespace rr

#endif  // rr_Assert_hpp
