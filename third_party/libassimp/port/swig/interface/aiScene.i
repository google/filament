%{
#include "aiScene.h"
%}


ASSIMP_ARRAY(aiScene, aiAnimation*, mAnimations, $self->mNumAnimations);
ASSIMP_ARRAY(aiScene, aiCamera*, mCameras, $self->mNumCameras);
ASSIMP_ARRAY(aiScene, aiLight*, mLights, $self->mNumLights);
ASSIMP_ARRAY(aiScene, aiMaterial*, mMaterials, $self->mNumMaterials);
ASSIMP_ARRAY(aiScene, aiMesh*, mMeshes, $self->mNumMeshes);
ASSIMP_ARRAY(aiScene, aiTexture*, mTextures, $self->mNumTextures);

ASSIMP_ARRAY(aiNode, aiNode*, mChildren, $self->mNumChildren);
ASSIMP_ARRAY(aiNode, unsigned int, mMeshes, $self->mNumMeshes);


%include "aiScene.h"
