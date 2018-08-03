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
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;

/**
 * TungstenEventManager marshals events from both the MouseListener and MouseMotionListener
 * interfaces to a single TungstenMouseListener.
 *
 * Classes should instantiate a TungsteneEventManager and add it as a both a MouseListener and
 * MouseMotionListener.
 */
class TungstenEventManager implements MouseListener, MouseMotionListener {

    private TungstenMouseListener mListener;
    private int mMousePreviousX;
    private int mMousePreviousY;
    private int mMouseDeltaX;
    private int mMouseDeltaY;
    private boolean mMouseIsDragging = false;

    TungstenEventManager(TungstenMouseListener listener) {
        mListener = listener;
    }

    int getMouseDeltaX() {
        return mMouseDeltaX;
    }

    int getMouseDeltaY() {
        return mMouseDeltaY;
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        mListener.mouseClicked(e);
    }

    @Override
    public void mousePressed(MouseEvent e) {
        mListener.mousePressed(e);
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        if (mMouseIsDragging) {
            mMouseIsDragging = false;
            mListener.mouseDragEnded(e);
        }
    }

    @Override
    public void mouseEntered(MouseEvent e) {
        mListener.mouseEntered(e);
    }

    @Override
    public void mouseExited(MouseEvent e) {
        mListener.mouseExited(e);
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        calculateMouseDelta(e);
        if (!mMouseIsDragging) {
            mMouseIsDragging = true;
            mListener.mouseDragStarted(e);
        }
        mListener.mouseDragged(e);
    }

    @Override
    public void mouseMoved(MouseEvent e) {
        calculateMouseDelta(e);
        mListener.mouseMoved(e);
    }

    private void calculateMouseDelta(MouseEvent e) {
        mMouseDeltaX = e.getX() - mMousePreviousX;
        mMouseDeltaY = e.getY() - mMousePreviousY;
        mMousePreviousX = e.getX();
        mMousePreviousY = e.getY();
    }
}
