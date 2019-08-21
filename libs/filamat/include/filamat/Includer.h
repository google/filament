/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMAT_INCLUDER_H
#define TNT_FILAMAT_INCLUDER_H

#include <utils/CString.h>

namespace filamat {

// Heavily influenced by glslang::TShader::Includer, but tailored for filamat / matc.
class Includer {
public:

    struct IncludeResult {
        // The full contents of the include file. This may contain additional, recursive #include
        // directives.
        utils::CString source;

        // The name of the include file. This gets passed as "includerName" for any includes inside
        // of source.
        utils::CString name;
    };

    // Called when an #include "file.h" directive is found.
    // headerName is the name referenced within the quotes
    // includerName is the name of the file with the #include directive
    virtual IncludeResult* includeLocal(const utils::CString& headerName,
            const utils::CString& includerName) {
        return nullptr;
    }

    // Called to free any memory allocated for the IncludeResult.
    virtual void releaseInclude(IncludeResult* result) { }

    virtual ~Includer() = default;

};

} // namespcae filamat

#endif
