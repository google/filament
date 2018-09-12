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

import com.google.android.filament.Engine;
import com.google.android.filament.FilamentPanel;
import com.google.android.filament.Renderer;
import com.google.android.filament.View;
import com.google.android.filament.tungsten.texture.TextureCache;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;

public final class Filament {

    public static class Viewer {
        public Renderer renderer;
        public FilamentPanel panel;
        public View view;

        public void update(long deltaTimeMs) {

        }
    }

    private static class Task {
        final Consumer<Engine> runnable;
        final Object wait;

        public Task(Consumer<Engine> runnable, Object wait) {
            this.runnable = runnable;
            this.wait = wait;
        }
    }

    private static final Filament FILAMENT_HOLDER = new Filament();

    private Thread mFilamentThread;
    private Engine mEngine;
    private final AtomicBoolean mRunFilament = new AtomicBoolean(true);
    private final BlockingQueue<Task> mJobQueue = new LinkedBlockingQueue<>();
    private final List<Viewer> mViewList = new LinkedList<>();
    private long mPreviousTime = 0;

    private Filament() {

    }

    public static Filament getInstance() {
        return FILAMENT_HOLDER;
    }

    public void start() {
        startFilamentThread();
    }

    public void runOnFilamentThread(Consumer<Engine> c) {
        mJobQueue.add(new Task(c, null));
    }

    public void runImmediatelyOnFilamentThread(Consumer<Engine> c) throws InterruptedException {
        Object conditionObject = new Object();
        synchronized (conditionObject) {
            mJobQueue.add(new Task(c, conditionObject));
            conditionObject.wait();
        }
    }

    public void addViewer(Viewer viewer) {
        runOnFilamentThread((Engine e) -> mViewList.add(viewer));
    }

    public void removeViewer(Viewer viewer) {
        runOnFilamentThread((Engine e) -> mViewList.remove(viewer));
    }

    public void shutdown() {
        try {
            TextureCache.INSTANCE.shutdownAndDestroyTextures();
            runImmediatelyOnFilamentThread((Engine e) -> {
                // Before shutting down, drain the queue of any remaining jobs. Some jobs that deal
                // with cleanup might still remain and we want to ensure these all run.
                drainJobQueue();
                e.destroy();
                mRunFilament.set(false);
            });
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void startFilamentThread() {
        mFilamentThread = new Thread(() -> {
            mEngine = Engine.create();

            while (mRunFilament.get()) {
                // Run all of the outstanding jobs
                drainJobQueue();

                renderViews();

                try {
                    Thread.sleep(16);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        });
        mFilamentThread.setPriority(Thread.MAX_PRIORITY);
        mFilamentThread.start();
    }

    public void assertIsFilamentThread() {
        assert Thread.currentThread().equals(mFilamentThread);
    }

    private void drainJobQueue() {
        while (!mJobQueue.isEmpty()) {
            Task task = mJobQueue.poll();
            assert task != null;
            task.runnable.accept(mEngine);
            if (task.wait != null) {
                synchronized (task.wait) {
                    task.wait.notifyAll();
                }
            }
        }
    }

    private void renderViews() {
        long now = System.currentTimeMillis();
        for (Viewer viewer : mViewList) {
            // We need to wait until Swing has had a chance to layout the panel before we can
            // attempt to render to it. Otherwise it won't have a valid swapchain.
            if (viewer.panel.getHeight() == 0 || viewer.panel.getWidth() == 0) {
                continue;
            }
            if (!viewer.panel.beginFrame(mEngine, viewer.renderer)) {
                continue;
            }

            viewer.update(now - mPreviousTime);

            viewer.renderer.render(viewer.view);
            viewer.panel.endFrame(viewer.renderer);
        }
        mPreviousTime = System.currentTimeMillis();
    }
}
