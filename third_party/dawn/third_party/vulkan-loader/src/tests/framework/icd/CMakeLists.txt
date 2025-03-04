# ~~~
# Copyright (c) 2021 Valve Corporation
# Copyright (c) 2021 LunarG, Inc.
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


set(TEST_ICD_VERSION_2_DEFINES TEST_ICD_EXPORT_NEGOTIATE_INTERFACE_VERSION=1 TEST_ICD_EXPORT_ICD_GIPA=1)

AddSharedLibrary(test_icd_export_none DEF_FILE test_icd_0
    SOURCES test_icd.cpp
    DEFINITIONS "")
AddSharedLibrary(test_icd_export_icd_gipa DEF_FILE test_icd_gipa
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_ICD_GIPA=1)
AddSharedLibrary(test_icd_export_negotiate_interface_version DEF_FILE test_icd_negotiate_version
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_NEGOTIATE_INTERFACE_VERSION=1)
AddSharedLibrary(test_icd_version_2 DEF_FILE test_icd_2
    SOURCES test_icd.cpp
    DEFINITIONS ${TEST_ICD_VERSION_2_DEFINES})
AddSharedLibrary(test_icd_version_2_export_icd_enumerate_adapter_physical_devices DEF_FILE test_icd_2_enum_adapter
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES=1 ${TEST_ICD_VERSION_2_DEFINES})
AddSharedLibrary(test_icd_version_2_export_icd_gpdpa DEF_FILE test_icd_2_gpdpa
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_ICD_GPDPA=1 ${TEST_ICD_VERSION_2_DEFINES})
AddSharedLibrary(test_icd_version_6 DEF_FILE test_icd_6
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_ICD_GPDPA=1 TEST_ICD_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES=1 ${TEST_ICD_VERSION_2_DEFINES})
AddSharedLibrary(test_icd_version_7 DEF_FILE test_icd_7
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_ICD_GPDPA=1 TEST_ICD_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES=1 TEST_ICD_EXPORT_VERSION_7=1 ${TEST_ICD_VERSION_2_DEFINES})
AddSharedLibrary(test_icd_version_7_without_exports DEF_FILE test_icd_7_without_exports
    SOURCES test_icd.cpp
    DEFINITIONS TEST_ICD_EXPORT_VERSION_7=0 TEST_ICD_EXPORT_ICD_GPDPA=1
        TEST_ICD_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES=1 ${TEST_ICD_VERSION_2_DEFINES})
AddSharedLibrary(test_unicode DEF_FILE 🌋
    SOURCES test_icd.cpp
    DEFINITIONS ${TEST_ICD_VERSION_2_DEFINES})
set_target_properties(test_unicode PROPERTIES OUTPUT_NAME "🌋") # Test unicode library
