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
import com.google.android.filament.tungsten.Filament;
import com.google.android.filament.tungsten.MaterialManager;
import com.google.android.filament.tungsten.compiler.CompiledGraph;
import com.google.android.filament.tungsten.compiler.GraphCompiler;
import com.google.android.filament.tungsten.compiler.NodeRegistry;
import com.google.android.filament.tungsten.compiler.Parameter;
import com.google.android.filament.tungsten.model.Connection;
import com.google.android.filament.tungsten.model.Graph;
import com.google.android.filament.tungsten.model.GraphInitializer;
import com.google.android.filament.tungsten.model.Node;
import com.google.android.filament.tungsten.model.Property;
import com.google.android.filament.tungsten.model.PropertyType;
import com.google.android.filament.tungsten.model.serialization.GraphFile;
import com.google.android.filament.tungsten.model.serialization.GraphSerializer;
import com.google.android.filament.tungsten.model.serialization.JsonDeserializer;
import com.google.android.filament.tungsten.model.serialization.JsonSerializer;
import com.google.android.filament.tungsten.properties.IPropertiesPresenter;
import com.google.android.filament.tungsten.properties.PropertiesPanel;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.atomic.AtomicReference;
import javax.swing.JTextArea;

import com.google.android.filament.tungsten.ui.preview.PreviewMeshPanel;
import org.jetbrains.annotations.Nullable;

public class GraphPresenter implements IPropertiesPresenter {

    // Views
    private final MaterialGraphComponent mGraphView;
    private final PreviewMeshPanel mPreviewMeshPanel;
    private final JTextArea mMaterialSource;
    private final PropertiesPanel mPropertiesPanel;

    // Dependencies
    private final NodeRegistry mNodeRegistry;
    private final MaterialManager mMaterialManager;
    private final TungstenFile mFile;

    private CompiledGraph mCompiledGraph;

    // Technically volatile is enough here b/c the Filament thread only ever reads
    private AtomicReference<Graph> mModel = new AtomicReference<>();

    // Only accessed from Filament thread
    @Nullable private MaterialInstance mCurrentMaterialInstance = null;

    public GraphPresenter(MaterialGraphComponent graphView, PreviewMeshPanel previewMeshPanel,
            JTextArea materialSource, PropertiesPanel propertiesPanel,
            MaterialManager materialManager, TungstenFile file) {
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
        mModel.getAndUpdate(graph -> graph.graphByFormingConnection(connection));
        mGraphView.render(mModel.get());
        recompileGraph();
    }

    /**
     * Action from the view that lets us know a connection between two nodes has been removed.
     */
    public void connectionDisconnectedAtInput(Connection connection) {
        mModel.getAndUpdate(graph -> graph.graphByRemovingConnection(connection));
        mGraphView.render(mModel.get());
        recompileGraph();
    }

    /**
     * Action from the view that lets us know the selection of nodes has changed.
     */
    public void selectionChanged(List<Integer> selectedNodes) {
        mModel.getAndUpdate(graph -> graph.graphByChangingSelection(selectedNodes));
        mGraphView.render(mModel.get());
        if (selectedNodes.size() != 1) {
            mPropertiesPanel.showNone();
            return;
        }
        mPropertiesPanel.showPropertiesForNode(mModel.get().getNodeWithId(selectedNodes.get(0)));
    }

    /**
     * Action from the view that lets us know a node has been dragged to a new location.
     */
    public void nodeMoved(Node node, int x, int y) {
        mModel.getAndUpdate(graph -> graph.graphByMovingNode(node, x, y));
        mGraphView.render(mModel.get());
        serializeAndSave();
    }

    /**
     * Action from the view that lets us know a NodeProperty's value has changed.
     */
    @Override
    public void propertyChanged(Node.PropertyHandle handle, Property property) {
        mModel.getAndUpdate(graph -> graph.graphByChangingProperty(handle, property));
        mGraphView.render(mModel.get());
        mPropertiesPanel.showPropertiesForNode(mModel.get().getSelectedNodes().get(0));

        // Graph property changes trigger a full recompilation of the graph, while material
        // parameter changes trigger only a material parameter update on the current material
        // instance.
        if (property.getType() == PropertyType.GRAPH_PROPERTY) {
            recompileGraph();
        } else if (property.getType() == PropertyType.MATERIAL_PARAMETER) {
            serializeAndSave();
            Parameter parameter = mCompiledGraph.getParameterMap().get(handle);
            if (parameter == null) {
                return;
            }
            Filament.getInstance().runOnFilamentThread(engine -> {
                if (mCurrentMaterialInstance == null) {
                    return;
                }
                // This check prevents us from setting a property on a newer material instance that
                // may no longer have that property.
                if (mCurrentMaterialInstance.getMaterial().hasParameter(parameter.getName())) {
                    property.getValue().applyToMaterialInstance(mCurrentMaterialInstance,
                            parameter.getName());
                }
            });
        }
    }

    /**
     * Action from the popup menu that lets us know an item has been clicked.
     */
    public void popupMenuItemSelected(String name, int x, int y) {
        Node newNode = mNodeRegistry.createNodeForLabel(name, mModel.get().getNewNodeId());
        if (newNode == null) {
            return;
        }
        mModel.getAndUpdate(graph -> graph.graphByAddingNodeAtLocation(newNode, x, y));
        recompileGraph();
        mGraphView.render(mModel.get());
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
                mModel.set(GraphInitializer.INSTANCE.getInitialGraphState());
                recompileGraph();
                mGraphView.render(mModel.get());
                return;
            }
            mModel.set(GraphSerializer.INSTANCE.deserialize(
                    toolBlock, mNodeRegistry, new JsonDeserializer()).getFirst());
            mGraphView.render(mModel.get());
            recompileGraph();
        });
    }

    private void recompileGraph() {
        if (mModel.get().getRootNode() == null) {
            return;
        }
        GraphCompiler compiler = new GraphCompiler(mModel.get());
        mCompiledGraph = compiler.compileGraph();
        mMaterialSource.setText(mCompiledGraph.getMaterialDefinition());

        CompletableFuture<Material> futureMaterial = mMaterialManager
                .compileMaterial(mCompiledGraph.getMaterialDefinition());

        // Update the model with the newly compiled expression map and replace any modified nodes.
        mModel.getAndUpdate(
                graph -> graph
                    .graphBySettingExpressionMap(mCompiledGraph.getExpressionMap())
                    .graphByReplacingNodes(mCompiledGraph.getOldToNewNodeMap()));

        mGraphView.render(mModel.get());

        serializeAndSave();

        final CompiledGraph compiledGraph = mCompiledGraph;
        futureMaterial.thenAccept(newMaterial -> {
            Filament.getInstance().assertIsFilamentThread();

            mCurrentMaterialInstance = newMaterial.createInstance();
            mPreviewMeshPanel.updateMaterial(mCurrentMaterialInstance);

            // For all parameters present in the compiled graph, update them based on the current
            // value in our model.
            for (Map.Entry<Node.PropertyHandle, Parameter> entry
                    : compiledGraph.getParameterMap().entrySet()) {

                Node.PropertyHandle handle = entry.getKey();
                Parameter parameter = entry.getValue();

                Property nodeProperty = mModel.get().getNodeProperty(handle);
                if (nodeProperty == null) {
                    continue;
                }
                if (mCurrentMaterialInstance.getMaterial().hasParameter(parameter.getName())) {
                    nodeProperty.getValue()
                            .applyToMaterialInstance(mCurrentMaterialInstance, parameter.getName());
                }
            }
        });
    }

    private void serializeAndSave() {
        if (mCompiledGraph == null) {
            return;
        }
        String serializedGraph = GraphSerializer.INSTANCE.serialize(mModel.get(),
                Collections.emptyMap(), new JsonSerializer());
        String materialDefinition =
                GraphFile.INSTANCE.addToolBlockToMaterialFile(
                mCompiledGraph.getMaterialDefinition(), serializedGraph);
        mFile.write(materialDefinition, success -> {
            if (!success) {
                // TODO: propagate this error to the user somehow in the UI
                System.out.println("Unable to write graph file.");
            }
        });
    }
}
