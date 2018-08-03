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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import javax.annotation.Nonnull;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Helpers to load resources packaged inside of a jar files.
 */
class ResourceLoader {

    /**
     * Load a dynamic library from a resource residing on the classpath.
     * @param libName The portable library name, without any prefixes or extensions (i.e., library)
     *                This will automatically be converted to a platform-specific name, like
     *                liblibrary.dylib.
     */
    static void loadDynamicLibrary(@Nonnull String libName) throws IOException {
        String filePath = "/" + System.mapLibraryName(libName);

        InputStream input = ResourceLoader.class.getResourceAsStream(filePath);

        if (input == null) {
            throw new FileNotFoundException("Could not find library " + filePath);
        }

        File file = createTemporaryFile(libName, null);
        writeStreamToFile(input, file);
        System.load(file.getAbsolutePath());
    }

    /**
     * Get a File object from a resource residing on the classpath.
     */
    @NotNull
    public static File getResourceAsFile(@Nonnull String name, @Nonnull String extension,
            boolean executable) throws IOException {
        String filePath = "/" + name + extension;

        InputStream input = ResourceLoader.class.getResourceAsStream(filePath);

        if (input == null) {
            throw new FileNotFoundException("Could not find resource file " + filePath);
        }

        File file = createTemporaryFile(name, extension);
        if (!file.setExecutable(executable)) {
            throw new RuntimeException("Could not make file executable " + filePath);
        }
        writeStreamToFile(input, file);

        return file;
    }

    @NotNull
    private static File createTemporaryFile(@Nonnull String name, @Nullable String extension)
            throws IOException {
        File file = File.createTempFile(name, extension);
        file.deleteOnExit();
        return file;
    }

    private static void writeStreamToFile(@Nonnull InputStream input, @Nonnull File file)
            throws IOException {
        OutputStream out = new FileOutputStream(file);
        int read;
        byte[] bytes = new byte[4096];
        while ((read = input.read(bytes)) != -1) {
            out.write(bytes, 0, read);
        }
        out.flush();
        out.close();
    }
}
