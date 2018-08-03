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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.List;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

public class GraphPopup implements MouseListener, ActionListener {

    private GraphPresenter mPresenter;
    private final JPopupMenu mMenu;
    private int mPopupXLocation;
    private int mPopupYLocation;

    /**
     * Adds a Popup menu and calls actions in the presenter when items are selected.
     * @param component The component to add the popup menu to.
     */
    public GraphPopup(JComponent component, List<String> items) {
        // Create a menu and for each node registered in NodeRegistry, create a corresponding item
        mMenu = new JPopupMenu();
        for (String nodeName : items) {
            JMenuItem menuItem = new JMenuItem(nodeName);
            mMenu.add(menuItem);
            menuItem.addActionListener(this);
        }

        component.addMouseListener(this);
    }

    public void setPresenter(GraphPresenter presenter) {
        mPresenter = presenter;
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        checkAndTriggerPopup(e);
    }

    @Override
    public void mousePressed(MouseEvent e) {
        checkAndTriggerPopup(e);
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        checkAndTriggerPopup(e);
    }

    @Override
    public void mouseEntered(MouseEvent e) {

    }

    @Override
    public void mouseExited(MouseEvent e) {

    }

    private void checkAndTriggerPopup(MouseEvent e) {
        if (e.isPopupTrigger()) {
            mMenu.show(e.getComponent(), e.getX(), e.getY());
            mPopupXLocation = e.getX();
            mPopupYLocation = e.getY();
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        // The action command is the text displayed on the JMenuItem
        String nodeName = e.getActionCommand();
        mPresenter.popupMenuItemSelected(nodeName, mPopupXLocation, mPopupYLocation);
    }
}
