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

import java.util.Collection;
import java.util.Collections;

final class GraphFormatter {

    // Amount of spaces global functions are indented by
    private static final int FUNCTION_INDENT_AMOUNT = 4;

    // Amount of spaces that the material function body is indented by
    private static final int BODY_CODE_INDENT_AMOUNT = 8;

    private GraphFormatter() { }

    static String formatFragmentSection(Collection<String> globalFunctions,
            String materialFunctionBodyPrologue, String materialFunctionBody) {
        StringBuilder functions = new StringBuilder();
        for (String functionBody : globalFunctions) {
            functions.append(indent(functionBody, FUNCTION_INDENT_AMOUNT));
            functions.append("\n");
        }
        return "fragment {\n"
                + functions.toString()
                + "    void material(inout MaterialInputs material) {\n"
                + formatMaterialFunctionBody(materialFunctionBodyPrologue ,materialFunctionBody)
                + "    }\n"
                + "}";
    }

    static String formatMaterialSection(Collection<String> attributes,
            Collection<Parameter> parameters, String shadingModel) {
        StringBuilder builder = new StringBuilder();
        for (String attribute : attributes) {
            builder.append(indent(attribute, BODY_CODE_INDENT_AMOUNT)).append("\n");
        }
        String attributeText = builder.toString();
        String parameterText = formatParameters(parameters);
        return "material {\n"
                + "    name : \"\",\n"
                + parameterText
                + "    requires : [\n"
                + attributeText
                + "    ],\n"
                + "    shadingModel : \"" + shadingModel + "\"\n"
                + "}\n"
                + "\n";
    }

    /**
     * Indents each line of a string by amount number of spaces.
     */
    static String indent(String s, int amount) {
        if (s == null || s.isEmpty()) {
            return "";
        }
        String spaces = String.join("", Collections.nCopies(amount, " "));
        String t = s.replaceAll("^", spaces);   // indent the first line
        return t.replaceAll("\n(?!$)", "\n" + spaces);  // indent remaining non-empty lines
    }

    private static String formatMaterialFunctionBody(String prologue, String epilogue) {
        return indent(prologue, BODY_CODE_INDENT_AMOUNT)
                + indent("prepareMaterial(material);\n", BODY_CODE_INDENT_AMOUNT)
                + indent(epilogue, BODY_CODE_INDENT_AMOUNT);
    }

    private static String formatParameters(Collection<Parameter> parameters) {
        if (parameters.isEmpty()) {
            return "";
        }
        StringBuilder builder = new StringBuilder("parameters : [\n");
        int i = 0;
        for (Parameter parameter : parameters) {
            builder.append(formatParameterSource(parameter));
            if (i++ < parameters.size() - 1) {
                builder.append(",");
            }
            builder.append("\n");
        }
        builder.append("],\n");
        return indent(builder.toString(), 4);
    }

    private static String formatParameterSource(Parameter p) {
        return indent("{\n"
                + "    type : " + p.getType() + ",\n"
                + "    name : " + p.getName() + "\n"
                + "}", 4);
    }
}
