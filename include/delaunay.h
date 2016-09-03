#pragma once

#include <vector>
#include <set>

namespace delaunay {
  struct Point {
    Point() : x(0), y(0) {};
    Point(float x, float y) : x(x), y(y) {};
    float x;
    float y;
  };

  struct TriNode {
    // Vertices of the triangle.
    Point m_pts[3];
    TriNode* m_children[3];

    TriNode(const Point& p1,
      const Point& p2,
      const Point& p3) {
      m_pts[0] = p1;
      m_pts[1] = p2;
      m_pts[2] = p3;

      m_children[0] = nullptr;
      m_children[1] = nullptr;
      m_children[2] = nullptr;
    };
  };

  class Triangulation {
  public:
    Triangulation(const Point& p1,
      const Point& p2,
      const Point p3,
      const std::vector<Point>& ps);

    ~Triangulation();

    TriNode* insert(const Point& pt);

    TriNode* split(const Point& p1, const Point& p2);

    std::vector<float> get_tris();

    // Finds the leaf nodes of the tree the point is contained in.
    // A point could be contained in many nodes if it is already an existing vertex.
    void find(const Point& pt, std::vector<TriNode*>& nodes);

    void get_triangulation(TriNode*& node,
      std::vector<float>& tris,
      std::set<TriNode*>& visited);

  private:
    // Finds the leaf nodes of the tree the point is contained in.
    // A point could be contained in many nodes if it is already an existing vertex.
    void find(const Point& pt, TriNode* node, std::vector<TriNode*>& nodes);

    // Find all triangles containing both p1 and p2.
    void find_by_edge(const Point& p1,
      const Point& p2,
      TriNode* node,
      std::vector<TriNode*>& nodes);

    void recursive_delete(TriNode*& node);

    TriNode* m_root;
    std::vector<Point> m_points;
  };

  void circle(const Point& a, 
    const Point& b, 
    const Point& c, 
    Point& center, 
    float& radius);

  Triangulation* triangulate(const std::vector<float>& points);
}
