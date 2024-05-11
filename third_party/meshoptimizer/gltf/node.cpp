// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include <math.h>
#include <string.h>

void markScenes(cgltf_data* data, std::vector<NodeInfo>& nodes)
{
	for (size_t i = 0; i < nodes.size(); ++i)
		nodes[i].scene = -1;

	for (size_t i = 0; i < data->scenes_count; ++i)
		for (size_t j = 0; j < data->scenes[i].nodes_count; ++j)
		{
			NodeInfo& ni = nodes[data->scenes[i].nodes[j] - data->nodes];

			if (ni.scene >= 0)
				ni.scene = -2; // multiple scenes
			else
				ni.scene = int(i);
		}

	for (size_t i = 0; i < data->nodes_count; ++i)
	{
		cgltf_node* root = &data->nodes[i];
		while (root->parent)
			root = root->parent;

		nodes[i].scene = nodes[root - data->nodes].scene;
	}
}

void markAnimated(cgltf_data* data, std::vector<NodeInfo>& nodes, const std::vector<Animation>& animations)
{
	for (size_t i = 0; i < animations.size(); ++i)
	{
		const Animation& animation = animations[i];

		for (size_t j = 0; j < animation.tracks.size(); ++j)
		{
			const Track& track = animation.tracks[j];

			// mark nodes that have animation tracks that change their base transform as animated
			if (!track.dummy)
			{
				NodeInfo& ni = nodes[track.node - data->nodes];

				ni.animated_paths |= (1 << track.path);
			}
		}
	}

	for (size_t i = 0; i < data->nodes_count; ++i)
	{
		NodeInfo& ni = nodes[i];

		for (cgltf_node* node = &data->nodes[i]; node; node = node->parent)
			ni.animated |= nodes[node - data->nodes].animated_paths != 0;
	}
}

void markNeededNodes(cgltf_data* data, std::vector<NodeInfo>& nodes, const std::vector<Mesh>& meshes, const std::vector<Animation>& animations, const Settings& settings)
{
	// mark all joints as kept
	for (size_t i = 0; i < data->skins_count; ++i)
	{
		const cgltf_skin& skin = data->skins[i];

		// for now we keep all joints directly referenced by the skin and the entire ancestry tree; we keep names for joints as well
		for (size_t j = 0; j < skin.joints_count; ++j)
		{
			NodeInfo& ni = nodes[skin.joints[j] - data->nodes];

			ni.keep = true;
		}
	}

	// mark all animated nodes as kept
	for (size_t i = 0; i < animations.size(); ++i)
	{
		const Animation& animation = animations[i];

		for (size_t j = 0; j < animation.tracks.size(); ++j)
		{
			const Track& track = animation.tracks[j];

			if (settings.anim_const || !track.dummy)
			{
				NodeInfo& ni = nodes[track.node - data->nodes];

				ni.keep = true;
			}
		}
	}

	// mark all mesh nodes as kept
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		const Mesh& mesh = meshes[i];

		for (size_t j = 0; j < mesh.nodes.size(); ++j)
		{
			NodeInfo& ni = nodes[mesh.nodes[j] - data->nodes];

			ni.keep = true;
		}
	}

	// mark all light/camera nodes as kept
	for (size_t i = 0; i < data->nodes_count; ++i)
	{
		const cgltf_node& node = data->nodes[i];

		if (node.light || node.camera)
		{
			nodes[i].keep = true;
		}
	}

	// mark all named nodes as needed (if -kn is specified)
	if (settings.keep_nodes)
	{
		for (size_t i = 0; i < data->nodes_count; ++i)
		{
			const cgltf_node& node = data->nodes[i];

			if (node.name && *node.name)
			{
				nodes[i].keep = true;
			}
		}
	}
}

void remapNodes(cgltf_data* data, std::vector<NodeInfo>& nodes, size_t& node_offset)
{
	// to keep a node, we currently need to keep the entire ancestry chain
	for (size_t i = 0; i < data->nodes_count; ++i)
	{
		if (!nodes[i].keep)
			continue;

		for (cgltf_node* node = &data->nodes[i]; node; node = node->parent)
			nodes[node - data->nodes].keep = true;
	}

	// generate sequential indices for all nodes; they aren't sorted topologically
	for (size_t i = 0; i < data->nodes_count; ++i)
	{
		NodeInfo& ni = nodes[i];

		if (ni.keep)
		{
			ni.remap = int(node_offset);

			node_offset++;
		}
	}
}

void decomposeTransform(float translation[3], float rotation[4], float scale[3], const float* transform)
{
	float m[4][4] = {};
	memcpy(m, transform, 16 * sizeof(float));

	// extract translation from last row
	translation[0] = m[3][0];
	translation[1] = m[3][1];
	translation[2] = m[3][2];

	// compute determinant to determine handedness
	float det =
	    m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
	    m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
	    m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

	float sign = (det < 0.f) ? -1.f : 1.f;

	// recover scale from axis lengths
	scale[0] = sqrtf(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]) * sign;
	scale[1] = sqrtf(m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1]) * sign;
	scale[2] = sqrtf(m[0][2] * m[0][2] + m[1][2] * m[1][2] + m[2][2] * m[2][2]) * sign;

	// normalize axes to get a pure rotation matrix
	float rsx = (scale[0] == 0.f) ? 0.f : 1.f / scale[0];
	float rsy = (scale[1] == 0.f) ? 0.f : 1.f / scale[1];
	float rsz = (scale[2] == 0.f) ? 0.f : 1.f / scale[2];

	float r00 = m[0][0] * rsx, r10 = m[1][0] * rsx, r20 = m[2][0] * rsx;
	float r01 = m[0][1] * rsy, r11 = m[1][1] * rsy, r21 = m[2][1] * rsy;
	float r02 = m[0][2] * rsz, r12 = m[1][2] * rsz, r22 = m[2][2] * rsz;

	// "branchless" version of Mike Day's matrix to quaternion conversion
	int qc = r22 < 0 ? (r00 > r11 ? 0 : 1) : (r00 < -r11 ? 2 : 3);
	float qs1 = qc & 2 ? -1.f : 1.f;
	float qs2 = qc & 1 ? -1.f : 1.f;
	float qs3 = (qc - 1) & 2 ? -1.f : 1.f;

	float qt = 1.f - qs3 * r00 - qs2 * r11 - qs1 * r22;
	float qs = 0.5f / sqrtf(qt);

	rotation[qc ^ 0] = qs * qt;
	rotation[qc ^ 1] = qs * (r01 + qs1 * r10);
	rotation[qc ^ 2] = qs * (r20 + qs2 * r02);
	rotation[qc ^ 3] = qs * (r12 + qs3 * r21);
}
