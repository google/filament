//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowTest.cpp: Sample used to test various function of OSWindow

#include <algorithm>
#include <iostream>

#include "util/OSWindow.h"
#include "util/test_utils.h"

int main(int argc, char *argv[])
{
    OSWindow *window = OSWindow::New();
    int width        = 400;
    int height       = 400;
    int x            = 0;
    int y            = 0;

    if (!window->initialize("Window Test", width, height))
    {
        return -1;
    }
    window->setVisible(true);
    window->setPosition(x, y);

    bool running = true;
    while (running)
    {
        Event event;
        while (window->popEvent(&event))
        {
            if (event.Type == Event::EVENT_CLOSED)
            {
                running = false;
                break;
            }

            if (event.Type == Event::EVENT_KEY_PRESSED)
            {
                int newWidth  = width;
                int newHeight = height;
                int newX      = x;
                int newY      = y;
                switch (event.Key.Code)
                {
                    case KEY_ESCAPE:
                        running = false;
                        break;

                    case KEY_W:
                        newWidth = std::max(0, width + (event.Key.Shift ? -20 : 20));
                        break;
                    case KEY_H:
                        newHeight = std::max(0, height + (event.Key.Shift ? -20 : 20));
                        break;

                    case KEY_LEFT:
                        newX = x - 20;
                        break;
                    case KEY_RIGHT:
                        newX = x + 20;
                        break;
                    case KEY_UP:
                        newY = y - 20;
                        break;
                    case KEY_DOWN:
                        newY = y + 20;
                        break;

                    case KEY_C:
                        window->setMousePosition(width / 2, height / 2);
                        break;
                    case KEY_T:
                        window->signalTestEvent();
                        window->messageLoop();
                        if (window->didTestEventFire())
                        {
                            std::cout << "Test event did fire" << std::endl;
                        }
                        else
                        {
                            std::cout << "Test event did not fire" << std::endl;
                        }
                        break;
                    case KEY_S:
                        window->setVisible(false);
                        window->messageLoop();
                        angle::Sleep(1000);
                        window->setVisible(true);
                        window->messageLoop();
                        break;

                    default:
                        break;
                }

                if (newWidth != width || newHeight != height)
                {
                    width  = newWidth;
                    height = newHeight;
                    window->resize(width, height);
                }
                if (newX != x || newY != y)
                {
                    x = newX;
                    y = newY;
                    window->setPosition(x, y);
                }

                angle::Sleep(0);
                window->messageLoop();
                if (window->getWidth() != width || window->getHeight() != height)
                {
                    std::cout << "Discrepancy between set dimensions and retrieved dimensions"
                              << std::endl;
                    std::cout << "Width: " << width << " vs. " << window->getWidth() << std::endl;
                    std::cout << "Height: " << height << " vs. " << window->getHeight()
                              << std::endl;
                }
                if (window->getX() != x || window->getY() != y)
                {
                    std::cout << "Discrepancy between set position and retrieved position"
                              << std::endl;
                    std::cout << "X: " << x << " vs. " << window->getX() << std::endl;
                    std::cout << "Y: " << y << " vs. " << window->getY() << std::endl;
                }
            }
        }

        angle::Sleep(0);
        window->messageLoop();
    }

    window->destroy();
}
