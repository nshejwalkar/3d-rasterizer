#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <QString>
#include <QImage>
#include <QColor>

// #include "segment.h"

struct BarycentricWeights {
    float s1, s2, s3;
    BarycentricWeights(float, float, float);
};

// A Vertex is a point in space that defines one corner of a polygon.
// Each Vertex has several attributes that determine how they contribute to the
// appearance of their Polygon, such as coloration.
struct Vertex
{
    glm::vec4 m_pos;    // The position of the vertex. In hw02, this is in pixel space.
    glm::vec3 m_color;  // The color of the vertex. X corresponds to Red, Y corresponds to Green, and Z corresponds to Blue.
    glm::vec4 m_normal; // The surface normal of the vertex (not yet used)
    glm::vec2 m_uv;     // The texture coordinates of the vertex (not yet used)

    Vertex() = default;
    Vertex(glm::vec4 p, glm::vec3 c, glm::vec4 n, glm::vec2 u)
        : m_pos(p), m_color(c), m_normal(n), m_uv(u)
    {}

};

class Segment
{
public:
    // Segment() = default;  // this is so Triangle can be default constructed
    Segment(Vertex, Vertex);  // two 2d (4d) points in pixel space. we'll construct this from pos's instead of the whole Vertex

    bool checkIntersection(int, float*) const;  // int is the y-coord of the scanline, float* is a ptr to a stack var that holds x val of intersection if true

private:
    glm::vec2 m_vertex1;
    glm::vec2 m_vertex2;
    float m_slope;
};

// Each Polygon can be decomposed into triangles that fill its area.
struct Triangle
{
    // The indices of the Vertices that make up this triangle.
    // The indices correspond to the std::vector of Vertices stored in the Polygon
    // which stores this Triangle
    unsigned int m_indices[3];

    struct BoundingBox {
        float minX, maxX, minY, maxY;
    };

    BoundingBox m_boundingBox;
    bool offScreen = false;  // depends on the bounding box

    // std::array<Vertex,3> m_verts;
    // the segments/sides of the triangle
    // std::array<Segment,3> m_segments;  // created automatically at initialization

};



class Polygon
{
public:
    // Populate this list of triangles in Triangulate()
    std::vector<Triangle> m_tris;
    // The list of Vertices that define this polygon. This is already filled by the Polygon constructor.
    std::vector<Vertex> m_verts;
    // The above list of triangles, after they've been projected to pixel space. changes every re-render
    std::vector<Triangle> m_proj_tris;
    // Same thing but for verts.
    std::vector<Vertex> m_proj_verts;
    // The name of this polygon, primarily to help you debug
    QString m_name;
    // The image that can be read to determine pixel color when used in conjunction with UV coordinates
    // Not used until homework 3.
    QImage* mp_texture;
    // The image that can be read to determine surface normal offset when used in conjunction with UV coordinates
    // Not used until homework 3
    QImage* mp_normalMap;

    // Polygon class constructors
    Polygon(const QString& name, const std::vector<glm::vec4>& pos, const std::vector<glm::vec3> &col);  // custom
    Polygon(const QString& name, int sides, glm::vec3 color, glm::vec4 pos, float rot, glm::vec4 scale);  // regular
    Polygon(const QString& name);  // the first part of the 3d render steps
    Polygon();
    Polygon(const Polygon& p);  // copy
    ~Polygon();

    // TODO: Complete the body of Triangulate() in polygon.cpp
    // Creates a set of triangles that, when combined, fill the area of this convex polygon.
    void Triangulate();

    // computes the min x, max x, min y, max y for each Triangle in m_tris
    void computeBoundingBoxes(Triangle&) const;
    void computeBoundingBoxes(Triangle&, const std::array<Vertex,3>&) const;

    // Copies the input QImage into this Polygon's texture
    void SetTexture(QImage*);

    // Copies the input QImage into this Polygon's normal map
    void SetNormalMap(QImage*);

    // Various getter, setter, and adder functions
    void AddVertex(const Vertex&);
    void AddTriangle(const Triangle&);
    void ClearTriangles();

    Triangle& TriAt(unsigned int);
    Triangle TriAt(unsigned int) const;

    Vertex& VertAt(unsigned int);
    Vertex VertAt(unsigned int) const;
};

// Returns the color of the pixel in the image at the specified texture coordinates.
// Returns white if the image is a null pointer
glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage* const image);
