#pragma once

#include <glm/glm.hpp>
#include "polygon.h"


class Segment
{
public:
    Segment();  // this is so Triangle can be default constructed
    Segment(glm::vec4, glm::vec4);  // two 2d (4d) points in pixel space. we'll construct this from pos's instead of the whole Vertex

    bool checkIntersection(int, float*) const;  // int is the y-coord of the scanline, float* is a ptr to a stack var that holds x val of intersection if true

private:
    glm::vec2 m_vertex1;
    glm::vec2 m_vertex2;
    float m_slope;
};

