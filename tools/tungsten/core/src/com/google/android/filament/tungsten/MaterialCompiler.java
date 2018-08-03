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

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.RandomAccessFile;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class MaterialCompiler {

    private final String mMatcPath;

    public MaterialCompiler(String matcPath) {
        mMatcPath = matcPath;
    }

    public Buffer compile(String source) throws MaterialCompilationException {
        try {
            File sourceFile = createTempFileForSource(source);
            File outputFile = createTempFileForOutput();
            executeCompileCommand(sourceFile, outputFile);
            return loadFileContent(outputFile.getAbsolutePath());
        } catch (IOException | InterruptedException e) {
            throw new MaterialCompilationException("Error: " + e.getMessage() + ". Could not " +
                    "compile material", e);
        }
    }

    private void executeCompileCommand(File sourceFile, File outputFile) throws
            IOException, InterruptedException {
        Process matcProcess = new ProcessBuilder(mMatcPath, "-o",
                outputFile.getAbsolutePath(), sourceFile.getAbsolutePath())
                .inheritIO()
                .start();
        matcProcess.waitFor();
    }

    private static File createTempFileForOutput() throws IOException {
        File compiledOutput = File.createTempFile("filament-material-editor-output", ".bmat");
        compiledOutput.deleteOnExit();
        return compiledOutput;
    }

    private static File createTempFileForSource(String source) throws IOException {
        File sourceFile = File.createTempFile("filament-material-editor-source", ".mat");
        sourceFile.deleteOnExit();
        PrintWriter sourceFileWriter = new PrintWriter(sourceFile);
        sourceFileWriter.println(source);
        sourceFileWriter.close();
        return sourceFile;
    }

    private static Buffer loadFileContent(String path) throws IOException {
        try (RandomAccessFile file = new RandomAccessFile(path, "r");
             FileChannel fileChannel = file.getChannel()) {

            int size = (int) file.length();

            ByteBuffer buf = ByteBuffer.allocate(size);
            int bytesRead = fileChannel.read(buf);
            while (bytesRead != 0) {
                bytesRead = fileChannel.read(buf);
            }
            buf.rewind();
            return buf;
        }
    }
}
