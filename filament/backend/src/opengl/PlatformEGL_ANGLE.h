#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_ANGLE_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_ANGLE_H

#include "PlatformEGL.h"

namespace filament {

class PlatformEGL_ANGLE final : public PlatformEGL {
public:
    explicit PlatformEGL_ANGLE(EGLDisplay externalDevice) noexcept;

    backend::Driver* createDriver(void* sharedContext) noexcept override;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_ANGLE_D3D11_H
