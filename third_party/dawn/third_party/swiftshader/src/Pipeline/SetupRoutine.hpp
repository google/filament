// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_SetupRoutine_hpp
#define sw_SetupRoutine_hpp

#include "Device/SetupProcessor.hpp"
#include "Reactor/Reactor.hpp"

namespace sw {

class Context;

class SetupRoutine
{
public:
	SetupRoutine(const SetupProcessor::State &state);

	virtual ~SetupRoutine();

	void generate();
	SetupFunction::RoutineType getRoutine();

private:
	void setupGradient(Pointer<Byte> &primitive, Pointer<Byte> &triangle, Float4 &w012, Float4 (&m)[3], Pointer<Byte> &v0, Pointer<Byte> &v1, Pointer<Byte> &v2, int attribute, int planeEquation, bool flatShading, bool perspective);
	void edge(Pointer<Byte> &primitive, Pointer<Byte> &data, const Int &Xa, const Int &Ya, const Int &Xb, const Int &Yb, Int &q);
	void conditionalRotate1(Bool condition, Pointer<Byte> &v0, Pointer<Byte> &v1, Pointer<Byte> &v2);
	void conditionalRotate2(Bool condition, Pointer<Byte> &v0, Pointer<Byte> &v1, Pointer<Byte> &v2);

	const SetupProcessor::State &state;

	SetupFunction::RoutineType routine;
};

}  // namespace sw

#endif  // sw_SetupRoutine_hpp
