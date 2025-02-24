# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of te License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This file sets configuration values for CMake's GraphViz generation.
# See https://cmake.org/cmake/help/latest/module/CMakeGraphVizOptions.html

# Defaults
set(GRAPHVIZ_EXECUTABLES TRUE)
set(GRAPHVIZ_STATIC_LIBS TRUE)
set(GRAPHVIZ_SHARED_LIBS TRUE)
set(GRAPHVIZ_MODULE_LIBS TRUE)
set(GRAPHVIZ_INTERFACE_LIBS TRUE)
set(GRAPHVIZ_OBJECT_LIBS TRUE)
set(GRAPHVIZ_UNKNOWN_LIBS TRUE)
set(GRAPHVIZ_CUSTOM_TARGETS TRUE)

# Below are non-defaults

# Render larger and with a nicer font. Default clips bottom of some characters.
set(GRAPHVIZ_GRAPH_HEADER "node [ fontsize=16; fontname=Helvetica ];")

# Don't show external lib targets
set(GRAPHVIZ_EXTERNAL_LIBS FALSE)

# Don't generate per-target dot files
set(GRAPHVIZ_GENERATE_PER_TARGET FALSE)

# Don't generate per-target depender dot files
set(GRAPHVIZ_GENERATE_DEPENDERS FALSE)

# List of targets to ignore
set(GRAPHVIZ_IGNORE_TARGETS
    core_tables
    enum_string_mapping
    extinst_tables
    gmock_main
    gtest_main
    spirv-*
    SPIRV-Tools-*
    spv-*
)
