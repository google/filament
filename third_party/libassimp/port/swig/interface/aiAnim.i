%{
#include "aiAnim.h"
%}

ASSIMP_ARRAY(aiAnimation, aiNodeAnim*, mChannels, $self->mNumChannels);
ASSIMP_ARRAY(aiAnimation, aiMeshAnim*, mMeshChannels, $self->mNumMeshChannels);

%include "aiAnim.h"
