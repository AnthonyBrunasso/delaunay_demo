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

GLuint vao;
GLuint vbo;

GLuint tvao = 0;
GLuint tvbo = 0;

std::vector<GLfloat> points;
std::vector<GLfloat> tpoints;
size_t pidx = 0;

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

  std::vector<float> d = delaunay::triangulate(pts);
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


  glm::mat4 model(1.0f);

  GLuint p = program::get("points");
  GLint view = glGetUniformLocation(p, "view");
  GLint proj = glGetUniformLocation(p, "proj");
  GLint modl = glGetUniformLocation(p, "model");

  GLuint p2 = program::get("tris");
  GLint view2 = glGetUniformLocation(p2, "view");
  GLint proj2 = glGetUniformLocation(p2, "proj");
  GLint modl2 = glGetUniformLocation(p2, "model");


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
    }



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
