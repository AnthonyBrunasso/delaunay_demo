#include "geometry.h"

#include <cmath>

void geometry::get_triangle(std::vector<GLfloat>& vertices
    , std::vector<GLfloat>& normals) {
  vertices = geometry::get_triangle();
  normals = {
    0.0f, 0.0f, 1.0f
  };
}

std::vector<GLfloat>& geometry::get_triangle() {
  static std::vector<GLfloat> triangle = {
    0.0f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
  };
  return triangle;
}

std::vector<GLfloat>& geometry::get_square() {
  static std::vector<GLfloat> square = {
    -0.5f, -0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
    0.5f,  0.5f,  0.0f,
    0.5f,  0.5f,  0.0f,
    -0.5f,  0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f
  };
  return square;
}
void geometry::get_square(std::vector<GLfloat>& vertices
    , std::vector<GLfloat>& normals) {
  vertices = geometry::get_square();
  normals = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
  };
}

std::vector<GLfloat>& geometry::get_squaretexcoords() {
  static std::vector<GLfloat> square = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
  };
  return square;
}

std::vector<GLfloat> geometry::get_hexagontexcoords() {
  std::vector<GLfloat> texcoords;
  // 30 degrees.
  GLfloat x = 0.5f * tan(30.0f * 3.14159265f / 180.0f);
  GLfloat d = 1.0f - x;

  // Tri 1
  texcoords.push_back(0.5f);
  texcoords.push_back(0.5f);

  texcoords.push_back(0.5f);
  texcoords.push_back(0.0f);

  texcoords.push_back(1.0f);
  texcoords.push_back(x);

  // Tri 2
  texcoords.push_back(0.5f);
  texcoords.push_back(0.5f);

  texcoords.push_back(1.0f);
  texcoords.push_back(x);

  texcoords.push_back(1.0f);
  texcoords.push_back(d);

  // Tri 3
  texcoords.push_back(0.5f);
  texcoords.push_back(0.5f);

  texcoords.push_back(1.0f);
  texcoords.push_back(d);

  texcoords.push_back(0.5f);
  texcoords.push_back(1.0f);

  // Tri 4
  texcoords.push_back(0.5f);
  texcoords.push_back(0.5f);

  texcoords.push_back(0.5f);
  texcoords.push_back(1.0f);

  texcoords.push_back(0.0f);
  texcoords.push_back(d);

  // Tri 5
  texcoords.push_back(0.5f);
  texcoords.push_back(0.5f);

  texcoords.push_back(0.0f);
  texcoords.push_back(d);

  texcoords.push_back(0.0f);
  texcoords.push_back(x);

  // Tri 6
  texcoords.push_back(0.5f);
  texcoords.push_back(0.5f);

  texcoords.push_back(0.0f);
  texcoords.push_back(x);

  texcoords.push_back(0.5f);
  texcoords.push_back(0.0f);

  return texcoords;
}

void geometry::get_cube(std::vector<GLfloat>& verts
    , std::vector<GLfloat>& normals
    , std::vector<GLuint>& indices) {

  verts = {1.0f, 1.0f, 1.0f,      // 0
           1.0f, -1.0f, 1.0f,     // 1
           -1.0f, 1.0f, 1.0f,     // 2
           -1.0f, -1.0f, 1.0f,    // 3
           1.0f, 1.0f, -1.0f,     // 4
           -1.0f, 1.0f, -1.0f,    // 5
           1.0f, -1.0f, -1.0f,    // 6
           -1.0f, -1.0f, -1.0f};  // 7

  normals = {1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f};

  indices = {0, 1, 2, 3, 2, 1, // Front face
             0, 4, 1, 4, 6, 1, // Right face
             4, 5, 7, 4, 7, 6,// Back face
             2, 5, 7, 2, 6, 3,// Left face
             6, 4, 0, 5, 0, 2,// Top face
             3, 1, 6, 3, 6, 7}; // Bottom face
          
}

void geometry::get_circle(std::vector<GLfloat>& verts, std::vector<GLfloat>& tex) {
  verts = {
    -0.8f, -0.8f, 0.0f,
    0.8f, -0.8f, 0.0f,
    0.8f,  0.8f, 0.0f,
    -0.8f, -0.8f, 0.0f,
    0.8f, 0.8f, 0.0f,
    -0.8f, 0.8f, 0.0f
  };

  tex = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
  };
}
