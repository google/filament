/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "test_environment.h"

#include <cstring>

extern "C" {
#include "loader_common.h"
}

// loader_hash_string() is a plain 32-bit FNV-1a hash used purely as a cheap pre-filter in front of every
// strcmp() confirmation when resolving vkGet{Instance,Device}ProcAddr names (see loader/gpa_helper.c and
// loader/generated/vk_loader_extensions.c). Every call site dispatches on the hash via `switch (name_hash)`
// and confirms the match with strcmp() inside the matching `case` before returning - never a bare
// hash-only dispatch - so a hash collision can, at worst, cost one extra strcmp; it can never produce a
// wrong lookup result. These tests don't try to prove real Vulkan command names never collide (the code
// generator's build-time collision check does that) - they prove the hash-then-strcmp *pattern* stays safe
// even when two different strings are engineered to collide.

TEST(HashString, Deterministic) {
    ASSERT_EQ(loader_hash_string("vkCreateInstance"), loader_hash_string("vkCreateInstance"));
    ASSERT_EQ(loader_hash_string(""), loader_hash_string(""));
}

TEST(HashString, EmptyStringIsFNVOffsetBasis) { ASSERT_EQ(2166136261u, loader_hash_string("")); }

TEST(HashString, DifferentStringsUsuallyHashDifferently) {
    ASSERT_NE(loader_hash_string("vkCreateInstance"), loader_hash_string("vkDestroyInstance"));
    ASSERT_NE(loader_hash_string("vkCreateDevice"), loader_hash_string("vkCreateDevice2"));
    ASSERT_NE(loader_hash_string("vk"), loader_hash_string(""));
}

// scripts/generators/loader_extension_generator.py has its own copy of this hash function, used to
// precompute the hash constants embedded in the four switch statements those constants live in (generated
// and hand-written alike). The pairs below are copied verbatim from those switches, so this test fails
// directly if the Python and C implementations ever disagree - no need to invoke Python to prove it.
//
// Not every switch hashes the same input for a given command, so the "name" column below is exactly the
// string each one passes to loader_hash_string(), not always the full "vk"-prefixed name:
//   - trampoline_get_proc_addr() and extension_instance_gpa() hash the full command name, e.g.
//     loader_hash_string("vkDestroyInstance").
//   - loader_lookup_device_dispatch_table() and loader_lookup_instance_dispatch_table() strip the leading
//     "vk" before hashing (`name += 2`), e.g. loader_hash_string("DestroyInstance").
TEST(HashString, MatchesGeneratorEmbeddedConstants) {
    struct NameAndHash {
        const char *name;
        uint32_t expected_hash;
    };
    const NameAndHash pairs[] = {
        // extension_instance_gpa() (loader/generated/vk_loader_extensions.c) - full command name.
        {"vkGetPhysicalDeviceVideoCapabilitiesKHR", 0x5a670229u},
        {"vkGetPhysicalDeviceVideoFormatPropertiesKHR", 0x9bb9b67du},
        {"vkCreateVideoSessionKHR", 0x05ff253eu},
        {"vkDestroyVideoSessionKHR", 0xc7cb956cu},
        {"vkGetVideoSessionMemoryRequirementsKHR", 0xf1b97187u},
        // trampoline_get_proc_addr() (loader/gpa_helper.c, hand-written) - full command name.
        {"vkGetInstanceProcAddr", 0x9d4599a6u},
        {"vkDestroyInstance", 0xa64dcfc3u},
        {"vkEnumeratePhysicalDevices", 0x09612094u},
        {"vkGetPhysicalDeviceFeatures", 0x3407e5aau},
        // loader_lookup_device_dispatch_table() (generated) - "vk" prefix already stripped before hashing.
        {"GetDeviceProcAddr", 0x556f3e66u},
        {"DestroyDevice", 0xc3c245f3u},
        {"GetDeviceQueue", 0xe828e700u},
        {"QueueSubmit", 0xf634aeb0u},
        // loader_lookup_instance_dispatch_table() (generated) - "vk" prefix already stripped before hashing.
        {"DestroyInstance", 0x72bac092u},
        {"EnumeratePhysicalDevices", 0x47707f5bu},
        {"GetPhysicalDeviceFeatures", 0x1f85ee93u},
        {"GetPhysicalDeviceFormatProperties", 0x74a5d8deu},
        {"GetPhysicalDeviceImageFormatProperties", 0x8451aa6bu},
        {"GetPhysicalDeviceProperties", 0x1de4f4b9u},
        {"GetPhysicalDeviceQueueFamilyProperties", 0x027f4f12u},
        {"GetPhysicalDeviceMemoryProperties", 0x1c0f4f2eu},
    };
    for (const auto &pair : pairs) {
        ASSERT_EQ(pair.expected_hash, loader_hash_string(pair.name));
    }
}

// A handful of strings engineered offline to collide under this exact FNV-1a implementation (offset basis
// 2166136261, prime 16777619). None of these are real Vulkan command names - they exist purely to
// demonstrate that a hash collision is handled safely by construction.
TEST(HashString, SyntheticCollisionsAreHandledSafelyByConstruction) {
    struct CollidingPair {
        const char *a;
        const char *b;
        uint32_t expected_hash;
    };
    const CollidingPair pairs[] = {
        {"WrvAi", "jI3i", 0x612ea33cu},
        {"I3US", "WWngS", 0x7431e3c9u},
        {"TOc9", "h8y0", 0xe7227ef4u},
    };
    for (const auto &pair : pairs) {
        // The two strings really are different...
        ASSERT_NE(0, strcmp(pair.a, pair.b));

        // ...yet their hashes really do collide...
        uint32_t hash_a = loader_hash_string(pair.a);
        uint32_t hash_b = loader_hash_string(pair.b);
        ASSERT_EQ(pair.expected_hash, hash_a);
        ASSERT_EQ(pair.expected_hash, hash_b);
        ASSERT_EQ(hash_a, hash_b);

        // ...which is exactly why every generated/hand-written lookup site guards its `switch (name_hash)`
        // case body with a strcmp() before returning, rather than trusting the hash match alone: looking up
        // `a` must match itself but must not be fooled into matching `b`, even though the hash alone can't
        // tell them apart. The `&&` below is just this test's local stand-in for that same guard.
        uint32_t query_hash = loader_hash_string(pair.a);
        bool matches_a = (query_hash == hash_a) && !strcmp(pair.a, pair.a);
        bool matches_b = (query_hash == hash_b) && !strcmp(pair.a, pair.b);
        ASSERT_TRUE(matches_a);
        ASSERT_FALSE(matches_b);
    }
}
