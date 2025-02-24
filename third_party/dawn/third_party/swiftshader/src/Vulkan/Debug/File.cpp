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

#include "File.hpp"

#include "marl/mutex.h"

namespace {

////////////////////////////////////////////////////////////////////////////////
// FileBase
////////////////////////////////////////////////////////////////////////////////
class FileBase : public vk::dbg::File
{
public:
	void clearBreakpoints() override;
	void addBreakpoint(int line) override;
	bool hasBreakpoint(int line) const override;
	std::unordered_set<int> getBreakpoints() const override;

protected:
	FileBase(ID id, std::string dir, std::string name, std::string source);

private:
	mutable marl::mutex breakpointMutex;
	std::unordered_set<int> breakpoints GUARDED_BY(breakpointMutex);
};

FileBase::FileBase(ID id, std::string dir, std::string name, std::string source)
    : File(id, std::move(dir), std::move(name), std::move(source))
{}

void FileBase::clearBreakpoints()
{
	marl::lock lock(breakpointMutex);
	breakpoints.clear();
}

void FileBase::addBreakpoint(int line)
{
	marl::lock lock(breakpointMutex);
	breakpoints.emplace(line);
}

bool FileBase::hasBreakpoint(int line) const
{
	marl::lock lock(breakpointMutex);
	return breakpoints.count(line) > 0;
}

std::unordered_set<int> FileBase::getBreakpoints() const
{
	marl::lock lock(breakpointMutex);
	return breakpoints;
}

////////////////////////////////////////////////////////////////////////////////
// VirtualFile
////////////////////////////////////////////////////////////////////////////////
class VirtualFile : public FileBase
{
public:
	VirtualFile(ID id, std::string name, std::string source);

	bool isVirtual() const override;

private:
};

VirtualFile::VirtualFile(ID id, std::string name, std::string source)
    : FileBase(id, "", std::move(name), std::move(source))
{}

bool VirtualFile::isVirtual() const
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// PhysicalFile
////////////////////////////////////////////////////////////////////////////////
struct PhysicalFile : public FileBase
{
	PhysicalFile(ID id,
	             std::string dir,
	             std::string name);

	bool isVirtual() const override;
};

PhysicalFile::PhysicalFile(ID id,
                           std::string dir,
                           std::string name)
    : FileBase(id, std::move(dir), std::move(name), "")
{}

bool PhysicalFile::isVirtual() const
{
	return false;
}

}  // anonymous namespace

namespace vk {
namespace dbg {

std::shared_ptr<File> File::createVirtual(ID id, std::string name, std::string source)
{
	return std::make_shared<VirtualFile>(id, std::move(name), std::move(source));
}

std::shared_ptr<File> File::createPhysical(ID id, std::string path)
{
	auto pathstr = path;
	auto pos = pathstr.rfind("/");
	if(pos != std::string::npos)
	{
		auto dir = pathstr.substr(0, pos);
		auto name = pathstr.substr(pos + 1);
		return std::make_shared<PhysicalFile>(id, dir.c_str(), name.c_str());
	}
	return std::make_shared<PhysicalFile>(id, "", std::move(path));
}

}  // namespace dbg
}  // namespace vk
