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

#ifndef sw_Memset_hpp
#define sw_Memset_hpp

#include <cstring>
#include <type_traits>

// GCC 8+ warns that
// "'void* memset(void*, int, size_t)' clearing an object of non-trivial type 'T';
//  use assignment or value-initialization instead [-Werror=class-memaccess]"
// This is benign iff it happens before any of the base or member constructors are called.
#if defined(__GNUC__) && (__GNUC__ >= 8)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
// Clang also warns that
// error: first argument in call to 'memset' is a pointer to non-trivially
// copyable type 'T' [-Werror,-Wnontrivial-memaccess]
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"
#endif

namespace sw {

// Memset<> is a helper class for clearing the memory of objects at construction.
// It is useful as the *first* base class of map keys which may contain padding
// bytes or bits otherwise left uninitialized.
template<class T>
struct Memset
{
	Memset(T *object, int val)
	{
		static_assert(std::is_base_of<Memset<T>, T>::value, "Memset<T> must only clear the memory of a type of which it is a base class");
		static_assert(!std::is_polymorphic<T>::value, "Memset<T> must not be used with classes that have virtual functions");
		::memset(object, 0, sizeof(T));
	}

	// Don't rely on the implicitly declared copy constructor and copy assignment operator.
	// They can leave padding bytes uninitialized.
	Memset(const Memset &rhs)
	{
		::memcpy(this, &rhs, sizeof(T));
	}

	Memset &operator=(const Memset &rhs)
	{
		::memcpy(this, &rhs, sizeof(T));
		return *this;
	}

	// The compiler won't declare an implicit move constructor and move assignment operator
	// due to having a user-defined copy constructor and copy assignment operator. Delete
	// them for explicitness. We always want memcpy() being called.
	Memset(const Memset &&rhs) = delete;
	Memset &operator=(const Memset &&rhs) = delete;

	friend bool operator==(const T &a, const T &b)
	{
		return ::memcmp(&a, &b, sizeof(T)) == 0;
	}

	friend bool operator!=(const T &a, const T &b)
	{
		return ::memcmp(&a, &b, sizeof(T)) != 0;
	}

	friend bool operator<(const T &a, const T &b)
	{
		return ::memcmp(&a, &b, sizeof(T)) < 0;
	}
};

}  // namespace sw

// Restore -Wclass-memaccess
#if defined(__GNUC__) && (__GNUC__ >= 8)
#	pragma GCC diagnostic pop
#endif

#endif  // sw_Memset_hpp