#===============================================================================
# Copyright 2016-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

if (NOT MKLDNN_BUILD_EXAMPLES)
    return()
endif()

# propagate EXAMPLE specific flags
append(CMAKE_C_FLAGS "${CMAKE_EXAMPLE_CCXX_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_EXAMPLE_CCXX_FLAGS}")

# propagate sanitizer flags
append(CMAKE_C_FLAGS "${CMAKE_CCXX_SANITIZER_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_CCXX_SANITIZER_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR}/include)

set_if(UNIX LIBM m)

register_exe(simple-net-c simple_net.c "test")
register_exe(simple-net-cpp simple_net.cpp "test")
register_exe(simple-training-net-c simple_training_net.c "test" ${LIBM})
register_exe(simple-training-net-cpp simple_training_net.cpp "test" ${LIBM})
register_exe(simple-net-int8-cpp simple_net_int8.cpp "test")
register_exe(simple-rnn-cpp simple_rnn.cpp "test")
register_exe(simple-rnn-int8-cpp simple_rnn_int8.cpp "test")
register_exe(simple-rnn-training-cpp simple_rnn_training.cpp "test")
