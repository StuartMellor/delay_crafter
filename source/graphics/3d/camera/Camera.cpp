#include "Camera.h"

Camera::Camera()
    : m_position{ 0.0f, 0.0f, -6.0f }
    , m_target{ 0.0f, 0.0f, 0.0f }
    , m_up{ 0.0f, 1.0f, 0.0f }
    , m_fovY{ 60.0f }
    , m_nearZ{ 0.01f }
    , m_farZ{ 100.0f }
    , m_aspect{ 1.0f }
{
}

void Camera::setPositionTarget(const bx::Vec3& pos, const bx::Vec3& target)
{
    m_position = pos;
    m_target = target;
}

void Camera::setFov(float fovYDegrees)
{
    m_fovY = fovYDegrees;
}

void Camera::setClipPlanes(float nearZ, float farZ)
{
    m_nearZ = nearZ;
    m_farZ = farZ;
}

void Camera::setViewportSize(uint32_t w, uint32_t h)
{
    m_aspect = (h == 0) ? 1.0f : float(w) / float(h);
}

void Camera::getView(float* out16) const
{
    bx::mtxLookAt(out16, m_position, m_target, m_up);
}

void Camera::getProj(float* out16) const
{
    bx::mtxProj(out16, m_fovY, m_aspect, m_nearZ, m_farZ, bgfx::getCaps()->homogeneousDepth);
}
