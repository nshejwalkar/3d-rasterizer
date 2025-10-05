#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "polygon.h"
#include "camera.h"

#define LOG(msg) std::cout << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl
// #define LOG(msg)

inline void printVertexPos(const glm::vec4& v) {
    std::cout << "(" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ")" << std::endl;
}

inline void printVertex(const Vertex& v) {
    std::cout << "Position: ";
    printVertexPos(v.m_pos);
    // std::cout << "Color:" << std::endl;
    // std::cout << "(" << v.m_color[0] << ", " << v.m_color[1] << ", " << v.m_color[2] << ")" << std::endl;
    // std::cout << "Normal:" << std::endl;
    // std::cout << "(" << v.m_normal[0] << ", " << v.m_normal[1] << ", " << v.m_normal[2] << ", " << v.m_normal[3] << ")" << std::endl;
    // std::cout << "UV:" << std::endl;
    // std::cout << "(" << v.m_uv[0] << ", " << v.m_uv[1] << ")" << std::endl;
}

inline void printTriangle(const Polygon& p, const Triangle& t) {
    printVertex(p.VertAt(t.m_indices[0]));
    printVertex(p.VertAt(t.m_indices[1]));
    printVertex(p.VertAt(t.m_indices[2]));

}

inline void printTriangle(const Polygon& p, const Triangle& t, const std::array<Vertex, 3>& pv) {
    printVertex(pv[0]);
    printVertex(pv[1]);
    printVertex(pv[2]);
}

inline void printCamera(const Camera& c) {
    std::cout << "m_forward: ";
    printVertexPos(c.m_forward);
    std::cout << "m_right: ";
    printVertexPos(c.m_right);
    std::cout << "m_up: ";
    printVertexPos(c.m_up);
    std::cout << "fov: " << c.m_fov << std::endl;
    std::cout << "m_position: ";
    printVertexPos(c.m_position);
    std::cout << "m_near_clip: " << c.m_near_clip << std::endl;
    std::cout << "m_far_clip: " << c.m_far_clip << std::endl;
    std::cout << "m_aspect_ratio: " << c.m_aspect_ratio << std::endl;

}

inline void drawBoundingBox(const Triangle& t, QImage& result) {
    if (t.offScreen) return;
    for (int x = t.m_boundingBox.minX; x < t.m_boundingBox.maxX; x++) {
        result.setPixelColor(x, (int)t.m_boundingBox.minY, QColor(0,255,0));
        result.setPixelColor(x, (int)t.m_boundingBox.maxY, QColor(0,255,0));
    }
    for (int y = t.m_boundingBox.minY; y < t.m_boundingBox.maxY; y++) {
        result.setPixelColor((int)t.m_boundingBox.minX, y, QColor(0,255,0));
        result.setPixelColor((int)t.m_boundingBox.maxX, y, QColor(0,255,0));
    }
}
