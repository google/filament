/*
* Copyright (C) 2019 The Android Open Source Project
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

/*
 * This file exists only to test the buildability of our TypeScript annotations, To run the test,
 * invoke Filament's easy build script with "./build.sh -up webgl release", or do:
 *
 * npx tsc --noEmit \
 *     ../../third_party/gl-matrix/gl-matrix.d.ts \
 *     ./filament.d.ts \
 *     test.ts
 *
 * Note that it suffices to simply build the test, it is not meant to be executed.
 */

import * as Filament from "./filament";
import * as glm from "gl-matrix";

const v2 = glm.vec2.create();
const v3 = glm.vec3.create();
const v4 = glm.vec4.create();
const m3 = glm.mat3.create();
const m4 = glm.mat4.create();
const qt = glm.quat.create();

const canvas = new HTMLCanvasElement();
const engine = Filament.Engine.create(canvas);

function smoke_camera_frustum() {
    const camera: Filament.Camera = engine.createCamera();
    camera.setProjection(Filament.Camera$Projection.ORTHO, 0, 1, 0, 1, 0, 1);
    camera.setProjectionFov(45, 1.0, 0.0, 1.0, Filament.Camera$Fov.HORIZONTAL);
    camera.setLensProjection(0, 0.33, 1, 2);
    camera.setCustomProjection(m4, 0, 1);
    const m5 = camera.getProjectionMatrix() as glm.mat4;
    const m6 = camera.getCullingProjectionMatrix() as glm.mat4;
    camera.lookAt([0, 0, 0], [0, 0, 0], [0, 0, 0]);
    camera.lookAt(v3, v3, v3);
    const frustum: Filament.Frustum = camera.getFrustum();
    frustum.setProjection(m4);
    const v5 = frustum.getNormalizedPlane(Filament.Frustum$Plane.BOTTOM) as glm.vec4;
    const b: boolean = frustum.intersectsSphere([0, 1, 2, 3]);
    const c: boolean = frustum.intersectsSphere(glm.vec4.fromValues(0, 1, 2, 3));
    console.log(m5, m6, v5, b, c);
}

function smoke_transforms() {
    const tcm = engine.getTransformManager();
    const entity = Filament.EntityManager.get().create();
    const inst: Filament.TransformManager$Instance = tcm.getInstance(entity);
    tcm.setTransform(inst, m4);
    const m5 = tcm.getTransform(inst) as glm.mat4;
    const m6 = tcm.getWorldTransform(inst) as glm.mat4;
    tcm.openLocalTransformTransaction();
    tcm.commitLocalTransformTransaction();
    console.log(m5, m6);
    inst.delete();
}

function smoke_renderables() {
    const rm = engine.getRenderableManager();
    const entity = Filament.EntityManager.get().create();
    const inst: Filament.RenderableManager$Instance = rm.getInstance(entity);
    const bone: Filament.RenderableManager$Bone = {
        unitQuaternion: qt,
        translation: v3
    };
    rm.setCastShadows(inst, true);
    rm.setBones(inst, [bone], 0);
    Filament.RenderableManager.Builder(1).skinningBones([bone]).build(engine, entity);
    inst.delete();
}

smoke_camera_frustum();
smoke_transforms();
smoke_renderables();
