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

package com.google.android.filament.tungsten.ui.preview;

import com.google.android.filament.Box;
import com.google.android.filament.Camera;
import com.google.android.filament.Engine;
import com.google.android.filament.EntityManager;
import com.google.android.filament.FilamentPanel;
import com.google.android.filament.IndexBuffer;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.MaterialInstance;
import com.google.android.filament.RenderableManager;
import com.google.android.filament.Renderer;
import com.google.android.filament.Scene;
import com.google.android.filament.VertexBuffer;
import com.google.android.filament.View;
import com.google.android.filament.Viewport;
import com.google.android.filament.filamesh.Filamesh;
import com.google.android.filament.filamesh.FilameshLoader;
import com.google.android.filament.tungsten.Filament;
import com.google.android.filament.tungsten.MathUtils;

import java.awt.BorderLayout;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

public class PreviewMeshPanel extends JPanel {

    // Filament resources
    private Renderer mRenderer;
    private Scene mScene;
    private Camera mCamera;
    private View mView;
    private FilamentPanel mFilamentPanel;
    private VertexBuffer mVertexBuffer;
    private IndexBuffer mIndexBuffer;
    private IndirectLight mIndirectLight;

    private int mMeshRenderable;
    private int mMeshEntity;
    private int mMeshTransform;
    private int mSun;
    private int mPointLight;

    private PreviewCamera mPreviewCamera;
    private Filament.Viewer mViewer;

    public PreviewMeshPanel() {
        // On Windows D3D seems to mess with our OpenGLContext, this disable it.
        System.setProperty("sun.java2d.d3d", "false");

        // Avoid flickering on X11 (Linux).
        System.setProperty("sun.awt.noerasebackground", "true");
        System.setProperty("sun.awt.erasebackgroundonresize", "false");

        setLayout(new BorderLayout());

        Filament.getInstance().runOnFilamentThread((Engine engine) -> {
            mScene = engine.createScene();
            mCamera = engine.createCamera(EntityManager.get().create());
            mView = engine.createView();
            mRenderer = engine.createRenderer();
            mPreviewCamera = new PreviewCamera(mCamera);

            // Load the vertex and index buffers
            ClassLoader classLoader = getClass().getClassLoader();
            InputStream previewMesh = classLoader.getResourceAsStream("sphere.mesh");
            Filamesh mesh = FilameshLoader.load("mesh", previewMesh, engine);
            mVertexBuffer = mesh.getVertexBuffer();
            mIndexBuffer = mesh.getIndexBuffer();

            mCamera.setProjection(90.0, 1.3, 0.1, 200.0, Camera.Fov.HORIZONTAL);
            mCamera.lookAt(1.5f, 1.5f, 1.5f, 0, 0, 0, 0, 1, 0);

            mIndirectLight = LightHelpers.addIndirectLight(engine, mScene);

            loadMesh(engine, mScene, mVertexBuffer, mIndexBuffer);

            int deviceScaleFactor = getDeviceScaleFactor();
            mView.setViewport(new Viewport(0, 0, 640 * deviceScaleFactor,
                    480 * deviceScaleFactor));
            mView.setShadowsEnabled(false);
            mView.setCamera(mCamera);
            mView.setScene(mScene);
            mView.setClearColor(0f, 0f, 0f, 1.0f);

            mFilamentPanel = new FilamentPanel();

            mViewer = new TungstenViewer(mCamera, PreviewMeshPanel.this);
            assert mFilamentPanel != null;
            assert mView != null;
            assert mRenderer != null;
            mViewer.panel = mFilamentPanel;
            mViewer.view = mView;
            mViewer.renderer = mRenderer;

            Filament.getInstance().addViewer(mViewer);

            SwingUtilities.invokeLater(() -> {
                add(mFilamentPanel, BorderLayout.CENTER);
                revalidate();
            });
        });
    }

    public void updateMaterial(MaterialInstance newMaterialInstance) {
        Filament.getInstance().runOnFilamentThread((Engine engine) -> {
            mMeshRenderable = engine.getRenderableManager().getInstance(mMeshEntity);
            engine.getRenderableManager().setMaterialInstanceAt(mMeshRenderable, 0,
                    newMaterialInstance);
        });
    }

    public void destroy() {
        Filament.getInstance().removeViewer(mViewer);
        Filament.getInstance().runOnFilamentThread((Engine engine) -> {
            engine.destroyRenderer(mRenderer);
            engine.destroyScene(mScene);
            engine.destroyCameraComponent(mCamera.getEntity());
            EntityManager.get().destroy(mCamera.getEntity());
            engine.destroyView(mView);
            mFilamentPanel.destroy(engine);
            engine.destroyVertexBuffer(mVertexBuffer);
            engine.destroyIndexBuffer(mIndexBuffer);
            engine.destroyIndirectLight(mIndirectLight);
            EntityManager.get().destroy(mMeshEntity);
            engine.getRenderableManager().destroy(mMeshEntity);
            engine.getTransformManager().destroy(mMeshTransform);
        });
    }

    private void loadMesh(Engine engine, Scene scene, VertexBuffer vb, IndexBuffer ib) {
        mMeshEntity = EntityManager.get().create();
        mMeshTransform = engine.getTransformManager().create(mMeshEntity, 0,
                MathUtils.createUniformScaleMatrix(3.0f));
        RenderableManager.Builder renderableBuilder = new RenderableManager.Builder(1);
        renderableBuilder
                .geometry(0, RenderableManager.PrimitiveType.TRIANGLES, vb, ib)
                .boundingBox(new Box(0, 0, 0, 1f, 1f, 1f))
                .build(engine, mMeshEntity);

        scene.addEntity(mMeshEntity);
    }

    private static int getDeviceScaleFactor() {
        final GraphicsDevice defaultScreenDevice = GraphicsEnvironment.getLocalGraphicsEnvironment()
                .getDefaultScreenDevice();
        int scaleFactor = 1;
        final Class<?> screenDeviceClass = defaultScreenDevice.getClass();
        if (screenDeviceClass.getCanonicalName().equals("sun.awt.CGraphicsDevice")) {
            try {
                final Method getScaleFactorMethod =
                        screenDeviceClass.getDeclaredMethod("getScaleFactor");
                Object result = getScaleFactorMethod.invoke(defaultScreenDevice);
                return (int) result;
            } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
                e.printStackTrace();
                System.err.println("Unable to determine scale factor of screen, defaulting to 1");
            }
        }
        return scaleFactor;
    }
}
