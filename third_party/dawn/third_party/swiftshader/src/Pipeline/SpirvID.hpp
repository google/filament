// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_ID_hpp
#define sw_ID_hpp

#include "System/Debug.hpp"

#include <cstdint>
#include <unordered_map>

namespace sw {

// SpirvID is a strongly-typed identifier backed by a uint32_t.
// The template parameter T is not actually used by the implementation of
// ID; instead it is used to prevent implicit casts between identifiers of
// different T types.
// IDs are typically used as a map key to value of type T.
template<typename T>
class SpirvID
{
public:
	SpirvID() = default;
	inline SpirvID(uint32_t id);

	inline bool operator==(const SpirvID<T> &rhs) const;
	inline bool operator!=(const SpirvID<T> &rhs) const;
	inline bool operator<(const SpirvID<T> &rhs) const;

	// Returns the numerical value of the identifier.
	inline uint32_t value() const;

private:
	uint32_t id = 0;  // Valid ids are in the range "0 < id < Bound".
};

template<typename T>
SpirvID<T>::SpirvID(uint32_t id)
    : id(id)
{}

template<typename T>
bool SpirvID<T>::operator==(const SpirvID<T> &rhs) const
{
	return id == rhs.id;
}

template<typename T>
bool SpirvID<T>::operator!=(const SpirvID<T> &rhs) const
{
	return id != rhs.id;
}

template<typename T>
bool SpirvID<T>::operator<(const SpirvID<T> &rhs) const
{
	return id < rhs.id;
}

template<typename T>
uint32_t SpirvID<T>::value() const
{
	// Assert that we don't attempt to use unassigned IDs.
	// Use if(id != 0) { ... } to avoid invalid code paths.
	ASSERT(id != 0);

	return id;
}

// HandleMap<T> is an unordered map of SpirvID<T> to T.
template<typename T>
using HandleMap = std::unordered_map<SpirvID<T>, T>;

}  // namespace sw

namespace std {

// std::hash implementation for sw::SpirvID<T>
template<typename T>
struct hash<sw::SpirvID<T> >
{
	std::size_t operator()(const sw::SpirvID<T> &id) const noexcept
	{
		return std::hash<uint32_t>()(id.value());
	}
};

}  // namespace std

#endif  // sw_ID_hpp
