#include <iostream>
#include <vector>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include "camera.h"
#include "geometry.h"
#include "gl.h"
#include "program.h"
#include "raycast.h"
#include "delaunay.h"

glm::vec3 s_selected;
glm::vec3 s_center;

GLuint vao;
GLuint vbo;

GLuint p3 = 0;
GLuint vbo_circle[2];
GLuint vao_circle;

GLuint cvbo = 0;
GLuint tvao = 0;
GLuint tvbo = 0;

std::vector<GLfloat> points;
std::vector<GLfloat> tpoints;

delaunay::Triangulation* tria = nullptr;

size_t pidx = 0;

void setup_circle(GLuint p, float r_inner, float r_outer);

void set_selected(glm::vec3& selected, double xpos, double ypos) {
  glm::vec3 from;
  glm::vec3 r = ray::from_mouse(xpos, ypos);

  Camera* c = camera::get_current();
  if (!c) return;

  float distance;
  // Intersect with the xy plane positioned at origin.
  if (!glm::intersectRayPlane(c->m_position,
      r, 
      glm::vec3(0.0f, 0.0f, 0.0f), 
      glm::vec3(0.0f, 0.0f, 1.0f), 
      distance)) {
    return;
  }

  // The intersected point is the defined by p = camera_position + ray * distance.
  glm::vec3 position = c->m_position + r * distance;
  selected = position;
}

void add_point(const glm::vec3& pt) {
  points[pidx++] = pt.x;
  points[pidx++] = pt.y;
  points[pidx++] = pt.z;

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER, (pidx - 3) * sizeof(GLfloat), 3 * sizeof(GLfloat), &pt[0]);

  std::vector<float> pts;
  uint32_t i = 0;
  while (i < pidx) {
    pts.push_back(points[i]);
    pts.push_back(points[i + 1]);
    i += 3;
  }

  tria = delaunay::triangulate(pts);
  std::vector<float> d = tria->get_tris();
  // Make it 3d.
  tpoints.clear();
  tpoints.resize(d.size() + d.size() / 2);

  size_t d3i = 0;
  for (size_t i = 0; i < d.size(); i += 2) {
    tpoints[d3i++] = d[i];
    tpoints[d3i++] = d[i + 1];
    tpoints[d3i++] = 0.0f;
  }

  glBindVertexArray(0);

  glDeleteVertexArrays(1, &tvao);
  glDeleteBuffers(1, &tvbo);


  glGenVertexArrays(1, &tvao);
  glBindVertexArray(tvao);

  glGenBuffers(1, &tvbo);
  glBindBuffer(GL_ARRAY_BUFFER, tvbo);
  glBufferData(GL_ARRAY_BUFFER, tpoints.size() * sizeof(GLfloat), tpoints.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  std::cout << "TEST" << std::endl;
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    set_selected(s_selected, xpos, ypos);

    add_point(s_selected);
  }
  else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    glm::vec3 selected;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    set_selected(selected, xpos, ypos);

    if (tria) {
      std::vector<delaunay::TriNode*> nodes;

      delaunay::Point p(selected.x, selected.y);
      tria->find(p, nodes);

      if (nodes.size() == 1) {
        delaunay::TriNode* f = nodes.front();
        delaunay::Point c;
        float r;
        delaunay::circle(f->m_pts[0], f->m_pts[1], f->m_pts[2], c, r);
        s_center = glm::vec3(c.x, c.y, 0.0f);
        setup_circle(p3, r, r + 0.05f);
      }
    }
  }
}

void setup_circle(GLuint p, float r_inner, float r_outer) {
  GLuint block_index = glGetUniformBlockIndex(p, "blob_settings");

  GLint block_size;
  glGetActiveUniformBlockiv(p, block_index, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size);

  GLubyte* block_buffer;
  block_buffer = (GLubyte*)malloc(block_size);

  const GLchar *names[] = { "blob_settings.inner_color", 
    "blob_settings.outer_color", 
    "blob_settings.radius_inner", 
    "blob_settings.radius_outer"
  };


  GLuint indices[4];

  glGetUniformIndices(p, 4, names, indices);

  GLint offset[4];
  glGetActiveUniformsiv(p, 4, indices, GL_UNIFORM_OFFSET, offset);

  GLfloat outer_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  GLfloat inner_color[] = { 1.0f, 1.0f, 0.75f, 1.0f };
  GLfloat inner_radius = r_inner, outer_radius = r_outer;

  memcpy(block_buffer + offset[0], inner_color, 4 * sizeof(GLfloat));
  memcpy(block_buffer + offset[1], outer_color, 4 * sizeof(GLfloat));
  memcpy(block_buffer + offset[2], &inner_radius, sizeof(GLfloat));
  memcpy(block_buffer + offset[3], &outer_radius, sizeof(GLfloat));

  glDeleteBuffers(1, &cvbo);
  glDeleteBuffers(2, vbo_circle);

  glGenBuffers(1, &cvbo);
  glBindBuffer(GL_UNIFORM_BUFFER, cvbo);
  glBufferData(GL_UNIFORM_BUFFER, block_size, block_buffer, GL_DYNAMIC_DRAW);
  free(block_buffer);

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, cvbo);

  std::vector<GLfloat> c_verts, c_tex;
  geometry::get_circle(c_verts, c_tex, r_outer + 1.0f);

  glGenBuffers(2, vbo_circle);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_circle[0]);
  glBufferData(GL_ARRAY_BUFFER, c_verts.size() * sizeof(GLfloat), c_verts.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_circle[1]);
  glBufferData(GL_ARRAY_BUFFER, c_tex.size() * sizeof(GLfloat), c_tex.data(), GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao_circle);
  glBindVertexArray(vao_circle);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_circle[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_circle[1]);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main() {
  GLFWwindow* window = gl::initialize("Hello Triangle", false, 1280, 720);
  if (!window) {
    std::cout << "Failed to initialize gl context. See logs." << std::endl;
    return 1;
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glfwSetMouseButtonCallback(window, mouse_callback);

  Camera camera(0.1f, 200.0f, 45.0f, static_cast<float>(1280) / 720);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);

  points.resize(3000);
  pidx = 0;

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  program::build("points", {
    {GL_VERTEX_SHADER, "points.vert"},
    {GL_FRAGMENT_SHADER, "points.frag"}
  });

  program::build("tris", {
    {GL_VERTEX_SHADER, "tris.vert"},
    {GL_FRAGMENT_SHADER, "tris.frag"}
  });

  program::build("circle", {
    {GL_VERTEX_SHADER, "circle.vert"},
    {GL_FRAGMENT_SHADER, "circle.frag"}
  });



  glm::mat4 model(1.0f);

  GLuint p = program::get("points");
  GLint view = glGetUniformLocation(p, "view");
  GLint proj = glGetUniformLocation(p, "proj");
  GLint modl = glGetUniformLocation(p, "model");

  GLuint p2 = program::get("tris");
  GLint view2 = glGetUniformLocation(p2, "view");
  GLint proj2 = glGetUniformLocation(p2, "proj");
  GLint modl2 = glGetUniformLocation(p2, "model");

  p3 = program::get("circle");
  GLint view3 = glGetUniformLocation(p3, "view");
  GLint proj3 = glGetUniformLocation(p3, "proj");
  GLint modl3 = glGetUniformLocation(p3, "model");

  setup_circle(p3, 0.35f, 0.40f);
  

  glEnable(GL_PROGRAM_POINT_SIZE);

  while (!glfwWindowShouldClose(window)) {
    static double previous_seconds = glfwGetTime();
    double current_seconds = glfwGetTime();
    double delta_seconds = current_seconds - previous_seconds;
    previous_seconds = current_seconds;

    camera.update(delta_seconds);

    // Draw and stuff.
    gl::update_fps_counter(window);
    // Get framebuffer size.
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, width, height);

    glBindVertexArray(vao);
    glUseProgram(p);
    glUniformMatrix4fv(view, 1, GL_FALSE, &camera.m_view[0][0]);
    glUniformMatrix4fv(proj, 1, GL_FALSE, &camera.m_projection[0][0]);
    glUniformMatrix4fv(modl, 1, GL_FALSE, &model[0][0]);

    glDrawArrays(GL_POINTS, 0, pidx / 3);

    if (tvao) {
      glBindVertexArray(tvao);
      glUseProgram(p2);
      glUniformMatrix4fv(view2, 1, GL_FALSE, &camera.m_view[0][0]);
      glUniformMatrix4fv(proj2, 1, GL_FALSE, &camera.m_projection[0][0]);
      glUniformMatrix4fv(modl2, 1, GL_FALSE, &model[0][0]);

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawArrays(GL_TRIANGLES, 0, tpoints.size());
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    model = glm::translate(model, s_center);

    glBindVertexArray(vao_circle);
    glUseProgram(p3);
    glUniformMatrix4fv(view3, 1, GL_FALSE, &camera.m_view[0][0]);
    glUniformMatrix4fv(proj3, 1, GL_FALSE, &camera.m_projection[0][0]);
    glUniformMatrix4fv(modl3, 1, GL_FALSE, &model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = glm::mat4(1.0f);

    glfwPollEvents();
    glfwSwapBuffers(window);

    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }
  }

  assert(!glGetError());

  glfwTerminate();
  return 0;

}
