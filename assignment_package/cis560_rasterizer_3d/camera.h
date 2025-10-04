#pragma once

#include "glm/glm.hpp"

class Camera
{
public:
    Camera();
    Camera(glm::vec4,
           glm::vec4,
           glm::vec4,
           float,
           glm::vec4,
           float,
           float,
           float);

    glm::mat4 viewMatrix() const;
    glm::mat4 perspProjMatrix() const;

    void translateX(float);
    void translateY(float);
    void translateZ(float);

    void rotateX(float);  // pitch
    void rotateY(float);  // yaw
    void rotateZ(float);  // roll

public:
    glm::vec4 m_forward;
    glm::vec4 m_right;
    glm::vec4 m_up;
    float m_fov;
    glm::vec4 m_position;
    float m_near_clip;
    float m_far_clip;
    float m_aspect_ratio;
};

