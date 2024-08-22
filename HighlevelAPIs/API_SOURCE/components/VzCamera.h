#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    struct API_EXPORT VzCamera : VzSceneComp
    {
        VzCamera(const VID vid, const std::string& originFrom)
            : VzSceneComp(vid, originFrom, "VzCamera", SCENE_COMPONENT_TYPE::CAMERA) {}
        // Pose parameters are defined in WS (not local space)
        void SetWorldPose(const float pos[3], const float view[3], const float up[3]);
        void SetPerspectiveProjection(const float zNearP, const float zFarP, const float fovInDegree, const float aspectRatio, const bool isVertical = true);
        void GetWorldPose(float pos[3], float view[3], float up[3]);
        void GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovInDegree, float* aspectRatio, bool isVertical = true);

        void SetCameraCubeVisibleLayerMask(const uint8_t layerBits = 0x3, const uint8_t maskBits = 0x2); // helper object

        void SetLensProjection(float focalLengthInMillimeters, float aspect, float near, float far);

        float GetNear();
        float GetCullingFar();

        void SetExposure(float aperture, float shutterSpeed, float sensitivity);
        float GetAperture();
        float GetShutterSpeed();
        float GetSensitivity();

        float GetFocalLength();

        void SetFocusDistance(float distance);
        float GetFocusDistance();

        struct API_EXPORT Controller{
        private:
            VID vidCam_ = INVALID_VID;
        public:
            Controller(VID vidCam) { vidCam_ = vidCam; }
            VID GetCameraVID() { return vidCam_; }

            // Configuration parameters //
            // Common properties
            float targetPosition[3] = {0, 0, 0};        //! World-space position of interest, defaults to (0,0,0)
            float upVector[3] = { 0, 1.f, 0 };            //! Orientation for the home position, defaults to (0,1,0)
            float zoomSpeed = 0.01f;                    //! Multiplied with scroll delta, defaults to 0.01
            // Orbit mode properties
            float orbitHomePosition[3] = {0, 0, 1.f};   //! Initial eye position in world space, defaults to (0,0,1)
            float orbitSpeed[2] = {0.01f, 0.01f};       //! Multiplied with viewport delta, defaults to 0.01
            // Map mode properties
            bool isVerticalFov = true;                  //! The axis that's held constant when viewport changes
            float fovDegrees = 90.f;                    //! The full FOV (not the half-angle)
            float farPlane = 1000.f;                    //! The distance to the far plane
            float mapExtent[2] = {10.f, 10.f};          //! The ground size for computing home position
            float mapMinDistance = 0.01f;               //! Constrains the zoom-in level
            // Free flight properties
            float flightStartPosition[3];               //! Initial eye position in world space, defaults to (0,0,0)
            float flightStartPitch = 0;
            float flightStartYaw = 0;
            float flightMaxSpeed = 10.f;                //! The maximum camera speed in world units per second, defaults to 10
            int flightSpeedSteps = 80;              //! The number of speed steps adjustable with scroll wheel, defaults to 80
            float flightPanSpeed[2] = {0.01f, 0.01f};   //! Multiplied with viewport delta, defaults to 0.01,0.01
            float flightMoveDamping = 15.f;             //! Applies a deceleration to camera movement, 0 (no damping), default is 15
            // Raycast properties
            float groundPlane[4] = {0, 1.f, 0, 0};  //! Plane equation used as a raycast fallback
            void* raycastCallback = nullptr;        //! Raycast function for accurate grab-and-pan
            void* raycastUserdata = nullptr;
            bool panning = true;                    //! Sets whether panning is enabled
            enum class Mode { ORBIT, MAP, FREE_FLIGHT };
            Mode mode = Mode::ORBIT;

            void UpdateControllerSettings();            // call this to apply current configuration 

            enum class Key { FORWARD, LEFT, BACKWARD, RIGHT, UP, DOWN, COUNT };
            void KeyDown(const Key key);
            void KeyUp(const Key key);
            void Scroll(const int x, const int y, const float scrollDelta);
            void GrabBegin(const int x, const int y, const bool strafe);
            void GrabDrag(const int x, const int y);
            void GrabEnd();
            void SetViewport(const int w, const int h);
            void UpdateCamera(const float deltaTime); // this is for final sync to the camera
        };
        Controller* GetController();              // this activates the camera manipulator
    };
}
