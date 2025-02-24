# ~~~
# Copyright (c) 2021-2022 Valve Corporation
# Copyright (c) 2021-2022 LunarG, Inc.
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
# ~~~

set(TEST_LAYER_BASE_EXPORTS TEST_LAYER_EXPORT_ENUMERATE_FUNCTIONS=1)
set(TEST_LAYER_VERSION_0_EXPORTS ${TEST_LAYER_BASE_EXPORTS} TEST_LAYER_EXPORT_LAYER_VK_GIPA=1 TEST_LAYER_EXPORT_LAYER_VK_GDPA=1)
set(TEST_LAYER_VERSION_0_NAMED_GPA_EXPORTS ${TEST_LAYER_BASE_EXPORTS} TEST_LAYER_EXPORT_LAYER_NAMED_GIPA=1 TEST_LAYER_EXPORT_LAYER_NAMED_GDPA=1)
set(TEST_LAYER_VERSION_1_EXPORTS ${TEST_LAYER_VERSION_0_EXPORTS} TEST_LAYER_EXPORT_OVERRIDE_GIPA=1 TEST_LAYER_EXPORT_OVERRIDE_GDPA=1)
set(TEST_LAYER_VERSION_2_EXPORTS ${TEST_LAYER_VERSION_1_EXPORTS} TEST_LAYER_EXPORT_GET_PHYSICAL_DEVICE_PROC_ADDR=1
    LAYER_EXPORT_NEGOTIATE_LOADER_LAYER_INTERFACE_VERSION=1)

AddSharedLibrary(test_layer_export_base DEF_FILE test_layer_base SOURCES test_layer.cpp DEFINITIONS ${TEST_LAYER_BASE_EXPORTS})
AddSharedLibrary(test_layer_export_version_0 DEF_FILE test_layer_0 SOURCES test_layer.cpp DEFINITIONS ${TEST_LAYER_VERSION_0_EXPORTS})
AddSharedLibrary(test_layer_export_version_0_named_gpa DEF_FILE test_layer_0_named_gpa SOURCES test_layer.cpp DEFINITIONS ${TEST_LAYER_VERSION_0_NAMED_GPA_EXPORTS})
AddSharedLibrary(test_layer_export_version_1 DEF_FILE test_layer_1 SOURCES test_layer.cpp DEFINITIONS ${TEST_LAYER_VERSION_1_EXPORTS})
AddSharedLibrary(test_layer_export_version_2 DEF_FILE test_layer_2 SOURCES test_layer.cpp DEFINITIONS ${TEST_LAYER_VERSION_2_EXPORTS})

# Wrap Objects layer which wraps dispatchable handles to test that the loader will properly
# work in the case where handles in the terminator don't match handles in the trampoline portion.
set(WRAP_LAYER_VERSION_1_EXPORTS
        TEST_LAYER_EXPORT_DIRECT_DISP=1
        TEST_LAYER_EXPORT_MAINT_1=1
    )
set(WRAP_LAYER_VERSION_2_EXPORTS
        TEST_LAYER_EXPORT_DISP_SURF_COUNT=1
        TEST_LAYER_EXPORT_PRESENT_IMAGE=1
    )
set(WRAP_LAYER_VERSION_3_EXPORTS
        TEST_LAYER_EXPORT_DIRECT_DISP=1
        TEST_LAYER_EXPORT_DISP_SURF_COUNT=1
        TEST_LAYER_EXPORT_MAINT_1=1
        TEST_LAYER_EXPORT_PRESENT_IMAGE=1
    )

AddSharedLibrary(test_layer_wrap_objects DEF_FILE wrap_objects SOURCES wrap_objects.cpp DEFINITIONS "")
AddSharedLibrary(test_layer_wrap_objects_1 DEF_FILE wrap_objects_1 SOURCES wrap_objects.cpp DEFINITIONS ${WRAP_LAYER_VERSION_1_EXPORTS})
AddSharedLibrary(test_layer_wrap_objects_2 DEF_FILE wrap_objects_2 SOURCES wrap_objects.cpp DEFINITIONS ${WRAP_LAYER_VERSION_2_EXPORTS})
AddSharedLibrary(test_layer_wrap_objects_3 DEF_FILE wrap_objects_3 SOURCES wrap_objects.cpp DEFINITIONS ${WRAP_LAYER_VERSION_3_EXPORTS})
