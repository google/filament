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

#ifndef VK_DEBUG_SERVER_HPP_
#define VK_DEBUG_SERVER_HPP_

#include <memory>

namespace vk {
namespace dbg {

class Context;

// Server implements a Debug Adapter Protocol server that listens on a specific
// port, that operates on the vk::dbg::Context passed to the constructor.
class Server
{
public:
	static std::shared_ptr<Server> create(const std::shared_ptr<Context> &, int port);

	virtual ~Server() = default;

private:
	class Impl;
};

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_SERVER_HPP_
