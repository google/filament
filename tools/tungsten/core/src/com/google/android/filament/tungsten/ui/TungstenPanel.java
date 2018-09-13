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

import com.google.android.filament.tungsten.MaterialManager;
import com.google.android.filament.tungsten.compiler.NodeRegistry;
import com.google.android.filament.tungsten.properties.PropertiesPanel;
import com.google.android.filament.tungsten.ui.preview.PreviewMeshPanel;

import java.awt.BorderLayout;
import java.awt.Dimension;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;

public class TungstenPanel extends JPanel {

    private PreviewMeshPanel mPreviewMeshPanel;

    public TungstenPanel(GuiComponentFactory componentFactory, MaterialManager materialManager,
            TungstenFile file) {

        BorderLayout panelLayout = new BorderLayout();
        setLayout(panelLayout);

        mPreviewMeshPanel = new PreviewMeshPanel();
        mPreviewMeshPanel.setMinimumSize(new Dimension(0, 300));

        JTextArea materialSource = new JTextArea();
        materialSource.setEditable(false);

        MaterialGraphComponent materialGraph = new MaterialGraphComponent();

        PropertiesPanel propertiesPanel = new PropertiesPanel();

        propertiesPanel.setMinimumSize(new Dimension(500, 200));

        NodeRegistry registry = new NodeRegistry();
        GraphPopup popup = new GraphPopup(materialGraph, registry.getNodeLabelsForMenu());

        JTabbedPane editor = new JTabbedPane();
        editor.addTab("Graph", materialGraph);
        editor.addTab("Source", materialSource);

        GraphPresenter presenter = new GraphPresenter(materialGraph, mPreviewMeshPanel,
                materialSource, propertiesPanel, materialManager, file);
        materialGraph.setPresenter(presenter);
        propertiesPanel.setPresenter(presenter);
        popup.setPresenter(presenter);

        add(componentFactory.createThreeComponentSplitLayout(editor, mPreviewMeshPanel,
                propertiesPanel));
    }

    public void destroy() {
        mPreviewMeshPanel.destroy();
    }
}
