file(GLOB Sources "*.cpp" "*.c" "../include/*.h")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("glfw-3.2")
include_directories("./")
find_package(OpenGL REQUIRED)

add_executable(delaunay ${Sources})

include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(delaunay glfw)
target_link_libraries(delaunay ${OPENGL_LIBRARIES})
