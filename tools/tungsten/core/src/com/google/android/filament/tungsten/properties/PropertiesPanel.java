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

package com.google.android.filament.tungsten.properties;

import com.google.android.filament.tungsten.model.NodeModel;
import javax.swing.JPanel;

public class PropertiesPanel extends JPanel {

    private IPropertiesPresenter mPresenter;

    public void setPresenter(IPropertiesPresenter presenter) {
        mPresenter = presenter;
    }

    public void showNone() {
        removeAll();
        repaint();
        revalidate();
    }

    public void showPropertiesForNode(NodeModel node) {
        // TODO: every time a node is selected, we recreate the property panel for its
        // properties. We could probably cache this somewhere and show / hide it to avoid
        // re-adding components each time.
        removeAll();
        for (NodeProperty property : node.getProperties()) {
            add(property.getComponent());
            property.setPresenter(mPresenter);
        }
        repaint();
        revalidate();
    }
}
