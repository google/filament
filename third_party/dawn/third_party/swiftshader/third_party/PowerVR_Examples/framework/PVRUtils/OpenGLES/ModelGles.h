/*!
\brief Contains a automated container class for managing Gles buffers and textures for a model.
\file PVRUtils/OpenGLES/ModelGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/Model.h"
#include "PVRUtils/OpenGLES/HelperGles.h"

namespace pvr {
namespace utils {

/// <summary>The ModelGles class provides the functionality for creating all of the buffers (vbos and ibos) and textures required for basic rendering of
/// a pvr::assets::Model using OpenGL ES.</summary>
class ModelGles
{
public:
	/// <summary>The ApiMeshGles structure handles the encapsulation of all buffers (vbos and ibos) for a particular mesh of a model.</summary>
	struct ApiMeshGles
	{
		/// <summary>A list of vertex buffer objects in use for the mesh.</summary>
		std::vector<GLuint> vbos;

		/// <summary>An index buffer objects in use for the mesh.</summary>
		GLuint ibo;
	};
	enum class Flags
	{
		None = 0,
		LoadMeshes = 1,
		LoadTextures = 2,
		GLES2Only = 4
	};

private:
	pvr::assets::ModelHandle modelHandle;
	pvr::assets::Model* model;

	std::vector<ApiMeshGles> meshes;
	std::vector<GLuint> textures;

	void destroy();

public:
	/// <summary>Getter for the pvr::assets::Model used to create the ModelGles class.</summary>
	/// <returns>Returns the pvr::assets::Model which was used to create this ModelGles class.</returns>
	pvr::assets::Model& getModel() { return *model; }

	/// <summary>Getter for the pvr::assets::Model used to create the ModelGles class.</summary>
	/// <returns>Returns the pvr::assets::Model which was used to create this ModelGles class.</returns>
	pvr::assets::ModelHandle& getModelHandle() { return modelHandle; }

	/// <summary>Default Constructor for a ModelGles class.</summary>
	ModelGles() : model(nullptr) {}

	/// <summary>Destructor for a ModelGles class. The destructor deletes and frees the buffers and textures being used by the ModelGles class.</summary>
	~ModelGles() { destroy(); }

	/// <summary>Initialises the ModelGles class using a pvr::IAssetProvider and pvr::assets::Model.</summary>
	/// <param name="assetProvider">A pvr::IAssetProvider used for loading assets from file.</param>
	/// <param name="inModel">A pvr::assets::Model which specifies the buffers and textures which are required for basic rendering of the Model.</param>
	/// <param name="isEs2">The isEs2 flag affects whether only OpenGL ES 2.0 functionality is used or whether newer OpenGL ES 3+ functionality can be used.</param>
	void init(pvr::IAssetProvider& assetProvider, pvr::assets::Model& inModel, Flags flags = (Flags)((int)Flags::LoadMeshes | (int)Flags::LoadTextures));

	/// <summary>Initialises the ModelGles class using a pvr::IAssetProvider and pvr::assets::Model.</summary>
	/// <param name="assetProvider">A pvr::IAssetProvider used for loading assets from file.</param>
	/// <param name="inModel">A pvr::assets::ModelHandle which specifies the buffers and textures which are required for basic rendering of the Model.</param>
	/// <param name="isEs2">The isEs2 flag affects whether only OpenGL ES 2.0 functionality is used or whether newer OpenGL ES 3+ functionality can be used.</param>
	void init(pvr::IAssetProvider& assetProvider, pvr::assets::ModelHandle& inModel, Flags flags = (Flags)((int)Flags::LoadMeshes | (int)Flags::LoadTextures));

	/// <summary>Getter for an OpenGL ES texture handle for a particular pvr::assets::Model texture index.</summary>
	/// <param name="texId">The pvr::assets::Model texture index.</param>
	/// <returns>Returns the OpenGL ES handle for the texture with id texId.</returns>
	GLuint getApiTextureById(uint32_t texId) { return textures[texId]; }

	/// <summary>Getter for an OpenGL ES texture handle for a particular node of the pvr::assets::Model with the provided texture semantic.</summary>
	/// <param name="nodeId">The node identifier into the pvr::assets::Model for which to retrieve the texture.</param>
	/// <param name="texSemantic">The texture semantic name for the pvr::assets::Model for which to retrieve the texture.</param>
	/// <returns>Returns the OpenGL ES handle for the texture with name matching texSemantic for the node specified.</returns>
	GLuint getApiTextureByNode(uint32_t nodeId, const pvr::StringHash& texSemantic) { return getApiTextureByMaterial(model->getNode(nodeId).getMaterialIndex(), texSemantic); }

	/// <summary>Getter for an OpenGL ES texture handle for a particular material of the pvr::assets::Model with the provided texture semantic.</summary>
	/// <param name="materialId">The material identifier into the pvr::assets::Model for which to retrieve the texture.</param>
	/// <param name="texSemantic">The texture semantic name for the pvr::assets::Model for which to retrieve the texture.</param>
	/// <returns>Returns the OpenGL ES handle for the texture with name matching texSemantic for the material specified.</returns>
	GLuint getApiTextureByMaterial(uint32_t materialId, const pvr::StringHash& texSemantic) { return textures[model->getMaterial(materialId).getTextureIndex(texSemantic)]; }

	/// <summary>Getter for a particular ApiMeshGles structure which encapsulates buffers (vbos and ibos) for rendering the mesh.</summary>
	/// <param name="meshId">The mesh identifier into the pvr::assets::Model for which to retrieve the ApiMeshGles.</param>
	/// <returns>Returns the ApiMeshGles for the mesh specified.</returns>
	ApiMeshGles& getApiMeshById(uint32_t meshId) { return meshes[meshId]; }

	/// <summary>Getter for a particular ApiMeshGles structure which encapsulates buffers (vbos and ibos) for rendering the mesh.</summary>
	/// <param name="nodeId">The node identifier into the pvr::assets::Model for which to retrieve the ApiMeshGles.</param>
	/// <returns>Returns the ApiMeshGles for the node specified.</returns>
	ApiMeshGles& getApiMeshByNodeId(uint32_t nodeId) { return meshes[model->getNode(nodeId).getObjectId()]; }

	/// <summary>Getter for a particular vertex buffer object for the specified mesh and vbo.</summary>
	/// <param name="meshId">The mesh identifier into the pvr::assets::Model for which to retrieve the vertex buffer object for.</param>
	/// <param name="vboId">The specific vertex buffer object to retrieve for the mesh.</param>
	/// <returns>Returns the vertex buffer object for the mesh specified at vboId index.</returns>
	GLuint getVboByMeshId(uint32_t meshId, uint32_t vboId) { return meshes[meshId].vbos[vboId]; }

	/// <summary>Getter for a particular index buffer object for the specified mesh.</summary>
	/// <param name="meshId">The mesh identifier into the pvr::assets::Model for which to retrieve the index buffer object for.</param>
	/// <returns>Returns the index buffer object for the mesh specified.</returns>
	GLuint getIboByMeshId(uint32_t meshId) { return meshes[meshId].ibo; }

	/// <summary>Getter for a particular vertex buffer object for the specified node and vbo.</summary>
	/// <param name="nodeId">The node identifier into the pvr::assets::Model for which to retrieve the vertex buffer object for.</param>
	/// <param name="vboId">The specific vertex buffer object to retrieve for the node.</param>
	/// <returns>Returns the vertex buffer object for the node specified at vboId index.</returns>
	GLuint getVboByNodeId(uint32_t nodeId, uint32_t vboId) { return getVboByMeshId(model->getNode(nodeId).getObjectId(), vboId); }

	/// <summary>Getter for a particular index buffer object for the specified node.</summary>
	/// <param name="nodeId">The node identifier into the pvr::assets::Model for which to retrieve the index buffer object for.</param>
	/// <returns>Returns the index buffer object for the node specified.</returns>
	GLuint getIboByNodeId(uint32_t nodeId) { return getIboByMeshId(model->getNode(nodeId).getObjectId()); }
};
inline ModelGles::Flags operator|(ModelGles::Flags lhs, ModelGles::Flags rhs) { return (ModelGles::Flags)((int)lhs | (int)rhs); }
inline ModelGles::Flags& operator|=(ModelGles::Flags& lhs, ModelGles::Flags rhs) { return lhs = (lhs | rhs); }
inline ModelGles::Flags operator&(ModelGles::Flags lhs, ModelGles::Flags rhs) { return (ModelGles::Flags)((int)lhs & (int)rhs); }
inline ModelGles::Flags& operator&=(ModelGles::Flags& lhs, ModelGles::Flags rhs) { return lhs = (lhs & rhs); }
} // namespace utils
} // namespace pvr
