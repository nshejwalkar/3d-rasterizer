#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "constants.h"
#include "debug.h"
#include "camera.h"
#include <algorithm>

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons)
    : m_polygons(polygons)
{}

float Rasterizer::computeSubTriangleArea(const glm::vec2& v1,
                                       const glm::vec2& v2,
                                       const glm::vec2& v3) const {
    const glm::vec2 a = v2 - v1;
    const glm::vec2 b = v3 - v1;
    const float twiceArea = a.x * b.y - a.y * b.x;
    return 0.5f * std::abs(twiceArea);
}


BarycentricWeights Rasterizer::ComputeBarycentricWeights(const Polygon& p,
                                                         const Triangle& t,
                                                         const glm::vec2 &fragPos) const {

    glm::vec2 v1(p.VertAt(t.m_indices[0]).m_pos.x, p.VertAt(t.m_indices[0]).m_pos.y);
    glm::vec2 v2(p.VertAt(t.m_indices[1]).m_pos.x, p.VertAt(t.m_indices[1]).m_pos.y);
    glm::vec2 v3(p.VertAt(t.m_indices[2]).m_pos.x, p.VertAt(t.m_indices[2]).m_pos.y);

    float s = computeSubTriangleArea(v1, v2, v3);
    float s1 = computeSubTriangleArea(fragPos, v2, v3) / s;
    float s2 = computeSubTriangleArea(fragPos, v1, v3) / s;
    float s3 = computeSubTriangleArea(fragPos, v2, v1) / s;

    return BarycentricWeights(s1, s2, s3);

}

bool Rasterizer::ConsultAndWriteToZBuffer(const int x, const int y, const float candidate_z) {
    float cur_z = m_zbuffer[y*SCREEN_WIDTH+x];
    if (candidate_z < cur_z) {
        m_zbuffer[y*SCREEN_WIDTH+x] = candidate_z;
        return true;
    }
    return false;
}

void Rasterizer::resetZBuffer() {
    m_zbuffer.assign(m_zbuffer.size(), std::numeric_limits<float>::infinity());
}

// pass triangle by copy and fill in proj_verts
Triangle Rasterizer::projectTriangleFromWorldtoPixelSpace(const glm::mat4 proj_mat,
                                                           const glm::mat4 view_mat,
                                                           const Polygon& p,
                                                           const Triangle t,
                                                           std::array<Vertex,3>& pv) const {

    Triangle proj_tri;  // same indices in p.m_verts
    std::copy(std::begin(t.m_indices), std::end(t.m_indices), std::begin(proj_tri.m_indices));

    for (int i=0; i<3; i++) {
        Vertex vert = p.VertAt(proj_tri.m_indices[i]);  // get a copy

        glm::vec4 unhomo = proj_mat * view_mat * vert.m_pos;
        unhomo /= unhomo.w;

        vert.m_pos = {(unhomo.x+1)*(SCREEN_WIDTH/2),
                       (1-unhomo.y)*(SCREEN_HEIGHT/2),
                       unhomo.z,
                       unhomo.w};

        pv[i] = vert;
    }

    return proj_tri;
};

// calculate this for every pixel. triangle is in pixel space
float Rasterizer::perspectiveCorrectInterpolateZ(const Triangle& t,
                                                 std::array<Vertex,3>& pv,
                                                 const glm::vec2& fragPos) const {
    glm::vec2 v1(pv[0].m_pos.x, pv[0].m_pos.y);
    glm::vec2 v2(pv[1].m_pos.x, pv[1].m_pos.y);
    glm::vec2 v3(pv[2].m_pos.x, pv[2].m_pos.y);

    float s = computeSubTriangleArea(v1, v2, v3);
    float s1 = computeSubTriangleArea(fragPos, v2, v3) / s;
    float s2 = computeSubTriangleArea(fragPos, v1, v3) / s;
    float s3 = computeSubTriangleArea(fragPos, v2, v1) / s;

    float z1 = pv[0].m_pos.z;
    float z2 = pv[1].m_pos.z;
    float z3 = pv[2].m_pos.z;

    return 1.f/(s1/z1 + s2/z2 + s3/z3);
}

// returns the same type that was put in
template <typename T>
T Rasterizer::perspectiveCorrectInterpolateAttrib(const T &v1attrib,
                                                  const T &v2attrib,
                                                  const T &v3attrib,
                                                  const Triangle& t,
                                                  std::array<Vertex,3>& pv,
                                                  const glm::vec2& fragPos) const {
    glm::vec2 v1(pv[0].m_pos.x, pv[0].m_pos.y);
    glm::vec2 v2(pv[1].m_pos.x, pv[1].m_pos.y);
    glm::vec2 v3(pv[2].m_pos.x, pv[2].m_pos.y);

    float s = computeSubTriangleArea(v1, v2, v3);
    float s1 = computeSubTriangleArea(fragPos, v2, v3) / s;
    float s2 = computeSubTriangleArea(fragPos, v1, v3) / s;
    float s3 = computeSubTriangleArea(fragPos, v2, v1) / s;

    float z1 = pv[0].m_pos.z;
    float z2 = pv[1].m_pos.z;
    float z3 = pv[2].m_pos.z;

    float pc_z = 1.f/(s1/z1 + s2/z2 + s3/z3);

    return pc_z*((v1attrib*s1/z1) + (v2attrib*s2/z2) + (v3attrib*s3/z3));
}

glm::vec4 Rasterizer::perspectiveCorrectInterpolateAttrib(const glm::vec4 &n1,
                                                        const glm::vec4 &n2,
                                                        const glm::vec4 &n3,
                                                        const Triangle& t,
                                                        std::array<Vertex,3>& pv,
                                                          const glm::vec2& fragPos) const {
    glm::vec2 v1(pv[0].m_pos.x, pv[0].m_pos.y);
    glm::vec2 v2(pv[1].m_pos.x, pv[1].m_pos.y);
    glm::vec2 v3(pv[2].m_pos.x, pv[2].m_pos.y);

    float s = computeSubTriangleArea(v1, v2, v3);
    float s1 = computeSubTriangleArea(fragPos, v2, v3) / s;
    float s2 = computeSubTriangleArea(fragPos, v1, v3) / s;
    float s3 = computeSubTriangleArea(fragPos, v2, v1) / s;

    float z1 = pv[0].m_pos.z;
    float z2 = pv[1].m_pos.z;
    float z3 = pv[2].m_pos.z;

    float pc_z = 1.f/(s1/z1 + s2/z2 + s3/z3);

    return pc_z*((n1*s1/z1) + (n2*s2/z2) + (n3*s3/z3));
}

void Rasterizer::RenderTriangle(const Polygon& p,
                                const Triangle& t,
                                std::array<Vertex,3>& proj_verts,
                                QImage& result) {
    // we have already computed all bounding boxes
    if (t.offScreen) {LOG("OFFSCREEN"); return;}

    Vertex vert0 = proj_verts[0];
    Vertex vert1 = proj_verts[1];
    Vertex vert2 = proj_verts[2];

    std::array<Segment, 3> segments = {Segment(vert0, vert1),
                                       Segment(vert0, vert2),
                                       Segment(vert1, vert2)};

    int yStart = (int)std::ceil(t.m_boundingBox.minY);  // should round up to nearest int
    int yEnd = (int)std::ceil(t.m_boundingBox.maxY);

    // does the bounding box go partially outside the screen? (it wouldn't entirely, because we would have set t.offScreen)
    yStart = std::max(0, yStart);
    yEnd = std::min((int)SCREEN_HEIGHT, yEnd);

    for (int scanline = yStart; scanline < yEnd; scanline++) {
        // for every scanline, check all three segments of the triangle for intersection. have two stack floats to store them
        float xLeft = SCREEN_WIDTH;
        float xRight = 0.f;

        for (const Segment& segment : segments) {
            float xInt;
            if (segment.checkIntersection(scanline, &xInt)) {
                if (t.m_boundingBox.minX <= xInt && xInt <= t.m_boundingBox.maxX) {
                    xLeft = std::min(xLeft, xInt);
                    xRight = std::max(xRight, xInt);
                }
            }
        }

        // fill in all pixels in between xLeft and xRight
        int xStart = (int)std::ceil(xLeft);  // should round up to nearest int
        int xEnd = (int)std::ceil(xRight);
        // the bounding box could have been partially outside the screen, so one of these could be outside as well
        xStart = std::max(0, xStart);
        xEnd = std::min((int)SCREEN_WIDTH, xEnd);


        for (int x_i = xStart; x_i < xEnd; x_i++) {

            // BarycentricWeights bw = ComputeBarycentricWeights(p, t, glm::vec2(x_i, scanline));
            // FOR 3D: compute cand_z using bw
            float z = perspectiveCorrectInterpolateZ(t, proj_verts, glm::vec2(x_i, scanline));

            if (!ConsultAndWriteToZBuffer(x_i, scanline, z)) {
                continue;
            }
            float u = perspectiveCorrectInterpolateAttrib(vert0.m_uv[0],
                                                          vert1.m_uv[0],
                                                          vert2.m_uv[0],
                                                          t,
                                                          proj_verts,
                                                          glm::vec2(x_i, scanline));
            float v = perspectiveCorrectInterpolateAttrib(vert0.m_uv[1],
                                                          vert1.m_uv[1],
                                                          vert2.m_uv[1],
                                                          t,
                                                          proj_verts,
                                                          glm::vec2(x_i, scanline));
            glm::vec4 normal = perspectiveCorrectInterpolateAttrib(vert0.m_normal,
                                                                   vert1.m_normal,
                                                                   vert2.m_normal,
                                                                   t,
                                                                   proj_verts,
                                                                   glm::vec2(x_i, scanline));
            float lambda = glm::clamp(glm::dot(normal, glm::normalize(-m_camera.m_forward)), 0.f, 1.f)*0.7 + 0.3;

            glm::vec3 color = GetImageColor({u,v}, p.mp_texture)*lambda;
            int r = static_cast<int>(std::clamp(std::lround(color[0]), 0l, 255l));
            int g = static_cast<int>(std::clamp(std::lround(color[1]), 0l, 255l));
            int b = static_cast<int>(std::clamp(std::lround(color[2]), 0l, 255l));

            result.setPixelColor(x_i, scanline, QColor(r,g,b));
        }
    }
}


QImage Rasterizer::RenderScene()
{
    resetZBuffer();
    QImage result(512, 512, QImage::Format_RGB32);
    // Fill the image with black pixels.

    result.fill(qRgb(0.f, 0.f, 0.f));

    std::cout << "rerendered" << std::endl;
    // printCamera(m_camera);
    glm::mat4 view_mat = m_camera.viewMatrix();
    glm::mat4 proj_mat = m_camera.perspProjMatrix();


    for (Polygon &p : this->m_polygons) {
        for (const Triangle &t : p.m_tris) {
            // after this, the single triangle will be projected to screen space, its vertices inside proj_verts.
            std::array<Vertex, 3> proj_verts;
            Triangle proj_tri = projectTriangleFromWorldtoPixelSpace(proj_mat, view_mat, p, t, proj_verts);

            // now proj_tri's bounding boxes are initialized
            p.computeBoundingBoxes(proj_tri, proj_verts);

            RenderTriangle(p, proj_tri, proj_verts, result);
        }
    }


    return result;
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}
