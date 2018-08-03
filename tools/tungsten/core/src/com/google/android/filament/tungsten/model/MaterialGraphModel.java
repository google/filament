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

package com.google.android.filament.tungsten.model;

import java.util.ArrayList;
import java.util.List;

/**
 * The MaterialGraphModel stores canonical information about a material graph, its nodes, and their
 * connections.
 */
public class MaterialGraphModel {

    private final List<NodeModel> mNodes;
    private NodeModel mRootNode;
    private final List<ConnectionModel> mConnections;
    private SelectionModel mSelection;

    public MaterialGraphModel() {
        mNodes = new ArrayList<>();
        mConnections = new ArrayList<>();
    }

    public void setRootNode(NodeModel rootNode) {
        mRootNode = rootNode;
    }

    public NodeModel getRootNode() {
        return mRootNode;
    }

    public void addNode(NodeModel node) {
        mNodes.add(node);
    }

    public void createConnection(ConnectionModel connection) {
        mConnections.add(connection);
    }

    public void removeConnection(ConnectionModel connection) {
        mConnections.remove(connection);
    }

    public void setSelection(SelectionModel selection) {
        mSelection = selection;
    }

    public List<ConnectionModel> getConnections() {
        return mConnections;
    }

    public List<NodeModel> getNodes() {
        return mNodes;
    }
}
