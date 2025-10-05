#pragma once
#include <polygon.h>
#include <QImage>

#include "constants.h"
#include <vector>
#include "camera.h"

class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    std::vector<Polygon> m_polygons;
public:
    Rasterizer(const std::vector<Polygon>& polygons);

    static const unsigned long long m_zbufsize = (unsigned long long)(SCREEN_HEIGHT*SCREEN_WIDTH);
    // initialize the z_buffer to be infinity everywhere
    std::vector<float> m_zbuffer = std::vector<float>(m_zbufsize, std::numeric_limits<float>::infinity());

    bool ConsultAndWriteToZBuffer(const int, const int, const float);
    void resetZBuffer();

    Camera m_camera;

    Triangle projectTriangleFromWorldtoPixelSpace(const glm::mat4,
                                                  const glm::mat4,
                                                  const Polygon&,
                                                  const Triangle,
                                                  std::array<Vertex,3>&) const;

    QImage RenderScene();
    void ClearScene();

    void RenderTriangle(const Polygon&, const Triangle&, QImage&);
    void RenderTriangle(const Polygon&, const Triangle&, std::array<Vertex,3>&, QImage&);

    float computeSubTriangleArea(const glm::vec2&, const glm::vec2&, const glm::vec2&) const; // make this const

    BarycentricWeights ComputeBarycentricWeights(const Polygon&,
                                                 const Triangle&,
                                                 const glm::vec2&) const; // make this const

    BarycentricWeights perspectiveCorrectBarycentricWeights(const Triangle&,
                                                            std::array<Vertex,3>&,
                                                            const glm::vec2&) const;

    template <typename T>
    T perspectiveCorrectInterpolateAttrib(const T&,
                                          const T&,
                                          const T&,
                                          const BarycentricWeights&,
                                          std::array<Vertex,3>&) const;

    glm::vec4 perspectiveCorrectInterpolateAttrib(const glm::vec4&,
                                                  const glm::vec4&,
                                                  const glm::vec4&,
                                                  const Triangle&,
                                                  std::array<Vertex,3>&,
                                                  const glm::vec2&) const;

};
