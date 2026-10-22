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

package com.google.android.filament.filamat;

import androidx.annotation.NonNull;
import java.nio.ByteBuffer;

public class MaterialPackage {

    private final ByteBuffer mBuffer;
    private final boolean mIsValid;

    MaterialPackage(@NonNull ByteBuffer buffer, boolean isValid) {
        mBuffer = buffer;
        mIsValid = isValid;
    }

    @NonNull
    public ByteBuffer getBuffer() {
        return mBuffer;
    }

    public boolean isValid() {
        return mIsValid;
    }
}

