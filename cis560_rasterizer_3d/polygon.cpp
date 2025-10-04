#include "polygon.h"
#include <glm/gtx/transform.hpp>

#include "debug.h"
#include "constants.h"
#include <algorithm>
#include <limits>
#include "camera.h"


Segment::Segment(Vertex vert1, Vertex vert2)
    : m_vertex1({vert1.m_pos[0], vert1.m_pos[1]}),
      m_vertex2({vert2.m_pos[0], vert2.m_pos[1]}),
      m_slope(std::fabs(m_vertex2[0]-m_vertex1[0]) < EPS
        ? std::numeric_limits<float>::infinity()
        : (m_vertex2[1]-m_vertex1[1])/(m_vertex2[0]-m_vertex1[0]))
{}

bool Segment::checkIntersection(int y, float* x) const {
    const float x0 = m_vertex1[0], y0 = m_vertex1[1];
    const float x1 = m_vertex2[0], y1 = m_vertex2[1];
    const float dy = y1 - y0;
    const float dx = x1 - x0;

    if (std::fabs(dy) < EPS) {
        return false;
    }

    // protects against the bounding box being some tiny epsilon past true boundary on int
    const float yTop = std::min(y0, y1);
    const float yBot = std::max(y0, y1);
    if (y < (int)std::ceil(yTop) || y >= (int)std::ceil(yBot)) {
        return false;
    }

    if (std::fabs(dx) < EPS) {
        *x = x0;
        return true;  // just return the x value of one endpoint. guaranteed to intersect
    }

    // regular case
    float x_intersect = (((static_cast<float>(y) - y0) / dy) * dx) + x0;
    *x = x_intersect;  // don't bother checking bounding box now, but in caller
    return true;

}

BarycentricWeights::BarycentricWeights(float s1, float s2, float s3)
    : s1(s1), s2(s2), s3(s3)
{}


void Polygon::computeBoundingBoxes(Triangle& t) const {
    t.offScreen = false;

    const glm::vec4& p1 = this->m_verts[t.m_indices[0]].m_pos;
    const glm::vec4& p2 = this->m_verts[t.m_indices[1]].m_pos;
    const glm::vec4& p3 = this->m_verts[t.m_indices[2]].m_pos;

    // these store the floats of the bb sides
    const float minXf = std::min({p1[0], p2[0], p3[0]});
    const float maxXf = std::max({p1[0], p2[0], p3[0]});
    const float minYf = std::min({p1[1], p2[1], p3[1]});
    const float maxYf = std::max({p1[1], p2[1], p3[1]});

    // if any side is outside [0,W) x [0,H), dont render the triangle
    if (maxXf <= 0.f || minXf >= float(SCREEN_WIDTH) || maxYf <= 0.f || minYf >= float(SCREEN_HEIGHT)) {
        t.offScreen = true;
    }

    t.m_boundingBox.minX = minXf;
    t.m_boundingBox.maxX = maxXf;
    t.m_boundingBox.minY = minYf;
    t.m_boundingBox.maxY = maxYf;
}

void Polygon::computeBoundingBoxes(Triangle& t, const std::array<Vertex,3>& pv) const {
    t.offScreen = false;

    const glm::vec4& p1 = pv[0].m_pos;
    const glm::vec4& p2 = pv[1].m_pos;
    const glm::vec4& p3 = pv[2].m_pos;

    // these store the floats of the bb sides
    const float minXf = std::min({p1[0], p2[0], p3[0]});
    const float maxXf = std::max({p1[0], p2[0], p3[0]});
    const float minYf = std::min({p1[1], p2[1], p3[1]});
    const float maxYf = std::max({p1[1], p2[1], p3[1]});

    // if any side is outside [0,W) x [0,H), dont render the triangle
    if (maxXf <= 0.f || minXf >= float(SCREEN_WIDTH) || maxYf <= 0.f || minYf >= float(SCREEN_HEIGHT)) {
        t.offScreen = true;
    }

    t.m_boundingBox.minX = minXf;
    t.m_boundingBox.maxX = maxXf;
    t.m_boundingBox.minY = minYf;
    t.m_boundingBox.maxY = maxYf;
}

void Polygon::Triangulate()
{
    int num_tris = this->m_verts.size() - 2;  // for an n-polygon, need n-2 triangles
    // vertices are guaranteed to be correct by constructor, custom or regular
    // remember, Triangle stores indices of vertices
    this->m_tris.clear();
    this->m_tris.reserve(num_tris);
    for (int i=1; i<=num_tris; ++i) {
        Triangle t;  // this can be done in a constructor
        t.m_indices[0] = 0;
        t.m_indices[1] = i;
        t.m_indices[2] = i+1;

        this->m_tris.push_back(t);
    }
}

glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage* const image)
{
    if(image)
    {
        int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
        int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
        QColor color = image->pixel(X, Y);
        return glm::vec3(color.red(), color.green(), color.blue());
    }
    return glm::vec3(255.f, 255.f, 255.f);
}


// Creates a polygon from the input list of vertex positions and colors
Polygon::Polygon(const QString& name, const std::vector<glm::vec4>& pos, const std::vector<glm::vec3>& col)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    for(unsigned int i = 0; i < pos.size(); i++)
    {
        m_verts.push_back(Vertex(pos[i], col[i], glm::vec4(), glm::vec2()));
    }
    Triangulate();
    for (Triangle& t : m_tris) {
        computeBoundingBoxes(t);
    }
}

// Creates a regular polygon with a number of sides indicated by the "sides" input integer.
// All of its vertices are of color "color", and the polygon is centered at "pos".
// It is rotated about its center by "rot" degrees, and is scaled from its center by "scale" units
Polygon::Polygon(const QString& name, int sides, glm::vec3 color, glm::vec4 pos, float rot, glm::vec4 scale)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    glm::vec4 v(0.f, 1.f, 0.f, 1.f);
    float angle = 360.f / sides;
    for(int i = 0; i < sides; i++)
    {
        glm::vec4 vert_pos = glm::translate(glm::vec3(pos.x, pos.y, pos.z))
                           * glm::rotate(rot, glm::vec3(0.f, 0.f, 1.f))
                           * glm::scale(glm::vec3(scale.x, scale.y, scale.z))
                           * glm::rotate(i * angle, glm::vec3(0.f, 0.f, 1.f))
                           * v;
        m_verts.push_back(Vertex(vert_pos, color, glm::vec4(), glm::vec2()));
    }

    Triangulate();
    for (Triangle& t : m_tris) {
        computeBoundingBoxes(t);
    }
}

Polygon::Polygon(const QString &name)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon()
    : m_tris(), m_verts(), m_name("Polygon"), mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon(const Polygon& p)
    : m_tris(p.m_tris), m_verts(p.m_verts), m_name(p.m_name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    if(p.mp_texture != nullptr)
    {
        mp_texture = new QImage(*p.mp_texture);
    }
    if(p.mp_normalMap != nullptr)
    {
        mp_normalMap = new QImage(*p.mp_normalMap);
    }
}

Polygon::~Polygon()
{
    delete mp_texture;
}

void Polygon::SetTexture(QImage* i)
{
    mp_texture = i;
}

void Polygon::SetNormalMap(QImage* i)
{
    mp_normalMap = i;
}

void Polygon::AddTriangle(const Triangle& t)
{
    m_tris.push_back(t);
}

void Polygon::AddVertex(const Vertex& v)
{
    m_verts.push_back(v);
}

void Polygon::ClearTriangles()
{
    m_tris.clear();
}

Triangle& Polygon::TriAt(unsigned int i)
{
    return m_tris[i];
}

Triangle Polygon::TriAt(unsigned int i) const
{
    return m_tris[i];
}

Vertex &Polygon::VertAt(unsigned int i)
{
    return m_verts[i];
}

Vertex Polygon::VertAt(unsigned int i) const
{
    return m_verts[i];
}
