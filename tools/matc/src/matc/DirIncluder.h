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

#ifndef TNT_DIRINCLUDER_H_
#define TNT_DIRINCLUDER_H_

#include <filamat/Includer.h>

#include <utils/Path.h>

namespace matc {

class DirIncluder : public filamat::Includer {
public:

    void setIncludeDirectory(utils::Path dir) noexcept {
        assert(dir.isDirectory());
        mIncludeDirectory = dir;
    }

    IncludeResult* includeLocal(const utils::CString& headerName,
            const utils::CString& includerName) final;

    void releaseInclude(IncludeResult* result) final;

private:
    utils::Path mIncludeDirectory;

};

} // namespace matc

#endif
