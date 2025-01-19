# add gl3w
add_subdirectory(gl3w)

# add glfw
add_subdirectory(glfw)
target_include_directories(${_EXE_NAME_} PUBLIC "glfw/include")
target_link_libraries(${_EXE_NAME_} glfw OpenGL::GL)

# add imgui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_include_directories(imgui PUBLIC ${IMGUI_DIR}/backends)

# add glm
add_subdirectory(glm)

# link libraries
target_link_libraries(imgui glfw gl3w)
target_link_libraries(${_EXE_NAME_} imgui glfw OpenGL::GL gl3w glm)