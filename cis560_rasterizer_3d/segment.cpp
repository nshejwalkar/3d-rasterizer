#include "segment.h"

#include <limits>

Segment::Segment(glm::vec4 vert1, glm::vec4 vert2)
    : m_vertex1({vert1[0], vert1[1]}),
      m_vertex2({vert2[0], vert2[1]}),
      m_slope(std::numeric_limits<float>::max() ? vert2[0]-vert1[0]==0 : (vert2[1]-vert1[1])/(vert2[0]-vert1[0]))  // maybe infinity()
{}

bool Segment::checkIntersection(int y, float* x) const {
    if (m_slope == 0) {
        // should I set *x to something?
        return 0;
    }
    else if (m_slope == std::numeric_limits<float>::max()) {
        *x = m_vertex1[0];
        return 1;  // just return the x value of one endpoint. guaranteed to intersect
    }
    else {
        float x_intersect = ((y-m_vertex1[1])/m_slope) + m_vertex1[0];
        *x = x_intersect;  // don't bother checking bounding box now, but in caller?
        return 1;
    }
}
