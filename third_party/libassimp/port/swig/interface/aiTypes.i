%{
#include "aiTypes.h"
%}

// The const char* overload is used instead.
%ignore aiString::Set(const std::string& pString);

%include "aiTypes.h"
