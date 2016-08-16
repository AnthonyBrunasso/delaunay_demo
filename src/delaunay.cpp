#include "delaunay.h"

#include <cmath>
#include <set>

namespace delaunay {

struct Point {
  Point() : x(0), y(0) {};
  Point(float x, float y) : x(x), y(y) {};
  float x;
  float y;
};

Point b1(0.0f, 5.0f);
Point b2(-5.0f, -5.0f);
Point b3(5.0f, -5.0f);

struct TriNode {
  TriNode(const Point& p1, 
      const Point& p2, 
      const Point& p3) {
    m_pts[0] = p1;
    m_pts[1] = p2;
    m_pts[2] = p3;

    m_children[0] = nullptr; 
    m_children[1] = nullptr; 
    m_children[2] = nullptr;
  }
  // Vertices of the triangle.
  Point m_pts[3];
  TriNode* m_children[3];
};

Point sub(const Point& p1, const Point& p2) {
  return Point(p1.x - p2.x, p1.y - p2.y);
}

float dot(const Point& p1, const Point& p2) {
  return p1.x * p2.x + p1.y * p2.y;
}

// Default epsilon assumes two points of distance .001 apart can be considered the same.
bool equal(const Point& p1, const Point& p2, float epsilon=0.001f) {
  return fabs(p1.x - p2.x) < epsilon && fabs(p1.y - p2.y) < epsilon;
}

bool vert_in(const Point& p, Point* pts) {
  return equal(p, pts[0]) || equal(p, pts[1]) || equal(p, pts[2]);
}

bool point_in_tri(const Point& pt, Point* pts) {
  Point v0 = sub(pts[2], pts[0]);
  Point v1 = sub(pts[1], pts[0]);
  Point v2 = sub(pt, pts[0]);

  float dot00 = dot(v0, v0);
  float dot01 = dot(v0, v1);
  float dot02 = dot(v0, v2);
  float dot11 = dot(v1, v1);
  float dot12 = dot(v1, v2);

  float inv_d = 1.0f / (dot00 * dot11 - dot01 * dot01);
  float u = (dot11 * dot02 - dot01 * dot12) * inv_d;
  float v = (dot00 * dot12 - dot01 * dot02) * inv_d;

  return (u >= 0) && (v >= 0) && (u + v < 1);
}

bool point_in_circle(const Point& pt,
    const Point& a,
    const Point& b,
    const Point& c) {
  // http://paulbourke.net/geometry/circlesphere/ 
  float ma = (b.y - a.y) /  (b.x - a.x);
  float mb = (c.y - b.y) /  (c.x - b.x);
  float x = (ma * mb * (a.y - c.y) + mb * (a.x + b.x) - ma * (b.x + c.x)) / (2 * (mb - ma));
  float y = (-1.0f / ma) * (x - (a.x + b.x) / 2.0f) + ((a.y + b.y) / 2.0f);

  Point cen(x, y);
  Point r_diff = sub(a, cen);
  float dot_diff = dot(r_diff, r_diff);
  float r_squared = dot_diff * dot_diff;
  Point p_diff = sub(pt, cen);
  dot_diff = dot(p_diff, p_diff);
  float p_squared = dot_diff * dot_diff;

  return p_squared < r_squared;
}

class Triangulation {
public:
  Triangulation(const Point& p1, const Point& p2, const Point p3) {
    m_root = new TriNode(p1, p2, p3);
  }

  ~Triangulation() {
    //recursive_delete(m_root);
  }

  TriNode* insert(const Point& pt) {
    std::vector<TriNode*> nodes;
    find(pt, m_root, nodes);
    // There should only be a single triangle containing this point, otherwise
    // it was likely an existing vertex.
    if (nodes.size() != 1) return nullptr;
    TriNode* node = nodes.front();
    // Create three new triangles with the given point.
    node->m_children[0] = new TriNode(pt, node->m_pts[0], node->m_pts[1]);
    node->m_children[1] = new TriNode(pt, node->m_pts[1], node->m_pts[2]);
    node->m_children[2] = new TriNode(pt, node->m_pts[2], node->m_pts[0]);

    return node;
  }

  TriNode* split(const Point& p1, const Point& p2) {
    std::vector<TriNode*> nodes;
    find_by_edge(p1, p2, m_root, nodes);
    // We can only split a convex quadrilateral.
    if (nodes.size() != 2) return nullptr;
    TriNode* n1 = nodes[0];
    TriNode* n2 = nodes[1];

    Point p3, p4;
    for (int i = 0; i < 3; ++i) {
      if (!vert_in(n2->m_pts[i], n1->m_pts)) {
        p3 = n2->m_pts[i]; 
      }

      if (!vert_in(n1->m_pts[i], n2->m_pts)) {
        p4 = n1->m_pts[i]; 
      }
    }

    // Check if a split needs to happen.
    if (!point_in_circle(p4, n2->m_pts[0], n2->m_pts[1], n2->m_pts[2]) && 
        !point_in_circle(p3, n1->m_pts[0], n1->m_pts[1], n1->m_pts[2])) {
      return nullptr;
    }

    TriNode* t1 = new TriNode(p1, p3, p4);
    TriNode* t2 = new TriNode(p2, p4, p3);

    n1->m_children[0] = n2->m_children[0] = t1;
    n1->m_children[1] = n2->m_children[1] = t2;

    return n1;
  }

  std::vector<float> get_tris() {
    std::vector<float> tris;
    std::set<TriNode*> visited;
    get_triangulation(m_root, tris, visited);
    return tris;
  }

private:
  // Finds the leaf nodes of the tree the point is contained in.
  // A point could be contained in many nodes if it is already an existing vertex.
  void find(const Point& pt, TriNode* node, std::vector<TriNode*>& nodes) {
    bool contains = vert_in(pt, node->m_pts) || point_in_tri(pt, node->m_pts);
    if (!contains) return;
    // Reset contains, it is used to detect if the current node is a leaf node.
    contains = false;

    // Loop through all the children, if none exist it is a leaf node, otherwise
    // recurse into that child.
    for (int i = 0; i < 3; ++i) {
      if (node->m_children[i]) {
        contains = true;
        find(pt, node->m_children[i], nodes);
      }
    }

    // If this node was a leaf add it to the list.
    if (!contains) nodes.push_back(node);
  }

  // Find all triangles containing both p1 and p2.
  void find_by_edge(const Point& p1,
      const Point& p2,
      TriNode* node,
      std::vector<TriNode*>& nodes) {
    std::vector<TriNode*> n1;
    find(p1, m_root, n1);
    for (auto& n : n1) {
      if (vert_in(p1, n->m_pts) && vert_in(p2, n->m_pts)) {
        nodes.push_back(n);
      }
    }
  }

  void get_triangulation(TriNode*& node, 
      std::vector<float>& tris, 
      std::set<TriNode*>& visited) {
    if (!node) return;
    bool recursed = false;
    for (int i = 0; i < 3; ++i) {
      if (node->m_children[i]) {
        get_triangulation(node->m_children[i], tris, visited);
        recursed = true;
      }
    }

    // If this is a leaf node and it hasn't been added to the tris list.
    //if (!recursed && visited.find(node) == visited.end()) {
    if (!recursed && visited.find(node) == visited.end()
        && !vert_in(b1, node->m_pts) && !vert_in(b2, node->m_pts) && !vert_in(b3, node->m_pts)) {
      visited.insert(node);
      for (int i = 0; i < 3; ++i) {
        tris.push_back(node->m_pts[i].x);
        tris.push_back(node->m_pts[i].y);
      }
    }
  }

  void recursive_delete(TriNode*& node) {
    if (!node) return;

    for (int i = 0; i < 3; ++i) {
      if (node->m_children[i]) recursive_delete(node->m_children[i]);
    }

    delete node;
    node = nullptr;
  }

  TriNode* m_root;
};

void legalize_edge(const Point& p1, const Point& p2, const Point& p3, Triangulation& tria) {
  TriNode* split = tria.split(p2, p3);
  if (!split) return;
  Point* pts1 = split->m_children[0]->m_pts;
  Point* pts2 = split->m_children[1]->m_pts;
  legalize_edge(p1, pts1[1], pts1[2], tria);
  legalize_edge(p1, pts2[1], pts2[2], tria);
}

}

std::vector<float> delaunay::triangulate(std::vector<float> points) {
  Triangulation tria(b1, b2, b3);
  for (size_t i = 0; i < points.size(); i += 2) {
    Point pt(points[i], points[i + 1]);
    TriNode* inserted = tria.insert(pt);

    if (!inserted) continue;

    // Legalize each new edge.
    Point* pts1 = inserted->m_children[0]->m_pts;
    Point* pts2 = inserted->m_children[1]->m_pts;
    Point* pts3 = inserted->m_children[2]->m_pts;
    legalize_edge(pt, pts1[1], pts1[2], tria);
    legalize_edge(pt, pts2[1], pts2[2], tria);
    legalize_edge(pt, pts3[1], pts3[2], tria);
  }
  return tria.get_tris();
}
