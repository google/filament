/*!
\brief Contains a automated container class for managing Gles buffers and textures for a model.
\file PVRUtils/OpenGLES/ModelGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "ModelGles.h"

namespace pvr {
namespace utils {
void ModelGles::destroy()
{
	model = nullptr;
	for (auto& mesh : meshes)
	{
		if (mesh.vbos.size()) { gl::DeleteBuffers(static_cast<GLsizei>(mesh.vbos.size()), mesh.vbos.data()); }
		mesh.vbos.clear();
		if (mesh.ibo)
		{
			gl::DeleteBuffers(1, &mesh.ibo);
			mesh.ibo = 0;
		}
	}
	meshes.clear();
	if (textures.size())
	{
		gl::DeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
		textures.clear();
	}
}

void ModelGles::init(pvr::IAssetProvider& assetProvider, pvr::assets::Model& inModel, Flags flags)
{
	this->model = &inModel;
	textures.resize(this->model->getNumTextures());
	meshes.resize(this->model->getNumMeshes());
	if ((flags & Flags::LoadTextures) == Flags::LoadTextures)
	{
		for (uint32_t i = 0; i < this->model->getNumTextures(); ++i)
		{ textures[i] = pvr::utils::textureUpload(assetProvider, this->model->getTexture(i).getName().c_str(), (flags & Flags::GLES2Only) == Flags::GLES2Only); }
	}

	if ((flags & Flags::LoadMeshes) == Flags::LoadMeshes)
	{
		for (uint32_t i = 0; i < this->model->getNumMeshes(); ++i)
		{
			auto& mesh = this->model->getMesh(i);
			pvr::utils::createMultipleBuffersFromMesh(mesh, meshes[i].vbos, meshes[i].ibo);
		}
	}
}

void ModelGles::init(pvr::IAssetProvider& assetProvider, pvr::assets::ModelHandle& inModel, Flags flags)
{
	if (!inModel) { throw pvr::InvalidArgumentError("model", "Model cannot be an empty ModelHandle"); }
	modelHandle = inModel;
	init(assetProvider, *inModel, flags);
}

} // namespace utils
} // namespace pvr
//!\endcond
