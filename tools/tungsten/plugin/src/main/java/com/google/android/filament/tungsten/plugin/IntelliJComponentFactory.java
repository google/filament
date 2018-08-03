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

package com.google.android.filament.tungsten.plugin;

import com.google.android.filament.tungsten.ui.GuiComponentFactory;
import com.intellij.ui.JBSplitter;
import javax.swing.JComponent;

public class IntelliJComponentFactory implements GuiComponentFactory {

    @Override
    public JComponent createThreeComponentSplitLayout(JComponent first, JComponent second,
            JComponent third) {
        JBSplitter verticalSplit = new JBSplitter(true);
        verticalSplit.setFirstComponent(second);
        verticalSplit.setSecondComponent(third);
        JBSplitter horizontalSplit = new JBSplitter(false);
        horizontalSplit.setFirstComponent(first);
        horizontalSplit.setSecondComponent(verticalSplit);
        return horizontalSplit;
    }
}
