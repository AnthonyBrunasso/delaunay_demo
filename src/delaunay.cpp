#include "delaunay.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>

namespace delaunay {


// Temp.
Point b1(20.0f, -20.0f);
Point b2(-20.0f, -20.0f);

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
  Point p_diff = sub(pt, cen);
  float p_dot = dot(p_diff, p_diff);

  return p_dot < dot_diff;
}

Triangulation::Triangulation(const Point& p1, 
    const Point& p2, 
    const Point p3, 
    const std::vector<Point>& ps) : m_points(ps) {
  m_root = new TriNode(p1, p2, p3);
}

Triangulation::~Triangulation() {
  //recursive_delete(m_root);
}

TriNode* Triangulation::insert(const Point& pt) {
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

TriNode* Triangulation::split(const Point& p1, const Point& p2) {
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

  std::cout << "FLIPPING" << std::endl;

  TriNode* t1 = new TriNode(p1, p3, p4);
  TriNode* t2 = new TriNode(p2, p4, p3);

  n1->m_children[0] = n2->m_children[0] = t1;
  n1->m_children[1] = n2->m_children[1] = t2;

  return n1;
}

std::vector<float> Triangulation::get_tris() {
  std::vector<float> tris;
  std::set<TriNode*> visited;
  get_triangulation(m_root, tris, visited);
  return tris;
}

void Triangulation::find(const Point& pt, std::vector<TriNode*>& nodes) {
  find(pt, m_root, nodes);
}

void Triangulation::find(const Point& pt, TriNode* node, std::vector<TriNode*>& nodes) {
  bool contains = vert_in(pt, node->m_pts) || point_in_tri(pt, node->m_pts);
  if (!contains) return;
  contains = false;

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
void Triangulation::find_by_edge(const Point& p1,
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

void Triangulation::get_triangulation(TriNode*& node,
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
  if (!recursed && visited.find(node) == visited.end()) {
  //if (!recursed && visited.find(node) == visited.end()
  //    && !vert_in(b1, node->m_pts) && !vert_in(b2, node->m_pts)) {
    visited.insert(node);
    for (int i = 0; i < 3; ++i) {
      tris.push_back(node->m_pts[i].x);
      tris.push_back(node->m_pts[i].y);
    }
  }
}

void Triangulation::recursive_delete(TriNode*& node) {
  if (!node) return;

  for (int i = 0; i < 3; ++i) {
    if (node->m_children[i]) recursive_delete(node->m_children[i]);
  }

  delete node;
  node = nullptr;
}

void legalize_edge(const Point& p1, const Point& p2, const Point& p3, Triangulation& tria) {
  TriNode* split = tria.split(p2, p3);
  if (!split) return;
  Point* pts1 = split->m_children[0]->m_pts;
  Point* pts2 = split->m_children[1]->m_pts;
  legalize_edge(p1, pts1[1], pts1[2], tria);
  legalize_edge(p1, pts2[1], pts2[2], tria);
}

}

void delaunay::circle(const Point& a,
    const Point& b,
    const Point& c, 
    Point& center,
    float& radius) {
  float ma = (b.y - a.y) / (b.x - a.x);
  float mb = (c.y - b.y) / (c.x - b.x);
  float x = (ma * mb * (a.y - c.y) + mb * (a.x + b.x) - ma * (b.x + c.x)) / (2 * (mb - ma));
  float y = (-1.0f / ma) * (x - (a.x + b.x) / 2.0f) + ((a.y + b.y) / 2.0f);

  center = Point(x, y);
  Point d = sub(a, c);

  Point r_diff = sub(a, center);
  float dot_diff = dot(r_diff, r_diff);

  radius = sqrtf(dot_diff);
}

delaunay::Triangulation* delaunay::triangulate(const std::vector<float>& points) {
  // Find max point.
  Point max(-FLT_MAX, -FLT_MAX);
  for (size_t i = 0; i < points.size(); i += 2) {
    if (points[i + 1] > max.y) max = Point(points[i], points[i + 1]);
  }

  std::vector<Point> ps((points.size() / 2) - 1);
  size_t j = 0;
  for (size_t i = 0; i < points.size(); i += 2) {
    Point pt(points[i], points[i + 1]);
    if (!equal(pt, max)) ps[j++] = pt;
  }

  Point bounds[3] = { b1, b2, max };
  Triangulation* tria = new Triangulation(b1, b2, max, ps);

  //std::random_shuffle(ps.begin(), ps.end());
  for (size_t i = 0; i < ps.size(); ++i) {
    Point& pt = ps[i];
    TriNode* inserted = tria->insert(pt);

    if (!inserted) continue;

    // Legalize each new edge.
    Point* pts1 = inserted->m_children[0]->m_pts;
    Point* pts2 = inserted->m_children[1]->m_pts;
    Point* pts3 = inserted->m_children[2]->m_pts;

    if (!vert_in(pts1[1], bounds) || !vert_in(pts1[2], bounds)) {
      legalize_edge(pt, pts1[1], pts1[2], *tria);
    }

    if (!vert_in(pts2[1], bounds) || !vert_in(pts2[2], bounds)) {
      legalize_edge(pt, pts2[1], pts2[2], *tria);
    }

    if (!vert_in(pts3[1], bounds) || !vert_in(pts3[2], bounds)) {
      legalize_edge(pt, pts3[1], pts3[2], *tria);
    }
  }
  return tria;
}
