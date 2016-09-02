#pragma once

#include <vector>
#include <set>

namespace delaunay {
  // Defined in delaunay.cpp
  struct TriNode;

  struct Point {
    Point() : x(0), y(0) {};
    Point(float x, float y) : x(x), y(y) {};
    float x;
    float y;
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
    void find(const Point& pt, TriNode* node, std::vector<TriNode*>& nodes);

    void get_triangulation(TriNode*& node,
      std::vector<float>& tris,
      std::set<TriNode*>& visited);

  private:
    // Find all triangles containing both p1 and p2.
    void find_by_edge(const Point& p1,
      const Point& p2,
      TriNode* node,
      std::vector<TriNode*>& nodes);

    void recursive_delete(TriNode*& node);

    TriNode* m_root;
    std::vector<Point> m_points;
  };

  std::vector<float> triangulate(const std::vector<float>& points);
}
