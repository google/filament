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

#include "System/Memory.hpp"
#ifdef __linux__
#	include "System/Linux/MemFd.hpp"
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>

using namespace sw;

#ifdef __linux__
TEST(MemFd, DefaultConstructor)
{
	LinuxMemFd memfd;
	ASSERT_FALSE(memfd.isValid());
	ASSERT_EQ(-1, memfd.exportFd());
}

TEST(MemFd, AllocatingConstructor)
{
	const size_t kRegionSize = sw::memoryPageSize() * 8;
	LinuxMemFd memfd("test-region", kRegionSize);
	ASSERT_TRUE(memfd.isValid());
	ASSERT_GE(memfd.fd(), 0);
	void *addr = memfd.mapReadWrite(0, kRegionSize);
	ASSERT_TRUE(addr);
	memfd.unmap(addr, kRegionSize);
}

TEST(MemFd, ExplicitAllocation)
{
	const size_t kRegionSize = sw::memoryPageSize() * 8;
	LinuxMemFd memfd;
	ASSERT_FALSE(memfd.isValid());
	ASSERT_EQ(-1, memfd.exportFd());
	ASSERT_TRUE(memfd.allocate("test-region", kRegionSize));
	ASSERT_TRUE(memfd.isValid());
}

TEST(MemFd, Close)
{
	const size_t kRegionSize = sw::memoryPageSize() * 8;
	LinuxMemFd memfd("test-region", kRegionSize);
	ASSERT_TRUE(memfd.isValid());
	int fd = memfd.exportFd();
	memfd.close();
	ASSERT_FALSE(memfd.isValid());
	ASSERT_EQ(-1, memfd.exportFd());
	::close(fd);
}

TEST(MemFd, ExportImportFd)
{
	const size_t kRegionSize = sw::memoryPageSize() * 8;
	LinuxMemFd memfd("test-region1", kRegionSize);
	auto *addr = reinterpret_cast<uint8_t *>(memfd.mapReadWrite(0, kRegionSize));
	ASSERT_TRUE(addr);
	for(size_t n = 0; n < kRegionSize; ++n)
	{
		addr[n] = static_cast<uint8_t>(n);
	}
	int fd = memfd.exportFd();
	ASSERT_TRUE(memfd.unmap(addr, kRegionSize));
	memfd.close();

	LinuxMemFd memfd2;
	memfd2.importFd(fd);
	ASSERT_TRUE(memfd2.isValid());
	addr = reinterpret_cast<uint8_t *>(memfd2.mapReadWrite(0, kRegionSize));
	ASSERT_TRUE(addr);
	for(size_t n = 0; n < kRegionSize; ++n)
	{
		ASSERT_EQ(addr[n], static_cast<uint8_t>(n)) << "# " << n;
	}
	ASSERT_TRUE(memfd2.unmap(addr, kRegionSize));
}
#endif  // __linux__
