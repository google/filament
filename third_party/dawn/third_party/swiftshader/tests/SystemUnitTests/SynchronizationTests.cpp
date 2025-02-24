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

#include "System/Synchronization.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

TEST(EventCounter, ConstructUnsignalled)
{
	sw::CountedEvent ev;
	ASSERT_FALSE(ev.signalled());
}

TEST(EventCounter, ConstructSignalled)
{
	sw::CountedEvent ev(true);
	ASSERT_TRUE(ev.signalled());
}

TEST(EventCounter, Reset)
{
	sw::CountedEvent ev(true);
	ev.reset();
	ASSERT_FALSE(ev.signalled());
}

TEST(EventCounter, AddUnsignalled)
{
	sw::CountedEvent ev;
	ev.add();
	ASSERT_FALSE(ev.signalled());
}

TEST(EventCounter, AddDoneUnsignalled)
{
	sw::CountedEvent ev;
	ev.add();
	ev.done();
	ASSERT_TRUE(ev.signalled());
}

TEST(EventCounter, Wait)
{
	sw::CountedEvent ev;
	bool b = false;

	ev.add();
	auto t = std::thread([=, &b] {
		b = true;
		ev.done();
	});

	ev.wait();
	ASSERT_TRUE(b);
	t.join();
}

TEST(EventCounter, WaitNoTimeout)
{
	sw::CountedEvent ev;
	bool b = false;

	ev.add();
	auto t = std::thread([=, &b] {
		b = true;
		ev.done();
	});

	ASSERT_TRUE(ev.wait(std::chrono::system_clock::now() + std::chrono::seconds(10)));
	ASSERT_TRUE(b);
	t.join();
}

TEST(EventCounter, WaitTimeout)
{
	sw::CountedEvent ev;
	ev.add();
	ASSERT_FALSE(ev.wait(std::chrono::system_clock::now() + std::chrono::milliseconds(1)));
}
