%{
#include "IOSystem.h"
%}

// The const char* overload is used instead.
%ignore Assimp::IOSystem::Exists(const std::string&) const;
%ignore Assimp::IOSystem::Open(const std::string& pFile);
%ignore Assimp::IOSystem::Open(const std::string& pFile, const std::string& pMode);
%ignore Assimp::IOSystem::ComparePaths(const std::string& one, const std::string& second) const;

%include "IOSystem.h"
