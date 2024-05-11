%{
#include "assimp.hpp"
%}


namespace Assimp {

// See docs in assimp.hpp.
%ignore Importer::ReadFile(const std::string& pFile, unsigned int pFlags);
%ignore Importer::GetExtensionList(std::string& szOut);
%ignore Importer::IsExtensionSupported(const std::string& szExtension);

// These are only necessary for extending Assimp with custom importers or post
// processing steps, which would require wrapping the internal BaseImporter and
// BaseProcess classes.
%ignore Importer::RegisterLoader(BaseImporter* pImp);
%ignore Importer::UnregisterLoader(BaseImporter* pImp);
%ignore Importer::RegisterPPStep(BaseProcess* pImp);
%ignore Importer::UnregisterPPStep(BaseProcess* pImp);
%ignore Importer::FindLoader(const char* szExtension);

}


// Each aiScene has to keep a reference to the Importer to prevent it from
// being garbage collected, whose destructor would release the underlying
// C++ memory the scene is stored in.
%typemap(dcode) aiScene "package Object m_importer;"
%typemap(dout)
    aiScene* GetScene,
    aiScene* ReadFile,
    aiScene* ApplyPostProcessing,
    aiScene* ReadFileFromMemory {
  void* cPtr = $wcall;
  $dclassname ret = (cPtr is null) ? null : new $dclassname(cPtr, $owner);$excode
  ret.m_importer = this;
  return ret;
}

%include <typemaps.i>
%apply bool *OUTPUT { bool *bWasExisting };

%include "assimp.hpp"

%clear bool *bWasExisting;
