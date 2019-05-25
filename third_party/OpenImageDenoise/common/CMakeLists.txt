## ======================================================================== ##
## Copyright 2009-2019 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

add_library(common STATIC
  platform.h
  platform.cpp
  exception.h
  ref.h
  barrier.h
  timer.h
  thread.h
  thread.cpp
  tasking.h
  tasking.cpp
  tensor.h
  tensor.cpp
)

if(OIDN_STATIC_LIB)
  target_compile_definitions(common PUBLIC -DOIDN_STATIC_LIB)
endif()

target_include_directories(common
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

target_link_libraries(common PUBLIC ${TBB_LIBRARIES})
if(UNIX)
  target_link_libraries(common PUBLIC pthread)
  if(NOT(APPLE))
    target_link_libraries(common PUBLIC rt)
  endif()
endif()
