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

package com.google.android.filament.tungsten;

import com.google.android.filament.tungsten.ui.GuiComponentFactory;
import javax.swing.JComponent;
import javax.swing.JSplitPane;

public class SwingComponentFactory implements GuiComponentFactory {

    @Override
    public JComponent createThreeComponentSplitLayout(JComponent first, JComponent second,
            JComponent third) {
        JSplitPane verticalSplit = new JSplitPane(JSplitPane.VERTICAL_SPLIT, second, third);
        JSplitPane horizontalSplit = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, first,
                verticalSplit);
        verticalSplit.setContinuousLayout(true);
        horizontalSplit.setContinuousLayout(true);
        return horizontalSplit;
    }
}
