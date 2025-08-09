/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.pagecurl;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.opengl.Matrix;
import android.os.Bundle;
import android.view.Choreographer;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.util.Log;

import com.google.android.filament.Camera;
import com.google.android.filament.Engine;
import com.google.android.filament.Entity;
import com.google.android.filament.EntityManager;
import com.google.android.filament.Fence;
import com.google.android.filament.Filament;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.LightManager;
import com.google.android.filament.Renderer;
import com.google.android.filament.Scene;
import com.google.android.filament.Skybox;
import com.google.android.filament.SwapChain;
import com.google.android.filament.Texture;
import com.google.android.filament.TransformManager;
import com.google.android.filament.Viewport;

import com.google.android.filament.android.DisplayHelper;
import com.google.android.filament.android.FilamentHelper;
import com.google.android.filament.android.TextureHelper;
import com.google.android.filament.android.UiHelper;

public class MainActivity extends Activity
        implements Choreographer.FrameCallback, UiHelper.RendererCallback, View.OnTouchListener {
    static {
        Filament.init();
    }

    private SurfaceView mSurfaceView;
    private UiHelper mUiHelper;
    private DisplayHelper mDisplayHelper;
    private Choreographer mChoreographer;
    private Engine mEngine;
    private SwapChain mSwapChain;
    private com.google.android.filament.View mView;
    private Renderer mRenderer;
    private Camera mCamera;
    private Page mPage;
    private PageMaterials mPageMaterials;
    private Scene mScene;
    private final Texture[] mTextures = new Texture[2];
    private @Entity int mLight;
    private IndirectLight mIndirectLight;

    private final float[] mTouchDownPoint = new float[2];
    private float mTouchDownValue = 0;
    private float mPageAnimationRadians = 0;
    private float mPageAnimationValue = 0;

    @Override
    public void onNativeWindowChanged(Surface surface) {
        if (mSwapChain != null) {
            mEngine.destroySwapChain(mSwapChain);
        }
        mSwapChain = mEngine.createSwapChain(surface);
        mDisplayHelper.attach(mRenderer, mSurfaceView.getDisplay());
    }

    @Override
    public void onDetachedFromSurface() {
        mDisplayHelper.detach();
        if (mSwapChain != null) {
            mEngine.destroySwapChain(mSwapChain);
            mEngine.flushAndWait();
            mSwapChain = null;
        }
    }

    @Override
    public void onResized(int width, int height) {
        float aspect = (float) width / (float) height;
        if (aspect < 1) {
            mCamera.setProjection(70.0, aspect, 1.0, 2000.0, Camera.Fov.VERTICAL);
        } else {
            mCamera.setProjection(60.0, aspect, 1.0, 2000.0, Camera.Fov.HORIZONTAL);
        }
        mView.setViewport(new Viewport(0, 0, width, height));
        FilamentHelper.synchronizePendingFrames(mEngine);
    }

    @Override
    public void doFrame(long frameTimeNanos) {
        mChoreographer.postFrameCallback(this);

        // Perform a rigid body rotation on the page and translate it along the Y axis.
        final float[] transformMatrix = new float[16];
        final double degrees = -Math.toDegrees(mPageAnimationRadians);
        Matrix.setRotateM(transformMatrix, 0, (float) degrees, 0.0f, 1.0f, 0.0f);
        Matrix.translateM(transformMatrix, 0, 0.0f, -0.5f, 0.0f);
        TransformManager tcm = mEngine.getTransformManager();
        tcm.setTransform(tcm.getInstance(mPage.renderable), transformMatrix);

        if (mUiHelper.isReadyToRender() && mRenderer.beginFrame(mSwapChain, frameTimeNanos)) {
            mRenderer.render(mView);
            mRenderer.endFrame();
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        mSurfaceView = new SurfaceView(this);
        setContentView(mSurfaceView);

        mChoreographer = Choreographer.getInstance();

        mDisplayHelper = new DisplayHelper(this);

        mUiHelper = new UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK);
        mUiHelper.setRenderCallback(this);
        mUiHelper.attachTo(mSurfaceView);

        mEngine = Engine.create();
        mRenderer = mEngine.createRenderer();
        mScene = mEngine.createScene();
        mView = mEngine.createView();
        mCamera = mEngine.createCamera(mEngine.getEntityManager().create());

        mCamera.lookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

        // Use unit-less light intensities since this is UI-style rendering, not physically based.
        mCamera.setExposure(1.0f);

        mLight = EntityManager.get().create();

        new LightManager.Builder(LightManager.Type.DIRECTIONAL)
                .color(1.0f, 1.0f, 1.0f)
                .intensity(15f)
                .direction(0.0f, -0.54f, -0.89f) // should be a unit vector
                .castShadows(false)
                .build(mEngine, mLight);

        mIndirectLight = IblLoader.loadIndirectLight(getAssets(), "envs/studio_small_02_2k",
                mEngine);
        mIndirectLight.setIntensity(1.0f);
        mScene.setIndirectLight(mIndirectLight);

        mScene.addEntity(mLight);

        Skybox sky = new Skybox.Builder().color(0.1f, 0.2f, 0.4f, 1.0f).build(mEngine);
        mScene.setSkybox(sky);

        mView.setCamera(mCamera);
        mView.setScene(mScene);

        mPageMaterials = new PageMaterials(mEngine, getAssets());
        mPage = new PageBuilder().materials(mPageMaterials).build(mEngine, EntityManager.get());
        if (mPage == null) {
            throw new IllegalStateException("Unable to build page geometry.");
        }

        mTextures[0] = loadTexture(mEngine, R.drawable.dog);
        mTextures[1] = loadTexture(mEngine, R.drawable.cat);
        mPage.setTextures(mTextures[0], mTextures[1]);

        mScene.addEntity(mPage.renderable);

        mSurfaceView.setOnTouchListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mChoreographer.postFrameCallback(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mChoreographer.removeFrameCallback(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mChoreographer.removeFrameCallback(this);
        mUiHelper.detach();

        mEngine.destroyEntity(mLight);
        mEngine.destroyEntity(mPage.renderable);
        mEngine.destroyRenderer(mRenderer);
        mEngine.destroyVertexBuffer(mPage.vertexBuffer);
        mEngine.destroyIndexBuffer(mPage.indexBuffer);
        mEngine.destroyMaterialInstance(mPage.material);
        mEngine.destroyMaterial(mPageMaterials.getMaterial());
        mEngine.destroyTexture(mTextures[0]);
        mEngine.destroyTexture(mTextures[1]);
        mEngine.destroyIndirectLight(mIndirectLight);

        mEngine.destroyView(mView);
        mEngine.destroyScene(mScene);
        mEngine.destroyCameraComponent(mCamera.getEntity());

        EntityManager.get().destroy(mPage.renderable);
        EntityManager.get().destroy(mCamera.getEntity());

        mEngine.destroy();
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN: {
                mTouchDownPoint[0] = event.getX(0);
                mTouchDownPoint[1] = event.getY(0);
                mTouchDownValue = mPageAnimationValue;
                return true;
            }
            case MotionEvent.ACTION_MOVE: {
                // Compute a value in [-0.5, +0.5]
                final float dx = (event.getX(0) - mTouchDownPoint[0]) / mSurfaceView.getWidth();
                mPageAnimationValue = Math.min(+0.5f, Math.max(-0.5f, mTouchDownValue + dx));

                // In this demo, we only care about dragging the right hand page leftwards.
                final float t = -mPageAnimationValue * 3.0f;
                final float rigidity = 0.1f;
                final Page.CurlStyle style = Page.CurlStyle.BREEZE;
                mPageAnimationRadians = mPage.updateVertices(mEngine, t, rigidity, style);
                return true;
            }
            case MotionEvent.ACTION_UP: {
                return true;
            }
        }
        return super.onTouchEvent(event);
    }

    @SuppressWarnings("SameParameterValue")
    private Texture loadTexture(Engine engine, int resourceId) {
        Resources resources = getResources();
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPremultiplied = true;

        Bitmap bitmap = BitmapFactory.decodeResource(resources, resourceId, options);
        bitmap = addWhiteBorder(bitmap, 20);

        Texture texture = new Texture.Builder()
                .width(bitmap.getWidth())
                .height(bitmap.getHeight())
                .sampler(Texture.Sampler.SAMPLER_2D)
                .usage(Texture.Usage.DEFAULT | Texture.Usage.GEN_MIPMAPPABLE)
                .format(Texture.InternalFormat.SRGB8_A8) // It is crucial to use an SRGB format.
                .levels(0xff) // tells Filament to figure out the number of mip levels
                .build(engine);

        android.os.Handler handler = new android.os.Handler();
        TextureHelper.setBitmap(engine, texture, 0, bitmap, handler, new Runnable() {
            @Override
            public void run() {
                Log.i("page-curl", "Bitmap is released.");
            }
        });
        texture.generateMipmaps(engine);
        return texture;
    }

    @SuppressWarnings("SameParameterValue")
    private Bitmap addWhiteBorder(Bitmap bmp, int borderSize) {
        Bitmap modified = Bitmap.createBitmap(
                bmp.getWidth() + borderSize * 2,
                bmp.getHeight() + borderSize * 2,
                bmp.getConfig());
        Canvas canvas = new Canvas(modified);
        canvas.drawColor(Color.WHITE);
        canvas.drawBitmap(bmp, borderSize, borderSize, null);
        return modified;
    }
}
