/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.google.android.filament.tungsten.ui;

import javax.swing.JComponent;

public interface GuiComponentFactory {

    /**
     * Creates a Panel comprised of 3 components separated by platform-specific splitters.
     *    _________
     *   |    |    |
     *   |    | 2  |
     *   | 1  |____|
     *   |    |    |
     *   |    | 3  |
     *   |____|____|
     */
    JComponent createThreeComponentSplitLayout(JComponent first, JComponent second,
            JComponent third);
}
