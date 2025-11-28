// Minimal interactive camera (look-at + perspective)
#pragma once

#include <bgfx/bgfx.h>
#include <bx/math.h>

class Camera
{
public:
    Camera();

    void setPositionTarget(const bx::Vec3& pos, const bx::Vec3& target);
    void setFov(float fovYDegrees);
    void setClipPlanes(float nearZ, float farZ);
    void setViewportSize(uint32_t w, uint32_t h); // updates aspect

    void getView(float* out16) const;  // look-at
    void getProj(float* out16) const;  // perspective

private:
    bx::Vec3 m_position;
    bx::Vec3 m_target;
    bx::Vec3 m_up;
    float m_fovY;      // degrees
    float m_nearZ;
    float m_farZ;
    float m_aspect;    // width/height
};