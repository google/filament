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

#ifndef VK_DEBUG_VALUE_HPP_
#define VK_DEBUG_VALUE_HPP_

#include "TypeOf.hpp"

#include "System/Debug.hpp"

#include <memory>
#include <string>

namespace vk {
namespace dbg {

class Variables;
class VariableContainer;

// FormatFlags holds settings used to serialize a Value to a string.
struct FormatFlags
{
	// The default FormatFlags used to serialize a Value to a string.
	static const FormatFlags Default;

	std::string listPrefix;         // Prefix to lists.
	std::string listSuffix;         // Suffix to lists.
	std::string listDelimiter;      // List item delimiter.
	std::string listIndent;         // List item indententation prefix.
	const FormatFlags *subListFmt;  // Format used for list sub items.
};

// Value holds a value that can be read.
class Value
{
public:
	virtual ~Value() = default;

	// type() returns the typename for the value.
	virtual std::string type() = 0;

	// get() returns a string representation of the value using the specified
	// FormatFlags.
	virtual std::string get(const FormatFlags & = FormatFlags::Default) = 0;

	// children() returns the optional child members of this value.
	virtual std::shared_ptr<Variables> children() { return nullptr; }
};

// Constant is constant value of type T.
template<typename T>
class Constant : public Value
{
public:
	Constant(const T &val)
	    : val(val)
	{}
	std::string type() override { return TypeOf<T>::name; }
	std::string get(const FormatFlags &fmt = FormatFlags::Default) override { return std::to_string(val); }

private:
	T const val;
};

// Reference is reference to a value in memory.
template<typename T>
class Reference : public Value
{
public:
	Reference(const T &ref)
	    : ref(ref)
	{}
	std::string type() override { return TypeOf<T>::name; }
	std::string get(const FormatFlags &fmt = FormatFlags::Default) override { return std::to_string(ref); }

private:
	const T &ref;
};

// Struct is an implementation of Value that delegates calls to children() on to
// the constructor provided Variables.
class Struct : public Value
{
public:
	Struct(const std::string &type, const std::shared_ptr<Variables> &members)
	    : ty(type)
	    , members(members)
	{
		ASSERT(members);
	}

	std::string type() override { return ty; }
	std::string get(const FormatFlags &fmt = FormatFlags::Default) override;
	std::shared_ptr<Variables> children() override { return members; }

	// create() constructs and returns a new Struct with the given type name and
	// calls fields to populate the child members.
	// fields must be a function that has the signature:
	//   void(std::shared_pointer<VariableContainer>&)
	template<typename F>
	static std::shared_ptr<Struct> create(const std::string &name, F &&fields)
	{
		auto vc = std::make_shared<VariableContainer>();
		fields(vc);
		return std::make_shared<Struct>(name, vc);
	}

private:
	std::string const ty;
	std::shared_ptr<Variables> const members;
};

// make_constant() returns a shared_ptr to a Constant with the given value.
template<typename T>
inline std::shared_ptr<Constant<T>> make_constant(const T &value)
{
	return std::make_shared<Constant<T>>(value);
}

// make_reference() returns a shared_ptr to a Reference with the given value.
template<typename T>
inline std::shared_ptr<Reference<T>> make_reference(const T &value)
{
	return std::make_shared<Reference<T>>(value);
}

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_VALUE_HPP_
