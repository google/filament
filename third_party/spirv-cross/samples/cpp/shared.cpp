/*
 * Copyright 2015-2017 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spirv_cross/external_interface.h"
#include <stdio.h>

#ifndef GLM_SWIZZLE
#define GLM_SWIZZLE
#endif

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include <glm/glm.hpp>
using namespace glm;

int main()
{
	// First, we get the C interface to the shader.
	// This can be loaded from a dynamic library, or as here,
	// linked in as a static library.
	auto *iface = spirv_cross_get_interface();

	// Create an instance of the shader interface.
	auto *shader = iface->construct();

// Build some input data for our compute shader.
#define NUM_WORKGROUPS 4
	float a[128 * NUM_WORKGROUPS];
	float b[NUM_WORKGROUPS] = {};

	for (int i = 0; i < 128 * NUM_WORKGROUPS; i++)
	{
		a[i] = float(i);
	}

	void *aptr = a;
	void *bptr = b;

	// Bind resources to the shader.
	// For resources like samplers and buffers, we provide a list of pointers,
	// since UBOs, SSBOs and samplers can be arrays, and can point to different types,
	// which is especially true for samplers.
	spirv_cross_set_resource(shader, 0, 0, &aptr, sizeof(aptr));
	spirv_cross_set_resource(shader, 0, 1, &bptr, sizeof(bptr));

	// We also have to set builtins.
	// The relevant builtins will depend on the shader,
	// but for compute, there are few builtins, which are gl_NumWorkGroups and gl_WorkGroupID.
	// LocalInvocationID and GlobalInvocationID are inferred when executing the invocation.
	uvec3 num_workgroups(NUM_WORKGROUPS, 1, 1);
	uvec3 work_group_id(0, 0, 0);
	spirv_cross_set_builtin(shader, SPIRV_CROSS_BUILTIN_NUM_WORK_GROUPS, &num_workgroups, sizeof(num_workgroups));
	spirv_cross_set_builtin(shader, SPIRV_CROSS_BUILTIN_WORK_GROUP_ID, &work_group_id, sizeof(work_group_id));

	// Execute 4 work groups.
	for (unsigned i = 0; i < NUM_WORKGROUPS; i++)
	{
		work_group_id.x = i;
		iface->invoke(shader);
	}

	// Call destructor.
	iface->destruct(shader);

	// Verify our output.
	// TODO: Implement a test framework that asserts results computed.
	for (unsigned i = 0; i < NUM_WORKGROUPS; i++)
	{
		float expected_sum = 0.0f;
		for (unsigned j = i * 128; j < (i + 1) * 128; j++)
			expected_sum += a[j];
		fprintf(stderr, "Sum in workgroup #%u = %.1f, expected %.1f\n", i, b[i], expected_sum);
	}
}
