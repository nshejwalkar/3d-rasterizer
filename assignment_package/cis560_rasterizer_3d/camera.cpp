#include "camera.h"
#include <cmath>
#include "glm/gtx/orthonormalize.hpp"
#include "glm/gtx/transform.hpp"

Camera::Camera()
    : m_forward(0.0f, 0.0f, -1.0f, 0.0f),
    m_right(1.0f, 0.0f, 0.0f, 0.0f),
    m_up(0.0f, 1.0f, 0.0f, 0.0f),
    m_fov(45.0f),
    m_position(0.0f, 0.0f, 10.0f, 1.0f),
    m_near_clip(0.01f),
    m_far_clip(100.0f),
    m_aspect_ratio(1.0f)
{}

Camera::Camera(glm::vec4 forward,
               glm::vec4 right,
               glm::vec4 up,
               float fov,
               glm::vec4 position,
               float near_clip,
               float far_clip,
               float aspect_ratio)
    :
    m_forward(forward),
    m_right(right),
    m_up(up),
    m_fov(fov),
    m_position(position),
    m_near_clip(near_clip),
    m_far_clip(far_clip),
    m_aspect_ratio(aspect_ratio)
{}

glm::mat4 Camera::viewMatrix() const {
    // use the position, forward, right and up
    const float tx = -glm::dot(m_right, m_position);
    const float ty = -glm::dot(m_up, m_position);
    const float tz = -glm::dot(m_forward, m_position);

    return glm::mat4({m_right.x, m_up.x, m_forward.x, 0},
                     {m_right.y, m_up.y, m_forward.y, 0},
                     {m_right.z, m_up.z, m_forward.z, 0},
                     {tx, ty, tz, 1});
}

glm::mat4 Camera::perspProjMatrix() const {
    // use the near and far planes for z
    // use aspect ratio and fov for x,y
    // dont convert to radians
    // after applying this matrix to camera coords, you get NDC:
    // x,y \in [-1,1], z \in [0,1]
    const float t = std::tan(glm::radians(m_fov/2));
    const float a = m_aspect_ratio;
    const float n = m_near_clip;
    const float f = m_far_clip;
    return glm::mat4({1.f/(a*t), 0, 0, 0},
                     {0, 1.f/t, 0, 0},
                     {0, 0, f/(f-n), 1.f},
                     {0, 0, (-f*n)/(f-n), 0});
}

// perform translations in camera space, not world space.
// just increment by respective basis vector
void Camera::translateX(float x) {
    m_position += m_right*x;
};
void Camera::translateY(float y) {
    m_position += m_up*y;
};
void Camera::translateZ(float z) {
    m_position += m_forward*z;
};

// this is needed to force up, forward and right to be normalized and stay orthogonal, even after
// potential drift from rotation
static inline void orthonormalize(glm::vec4& R, glm::vec4& U, glm::vec4& F) {
    glm::mat3 B({glm::vec3(R), glm::vec3(U), glm::vec3(F)});
    B = glm::orthonormalize(B);
    R = glm::vec4(B[0], 0.f);
    U = glm::vec4(B[1], 0.f);
    F = glm::vec4(B[2], 0.f);
}

void Camera::rotateX(float theta) {  // pitch
    // rotating about x means that m_right won't change
    // the x coordinate for both vectors won't change either
    glm::mat4 M = glm::rotate(glm::mat4(1.f), theta, glm::vec3(m_right));
    m_up = M * m_up;
    m_forward = M * m_forward;
    orthonormalize(m_right, m_up, m_forward);
};

void Camera::rotateY(float theta) {  // yaw
    glm::mat4 M = glm::rotate(glm::mat4(1.f), theta, glm::vec3(m_up));
    m_right = M * m_right;
    m_forward = M * m_forward;
    orthonormalize(m_right, m_up, m_forward);
};

void Camera::rotateZ(float theta) {  // roll
    glm::mat4 M = glm::rotate(glm::mat4(1.f), theta, glm::vec3(m_forward));
    m_right = M * m_right;
    m_up = M * m_up;
    orthonormalize(m_right, m_up, m_forward);
};
