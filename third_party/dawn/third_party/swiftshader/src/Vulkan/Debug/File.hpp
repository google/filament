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

#ifndef VK_DEBUG_FILE_HPP_
#define VK_DEBUG_FILE_HPP_

#include "ID.hpp"

#include <memory>
#include <string>
#include <unordered_set>

namespace vk {
namespace dbg {

class File
{
public:
	using ID = dbg::ID<File>;

	// createVirtual() returns a new file that is not backed by the filesystem.
	// name is the name of the file.
	// source is the content of the file.
	static std::shared_ptr<File> createVirtual(ID id, std::string name, std::string source);

	// createPhysical() returns a new file that is backed by the file at path.
	static std::shared_ptr<File> createPhysical(ID id, std::string path);

	// clearBreakpoints() removes all the breakpoints set on the file.
	// This function and addBreakpoint() is safe to call concurrently on
	// multiple threads.
	virtual void clearBreakpoints() = 0;

	// addBreakpoint() adds a new line breakpoint at the line with the given
	// index.
	// This function and clearBreakpoints() is safe to call concurrently on
	// multiple threads.
	virtual void addBreakpoint(int line) = 0;

	// hasBreakpoint() returns true iff the file has a breakpoint set at the
	// line with the given index.
	virtual bool hasBreakpoint(int line) const = 0;

	// getBreakpoints() returns all the breakpoints set in the file.
	virtual std::unordered_set<int> getBreakpoints() const = 0;

	// isVirtual() returns true iff the file is not backed by the filesystem.
	virtual bool isVirtual() const = 0;

	// path() returns the path to the file, if backed by the filesystem,
	// otherwise and empty string.
	inline std::string path() const;

	// The unique identifier of the file.
	const ID id;

	// The directory of file if backed by the filesystem, otherwise an empty string.
	const std::string dir;

	// The name of the file.
	const std::string name;

	// The source of the file if not backed by the filesystem, otherwise an empty string.
	const std::string source;

	virtual ~File() = default;

protected:
	inline File(ID id, std::string dir, std::string name, std::string source);
};

File::File(ID id, std::string dir, std::string name, std::string source)
    : id(std::move(id))
    , dir(std::move(dir))
    , name(std::move(name))
    , source(source)
{}

std::string File::path() const
{
	return (dir.size() > 0) ? (dir + "/" + name) : name;
}

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_FILE_HPP_
