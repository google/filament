#include "VzCamera.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern VzEngineApp gEngineApp;

namespace vzm
{
    // Pose parameters are defined in WS (not local space)
    void VzCamera::SetWorldPose(const float pos[3], const float view[3], const float up[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );

        // up vector correction
        double3 _eye = *(float3*)pos;
        double3 _view = normalize((double3) * (float3*)view);
        double3 _up = *(float3*)up;
        double3 _right = cross(_view, _up);
        _up = normalize(cross(_right, _view));

        // note the pose info is defined in WS
        //mat4f ws2cs = mat4f::lookTo(_view, _eye, _up);
        //mat4f cs2ws = inverse(ws2cs);
        Camera* camera = gEngine->getCameraComponent(ett);
        camera->lookAt(_eye, _eye + _view, _up);
        mat4 ws2cs_d = camera->getViewMatrix();
        mat4 cs2ws_d = inverse(ws2cs_d);

        Entity ett_parent = tc.getParent(ins);
        mat4 parent2ws_d = mat4();
        while (!ett_parent.isNull())
        {
            auto ins_parent = tc.getInstance(ett_parent);
            parent2ws_d = mat4(tc.getTransform(ins_parent)) * parent2ws_d;
            ett_parent = tc.getParent(ins_parent);
        }

        mat4f local = mat4f(inverse(parent2ws_d) * cs2ws_d);
        SetMatrix((float*)&local[0][0], false, false);
    }
    void VzCamera::SetPerspectiveProjection(const float zNearP, const float zFarP, const float fovInDegree, const float aspectRatio, const bool isVertical)
    {
        COMP_CAMERA(camera, ett, );
        // aspectRatio is W / H
        camera->setProjection(fovInDegree, aspectRatio, zNearP, zFarP,
            isVertical ? Camera::Fov::VERTICAL : Camera::Fov::HORIZONTAL);
        timeStamp = std::chrono::high_resolution_clock::now();
    }

    void VzCamera::SetCameraCubeVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        VzCameraRes* cam_res = gEngineApp.GetCameraRes(componentVID);
        if (cam_res == nullptr) return;

        Cube* camera_cube = cam_res->GetCameraCube();
        auto& rcm = gEngine->getRenderableManager();
        rcm.setLayerMask(rcm.getInstance(camera_cube->getSolidRenderable()), layerBits, maskBits);
        rcm.setLayerMask(rcm.getInstance(camera_cube->getWireFrameRenderable()), layerBits, maskBits);

        cubeToScene(camera_cube->getSolidRenderable(), componentVID);
        cubeToScene(camera_cube->getWireFrameRenderable(), componentVID);
        timeStamp = std::chrono::high_resolution_clock::now();
    }

    void VzCamera::GetWorldPose(float pos[3], float view[3], float up[3])
    {
        COMP_CAMERA(camera, ett, );
        double3 p = camera->getPosition();
        double3 v = camera->getForwardVector();
        double3 u = camera->getUpVector();
        if (pos) *(float3*)pos = float3(p);
        if (view) *(float3*)view = float3(v);
        if (up) *(float3*)up = float3(u);
    }
    void VzCamera::GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovInDegree, float* aspectRatio, bool isVertical)
    {
        COMP_CAMERA(camera, ett, );
        if (zNearP) *zNearP = (float)camera->getNear();
        if (zFarP) *zFarP = (float)camera->getCullingFar();
        if (fovInDegree) *fovInDegree = (float)camera->getFieldOfViewInDegrees(isVertical ? Camera::Fov::VERTICAL : Camera::Fov::HORIZONTAL);
        if (aspectRatio)
        {
            mat4 mat_proj = camera->getProjectionMatrix();
            *aspectRatio = (float)(mat_proj[1][1] / mat_proj[0][0]);
        }
    }

    VzCamera::Controller* VzCamera::GetController()
    {
        VzCameraRes* cam_res = gEngineApp.GetCameraRes(componentVID);
        if (cam_res == nullptr) return nullptr;
        CameraManipulator* cm = cam_res->GetCameraManipulator();
        Controller* cc = cam_res->GetCameraController();
        if (cm == nullptr)
        {
            Controller controller(componentVID);
            cam_res->NewCameraManipulator(controller);
            cc = cam_res->GetCameraController();
        }
        return cc;
    }
#define GET_CM(CAMRES, CM) VzCameraRes* CAMRES = gEngineApp.GetCameraRes(GetCameraVID()); if (CAMRES == nullptr) return;  CameraManipulator* CM = CAMRES->GetCameraManipulator();
#define GET_CM_WARN(CAMRES, CM) GET_CM(CAMRES, CM) if (CM == nullptr) { backlog::post("camera manipulator is not set!", backlog::LogLevel::Warning); return; }
    void VzCamera::Controller::UpdateControllerSettings()
    {
        GET_CM(cam_res, cm);
        cam_res->NewCameraManipulator(*this);
    }
    void VzCamera::Controller::KeyDown(const Key key)
    {
        GET_CM_WARN(cam_res, cm);
        cm->keyDown((CameraManipulator::Key)key);
    }
    void VzCamera::Controller::KeyUp(const Key key)
    {
        GET_CM_WARN(cam_res, cm);
        cm->keyUp((CameraManipulator::Key)key);
    }
    void VzCamera::Controller::Scroll(const int x, const int y, const float scrollDelta)
    {
        GET_CM_WARN(cam_res, cm);
        cm->scroll(x, y, scrollDelta);
    }
    void VzCamera::Controller::GrabBegin(const int x, const int y, const bool strafe)
    {
        GET_CM_WARN(cam_res, cm);
        cm->grabBegin(x, y, strafe);
    }
    void VzCamera::Controller::GrabDrag(const int x, const int y)
    {
        GET_CM_WARN(cam_res, cm);
        cm->grabUpdate(x, y);
    }
    void VzCamera::Controller::GrabEnd()
    {
        GET_CM_WARN(cam_res, cm);
        cm->grabEnd();
    }
    void VzCamera::Controller::SetViewport(const int w, const int h)
    {
        GET_CM_WARN(cam_res, cm);
        cm->setViewport(w, h);
    }
    void VzCamera::Controller::UpdateCamera(const float deltaTime)
    {
        GET_CM_WARN(cam_res, cm);
        cam_res->UpdateCameraWithCM(deltaTime);
    }
}
