#pragma once

#include <vector>
#include <GL/gl3w.h>

namespace geometry {
  std::vector<GLfloat>& get_triangle();
  void get_triangle(std::vector<GLfloat>& vertices
    , std::vector<GLfloat>& normals);

  std::vector<GLfloat>& get_square();
  std::vector<GLfloat>& get_squaretexcoords();
  void get_square(std::vector<GLfloat>& vertices
    , std::vector<GLfloat>& normals);

  std::vector<GLfloat> get_hexagontexcoords();

  void get_cube(std::vector<GLfloat>& verts
    , std::vector<GLfloat>& normals
    , std::vector<GLuint>& indices);

  void get_circle(std::vector<GLfloat>& verts, std::vector<GLfloat>& tex, float sz);
}
