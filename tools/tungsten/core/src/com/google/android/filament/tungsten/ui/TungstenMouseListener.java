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

import java.awt.event.MouseEvent;

/**
 * TungstenMouseListener combines Swing's MouseListener and MouseMotionListener as well as adding
 * some helpful events, like mouseDragStarted and mouseDragEnded.
 *
 * An empty default implementation for each method is provided so classes that adhere are not
 * required to override every method.
 */
public interface TungstenMouseListener {

    default void mouseDragged(MouseEvent e) {

    }

    default void mouseDragStarted(MouseEvent e) {

    }

    default void mouseDragEnded(MouseEvent e) {

    }

    default void mouseMoved(MouseEvent e) {

    }

    default void mouseClicked(MouseEvent e) {

    }

    default void mousePressed(MouseEvent e) {

    }

    default void mouseReleased(MouseEvent e) {

    }

    default void mouseEntered(MouseEvent e) {

    }

    default void mouseExited(MouseEvent e) {

    }
}
