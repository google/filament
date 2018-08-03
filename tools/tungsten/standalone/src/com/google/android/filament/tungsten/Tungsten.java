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

import com.google.android.filament.tungsten.ui.TungstenPanel;

import com.google.android.filament.tungsten.util.OperatingSystem;
import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;

public class Tungsten {

    public static void main(String[] args) {

        String matcPath;
        if (OperatingSystem.isWindows()) {
            matcPath = "../../../dist/bin/matc.exe";
        } else {
            matcPath = "../../../dist/bin/matc";
        }
        MaterialManager materialManager = new MaterialManager(matcPath);

        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                System.loadLibrary("filament-jni");
                Filament.getInstance().start();

                TungstenPanel panel = createTungstenFrameAndPanel();

                Runtime.getRuntime().addShutdownHook(new Thread(() -> {
                    panel.destroy();
                    Filament.getInstance().shutdown();
                }));
            }

            private TungstenPanel createTungstenFrameAndPanel() {
                JFrame tungstenFrame = new JFrame();

                TungstenPanel tungstenPanel = new TungstenPanel(new SwingComponentFactory(),
                        materialManager, new InMemoryFile());
                tungstenFrame.setTitle("Tungsten");
                tungstenFrame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
                tungstenFrame.add(tungstenPanel);

                Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
                tungstenFrame.setSize(screenSize);
                tungstenFrame.setLocationRelativeTo(null);

                tungstenFrame.setVisible(true);

                return tungstenPanel;
            }
        });
    }
}
