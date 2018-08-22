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

import com.google.android.filament.Material;
import com.google.android.filament.MaterialInstance;
import com.google.android.filament.tungsten.MaterialManager;
import com.google.android.filament.tungsten.compiler.GraphCompiler;
import com.google.android.filament.tungsten.compiler.NodeRegistry;
import com.google.android.filament.tungsten.model.Connection;
import com.google.android.filament.tungsten.model.Graph;
import com.google.android.filament.tungsten.model.GraphInitializer;
import com.google.android.filament.tungsten.model.Node;
import com.google.android.filament.tungsten.model.PropertyValue;
import com.google.android.filament.tungsten.model.serialization.GraphFile;
import com.google.android.filament.tungsten.model.serialization.GraphSerializer;
import com.google.android.filament.tungsten.model.serialization.JsonDeserializer;
import com.google.android.filament.tungsten.model.serialization.JsonSerializer;
import com.google.android.filament.tungsten.properties.IPropertiesPresenter;
import com.google.android.filament.tungsten.properties.PropertiesPanel;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import javax.swing.JTextArea;

public class GraphPresenter implements IPropertiesPresenter {

    // Views
    private Graph mModel;
    private final MaterialGraphComponent mGraphView;
    private final PreviewMeshPanel mPreviewMeshPanel;
    private final JTextArea mMaterialSource;
    private final PropertiesPanel mPropertiesPanel;

    // Dependencies
    private final NodeRegistry mNodeRegistry;
    private final MaterialManager mMaterialManager;
    private final TungstenFile mFile;

    private String mCompiledGraph;

    public GraphPresenter(Graph model, MaterialGraphComponent graphView,
            PreviewMeshPanel previewMeshPanel, JTextArea materialSource,
            PropertiesPanel propertiesPanel, MaterialManager materialManager, TungstenFile file) {
        mModel = model;
        mGraphView = graphView;
        mPreviewMeshPanel = previewMeshPanel;
        mMaterialSource = materialSource;
        mPropertiesPanel = propertiesPanel;
        mMaterialManager = materialManager;
        mNodeRegistry = new NodeRegistry();
        mFile = file;

        loadModelFromFile(file);
    }

    /**
     * Action from the view that lets us know a connection between two nodes has been created.
     */
    public void connectionCreated(Connection connection) {
        mModel = mModel.graphByFormingConnection(connection);
        mGraphView.render(mModel);
        recompileGraph();
    }

    /**
     * Action from the view that lets us know a connection between two nodes has been removed.
     */
    public void connectionDisconnectedAtInput(Connection connection) {
        mModel = mModel.graphByRemovingConnection(connection);
        mGraphView.render(mModel);
        recompileGraph();
    }

    /**
     * Action from the view that lets us know the selection of nodes has changed.
     */
    public void selectionChanged(List<Integer> selectedNodes) {
        mModel = mModel.graphByChangingSelection(selectedNodes);
        mGraphView.render(mModel);
        if (selectedNodes.size() != 1) {
            mPropertiesPanel.showNone();
            return;
        }
        mPropertiesPanel.showPropertiesForNode(mModel.getNodeWithId(selectedNodes.get(0)));
    }

    /**
     * Action from the view that lets us know a node has been dragged to a new location.
     */
    public void nodeMoved(Node node, int x, int y) {
        mModel = mModel.graphByMovingNode(node, x, y);
        mGraphView.render(mModel);
        serializeAndSave();
    }

    /**
     * Action from the view that lets us know a NodeProperty's value has changed.
     */
    @Override
    public void propertyChanged(int nodeId, String propertyName, PropertyValue value) {
        mModel = mModel.graphByChangingProperty(nodeId, propertyName, value);
        mGraphView.render(mModel);
        mPropertiesPanel.showPropertiesForNode(mModel.getSelectedNodes().get(0));
        recompileGraph();
    }

    /**
     * Action from the popup menu that lets us know an item has been clicked.
     */
    public void popupMenuItemSelected(String name, int x, int y) {
        Node newNode = mNodeRegistry.createNodeForLabel(name, mModel.getNewNodeId());
        if (newNode == null) {
            return;
        }
        mModel = mModel.graphByAddingNodeAtLocation(newNode, x, y);
        serializeAndSave();
        mGraphView.render(mModel);
    }

    private void loadModelFromFile(TungstenFile file) {
        file.read((success, contents) -> {
            if (!success) {
                // TODO: propagate this error to the user somehow in the UI
                System.out.println("Unable to read graph file.");
                return;
            }
            String toolBlock = GraphFile.INSTANCE.extractToolBlockFromMaterialFile(contents);
            if (toolBlock.isEmpty()) {
                mModel = GraphInitializer.INSTANCE.getInitialGraphState();
                recompileGraph();
                mGraphView.render(mModel);
                return;
            }
            mModel = GraphSerializer.INSTANCE.deserialize(
                    toolBlock, mNodeRegistry, new JsonDeserializer()).getFirst();
            mGraphView.render(mModel);
            recompileGraph();
        });
    }

    private void recompileGraph() {
        if (mModel.getRootNode() == null) {
            return;
        }
        GraphCompiler compiler = new GraphCompiler(mModel);
        mCompiledGraph = compiler.compileGraph();
        mMaterialSource.setText(mCompiledGraph);

        CompletableFuture<Material> futureMaterial = mMaterialManager
                .compileMaterial(mCompiledGraph);

        serializeAndSave();

        futureMaterial.thenAccept(newMaterial -> {
            // The returned future will always be completed on the main thread, so
            // we're safe to call filament methods here.
            MaterialInstance newMaterialInstance = newMaterial.createInstance();
            mPreviewMeshPanel.updateMaterial(newMaterialInstance);
        });
    }

    private void serializeAndSave() {
        if (mCompiledGraph == null) {
            return;
        }
        String serializedGraph = GraphSerializer.INSTANCE.serialize(mModel, Collections.emptyMap(),
                new JsonSerializer());
        String materialDefinition =
                GraphFile.INSTANCE.addToolBlockToMaterialFile(mCompiledGraph, serializedGraph);
        mFile.write(materialDefinition, success -> {
            if (!success) {
                // TODO: propagate this error to the user somehow in the UI
                System.out.println("Unable to write graph file.");
            }
        });
    }
}
