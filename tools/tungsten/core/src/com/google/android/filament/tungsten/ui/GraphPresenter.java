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
import com.google.android.filament.tungsten.compiler.ShaderNode;
import com.google.android.filament.tungsten.model.ConnectionModel;
import com.google.android.filament.tungsten.model.MaterialGraphModel;
import com.google.android.filament.tungsten.model.NodeModel;
import com.google.android.filament.tungsten.model.SelectionModel;
import com.google.android.filament.tungsten.model.serialization.GraphFile;
import com.google.android.filament.tungsten.model.serialization.GraphSerializer;
import com.google.android.filament.tungsten.model.serialization.JsonDeserializer;
import com.google.android.filament.tungsten.model.serialization.JsonSerializer;
import com.google.android.filament.tungsten.properties.IPropertiesPresenter;
import com.google.android.filament.tungsten.properties.PropertiesPanel;
import java.util.Collections;
import java.util.concurrent.CompletableFuture;
import java.util.function.Consumer;
import java.util.regex.Pattern;
import javax.swing.JTextArea;

public class GraphPresenter implements IPropertiesPresenter {

    // Views
    private MaterialGraphModel mModel;
    private final MaterialGraphComponent mGraphView;
    private final PreviewMeshPanel mPreviewMeshPanel;
    private final JTextArea mMaterialSource;
    private final PropertiesPanel mPropertiesPanel;

    // Dependencies
    private final GraphCompiler mGraphCompiler;
    private final NodeRegistry mNodeRegistry;
    private final MaterialManager mMaterialManager;
    private final TungstenFile mFile;

    private String mCompiledGraph;

    public GraphPresenter(MaterialGraphModel model, MaterialGraphComponent graphView,
            PreviewMeshPanel previewMeshPanel, JTextArea materialSource,
            PropertiesPanel propertiesPanel, MaterialManager materialManager, TungstenFile file) {
        mModel = model;
        mGraphView = graphView;
        mPreviewMeshPanel = previewMeshPanel;
        mMaterialSource = materialSource;
        mPropertiesPanel = propertiesPanel;
        mGraphCompiler = new GraphCompiler();
        mMaterialManager = materialManager;
        mNodeRegistry = new NodeRegistry();
        mFile = file;

        loadModelFromFile(file);
    }

    /**
     * Action from the view that lets us know a connection between two nodes has been created.
     */
    public void connectionCreated(ConnectionModel connection) {
        mModel.createConnection(connection);
        mGraphView.addConnection(connection);
        connection.getInputSlot().connectTo(connection.getOutputSlot());
        recompileGraph();
    }

    /**
     * Action from the view that lets us know a connection between two nodes has been removed.
     */
    public void connectionDisconnectedAtInput(ConnectionModel connection) {
        mModel.removeConnection(connection);
        mGraphView.disconnectConnectionAtInput(connection);
        connection.getInputSlot().removeConnection();
        recompileGraph();
    }

    /**
     * Action from the view that lets us know the selection of nodes has changed.
     */
    public void selectionChanged(SelectionModel selection) {
        mModel.setSelection(selection);
        if (selection.getSelectionCount() != 1) {
            mPropertiesPanel.showNone();
            return;
        }
        mPropertiesPanel.showPropertiesForNode(selection.getNodes().get(0));
    }

    /**
     * Action from the view that lets us know a node has been dragged to a new location.
     */
    public void nodeMoved(NodeModel node, int x, int y) {
        node.setPosition(x, y);
        serializeAndSave();
    }

    /**
     * Action from the view that lets us know a NodeProperty's value has changed.
     */
    @Override
    public void propertyChanged() {
        recompileGraph();
    }

    /**
     * Action from the popup menu that lets us know an item has been clicked.
     */
    public void popupMenuItemSelected(String name, int x, int y) {
        NodeModel newNode = mNodeRegistry.createNodeForLabel(name);
        newNode.setPosition(x, y);
        mModel.addNode(newNode);
        mGraphView.addNode(newNode);
        serializeAndSave();
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
                mModel = new MaterialGraphModel();
                ShaderNode shaderNode = new ShaderNode();
                mModel.addNode(shaderNode);
                mModel.setRootNode(shaderNode);
                addInitialNodes();
                recompileGraph();
                return;
            }
            mModel = GraphSerializer.INSTANCE.deserialize(
                    toolBlock, mNodeRegistry, new JsonDeserializer()).getFirst();
            addInitialNodes();
            recompileGraph();
        });
    }

    private void addInitialNodes() {
        for (NodeModel node : mModel.getNodes()) {
            mGraphView.addNode(node);
        }
        for (ConnectionModel connection : mModel.getConnections()) {
            mGraphView.addConnection(connection);
        }
    }


    private void recompileGraph() {
        if (mModel.getRootNode() == null) {
            return;
        }
        mCompiledGraph = mGraphCompiler.compileGraph(mModel.getRootNode());
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
        String serializedGraph = GraphSerializer.INSTANCE.serialize(
                mModel, Collections.emptyMap(), mNodeRegistry, new JsonSerializer());
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
