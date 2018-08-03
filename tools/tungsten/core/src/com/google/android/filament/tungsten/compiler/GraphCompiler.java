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

package com.google.android.filament.tungsten.compiler;

import com.google.android.filament.tungsten.model.NodeModel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * GraphCompiler takes a a graph of connected NodeModels and generates filament shader code.
 */
public class GraphCompiler {

    private int mTextureNumber;
    private StringBuilder mMaterialFunctionBodyBuilder;
    private final List<String> mRequiredAttributes;
    private final List<Parameter> mParameters;
    private final LinkedHashMap<String, String> mGlobalFunctions;
    private final Map<String, Integer> mVariableNameMap;

    public GraphCompiler() {
        mMaterialFunctionBodyBuilder = new StringBuilder();
        mRequiredAttributes = new ArrayList<>();
        mParameters = new ArrayList<>();
        mGlobalFunctions = new LinkedHashMap<>();
        mVariableNameMap = new HashMap<>();
        reset();
    }

    public String compileGraph(NodeModel rootNode) {
        rootNode.compile(this);
        String fragmentSection = GraphFormatter.formatFragmentSection(mGlobalFunctions.values(),
                mMaterialFunctionBodyBuilder.toString());
        String finalResult = GraphFormatter.formatMaterialSection(mRequiredAttributes, mParameters)
                + fragmentSection;
        invalidate(rootNode);
        return finalResult;
    }

    /**
     * Called by a NodeModel subclass to denote that the given vertex attribute is required.
     * The attribute will be added to the material source's "requires" section.
     *
     * @param requirement The name of the vertex attribute, such as "uv0"
     */
    public void requireAttribute(String requirement) {
        if (!mRequiredAttributes.contains(requirement)) {
            mRequiredAttributes.add(requirement);
        }
    }

    /**
     * Called by a NodeModel subclass to add a new material parameter.
     * The parameter will be added to the material source's "parameters" section.
     *
     * @param type The type of parameter
     * @return A String representing the name of the parameter variable that the node should use
     * in its source.
     */
    public String addParameter(String type) {
        String parameterName = allocateNewParameterName();
        mParameters.add(new Parameter(type, parameterName));
        // todo: Handle parameters other than sampler parameters
        return "materialParams_" + parameterName;
    }

    /**
     * Called by a NodeModel subclass to allocate a new temporary variable for its use.
     * @return The symbol name of the variable.
     */
    public String getNewTemporaryVariableName(String name) {
        int iterationNumber = mVariableNameMap.getOrDefault(name, 0);
        String result = name + iterationNumber;
        mVariableNameMap.put(name, ++iterationNumber);
        return result;
    }

    /**
     * Called by a NodeModel subclass to add code to the material function's body.
     * @param code code to be concatenated to the material function body.
     */
    public void addCodeToMaterialFunctionBody(String code) {
        mMaterialFunctionBodyBuilder.append(code);
    }

    /**
     * Convenience method for calling addCodeToMaterialFunctionBody with a format String and
     * arguments.
     */
    public void addCodeToMaterialFunctionBody(String format, Object... args) {
        addCodeToMaterialFunctionBody(String.format(format, args));
    }

    /**
     * Called by a NodeModel to add a global function to the material code.
     * @param requestedName The name the Node wishes to use for the function.
     * @return The resulting symbol name for the global function. The NodeModel should use this name
     * when supplying the function definition and for references to the function in shader code.
     */
    public String allocateGlobalFunction(String requestedName, String scope) {
        // Mangle the function name
        String mangledName = requestedName + "_" + scope;
        if (!mGlobalFunctions.containsKey(mangledName)) {
            mGlobalFunctions.put(mangledName, "");
        }
        return mangledName;
    }

    /**
     * Called by a NodeModule to provide a definition for a previously allocated global function.
     * @param symbolName The name of the function symbol returned by allocateGlobalFunction.
     * @param definition The complete source text of the function, using symbolName.
     */
    public void provideFunctionDefinition(String symbolName, String definition) {
        mGlobalFunctions.put(symbolName, definition);
    }

    /**
     * Convenience method for providing a function definition that uses a format String and
     * arguments.
     */
    public void provideFunctionDefinition(String symbolName, String format, Object... args) {
        provideFunctionDefinition(symbolName, String.format(format, args));
    }

    private String allocateNewParameterName() {
        return "texture" + mTextureNumber++;
    }

    private void invalidate(NodeModel rootNode) {
        rootNode.invalidate();
        reset();
    }

    private void reset() {
        mTextureNumber = 1;
        mRequiredAttributes.clear();
        mParameters.clear();
        mGlobalFunctions.clear();
        mVariableNameMap.clear();
        mMaterialFunctionBodyBuilder.setLength(0);
    }
}
