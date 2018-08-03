%module assimp

// SWIG helpers for std::string and std::vector wrapping.
%include <std_string.i>
%include <std_vector.i>

// Globally enable enum prefix stripping.
%dstripprefix;


// PACK_STRUCT is a no-op for SWIG – it does not matter for the generated
// bindings how the underlying C++ code manages its memory.
#define PACK_STRUCT


// Helper macros for wrapping the pointer-and-length arrays used in the
// Assimp API.

%define ASSIMP_ARRAY(CLASS, TYPE, NAME, LENGTH)
%newobject CLASS::NAME;
%extend CLASS {
  std::vector<TYPE > *NAME() const {
    std::vector<TYPE > *result = new std::vector<TYPE >;
    result->reserve(LENGTH);

    for (unsigned int i = 0; i < LENGTH; ++i) {
      result->push_back($self->NAME[i]);
    }

    return result;
  }
}
%ignore CLASS::NAME;
%enddef

%define ASSIMP_POINTER_ARRAY(CLASS, TYPE, NAME, LENGTH)
%newobject CLASS::NAME;
%extend CLASS {
  std::vector<TYPE *> *NAME() const {
    std::vector<TYPE *> *result = new std::vector<TYPE *>;
    result->reserve(LENGTH);

    TYPE *currentValue = $self->NAME;
    TYPE *valueLimit = $self->NAME + LENGTH;
    while (currentValue < valueLimit) {
      result->push_back(currentValue);
      ++currentValue;
    }

    return result;
  }
}
%ignore CLASS::NAME;
%enddef

%define ASSIMP_POINTER_ARRAY_ARRAY(CLASS, TYPE, NAME, OUTER_LENGTH, INNER_LENGTH)
%newobject CLASS::NAME;
%extend CLASS {
  std::vector<std::vector<TYPE *> > *NAME() const {
    std::vector<std::vector<TYPE *> > *result = new std::vector<std::vector<TYPE *> >;
    result->reserve(OUTER_LENGTH);

    for (unsigned int i = 0; i < OUTER_LENGTH; ++i) {
      std::vector<TYPE *> currentElements;

      if ($self->NAME[i] != 0) {
        currentElements.reserve(INNER_LENGTH);

        TYPE *currentValue = $self->NAME[i];
        TYPE *valueLimit = $self->NAME[i] + INNER_LENGTH;
        while (currentValue < valueLimit) {
          currentElements.push_back(currentValue);
          ++currentValue;
        }
      }

      result->push_back(currentElements);
    }

    return result;
  }
}
%ignore CLASS::NAME;
%enddef


%include "interface/aiDefines.i"
%include "interface/aiTypes.i"
%include "interface/assimp.i"
%include "interface/aiTexture.i"
%include "interface/aiMatrix4x4.i"
%include "interface/aiMatrix3x3.i"
%include "interface/aiVector3D.i"
%include "interface/aiVector2D.i"
%include "interface/aiColor4D.i"
%include "interface/aiLight.i"
%include "interface/aiCamera.i"
%include "interface/aiFileIO.i"
%include "interface/aiAssert.i"
%include "interface/aiVersion.i"
%include "interface/aiAnim.i"
%include "interface/aiMaterial.i"
%include "interface/aiMesh.i"
%include "interface/aiPostProcess.i"
%include "interface/aiConfig.i"
%include "interface/assimp.i"
%include "interface/aiQuaternion.i"
%include "interface/aiScene.i"
%include "interface/Logger.i"
%include "interface/DefaultLogger.i"
%include "interface/NullLogger.i"
%include "interface/LogStream.i"
%include "interface/IOStream.i"
%include "interface/IOSystem.i"


// We have to "instantiate" the templates used by the ASSSIMP_*_ARRAY macros
// here at the end to avoid running into forward reference issues (SWIG would
// spit out the helper functions before the header includes for the element
// types otherwise).

%template(UintVector) std::vector<unsigned int>;
%template(aiAnimationVector) std::vector<aiAnimation *>;
%template(aiAnimMeshVector) std::vector<aiAnimMesh *>;
%template(aiBonesVector) std::vector<aiBone *>;
%template(aiCameraVector) std::vector<aiCamera *>;
%template(aiColor4DVector) std::vector<aiColor4D *>;
%template(aiColor4DVectorVector) std::vector<std::vector<aiColor4D *> >;
%template(aiFaceVector) std::vector<aiFace *>;
%template(aiLightVector) std::vector<aiLight *>;
%template(aiMaterialVector) std::vector<aiMaterial *>;
%template(aiMaterialPropertyVector) std::vector<aiMaterialProperty *>;
%template(aiMeshAnimVector) std::vector<aiMeshAnim *>;
%template(aiMeshVector) std::vector<aiMesh *>;
%template(aiNodeVector) std::vector<aiNode *>;
%template(aiNodeAnimVector) std::vector<aiNodeAnim *>;
%template(aiTextureVector) std::vector<aiTexture *>;
%template(aiVector3DVector) std::vector<aiVector3D *>;
%template(aiVector3DVectorVector) std::vector<std::vector<aiVector3D *> >;
%template(aiVertexWeightVector) std::vector<aiVertexWeight *>;
