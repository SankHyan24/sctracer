set(_PROJECT_NAME_ "sc path tracer") # project name
set(_EXE_NAME_ "sctracer") # target name
set(_SRC_FILE_NAME_ "src") # source file location

cmake_minimum_required(VERSION 3.15)
project(${_PROJECT_NAME_} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    message(STATUS "using MSVC")
    message(STATUS "warning: If u use other compiler, please modify the CMakeLists.txt")
endif()

find_package(OpenGL REQUIRED)

add_executable(${_EXE_NAME_})
aux_source_directory(${_SRC_FILE_NAME_} _SOURCE_)
target_sources(${_EXE_NAME_} PUBLIC ${_SOURCE_})
target_include_directories(${_EXE_NAME_} PUBLIC "./include")
add_subdirectory(${_SRC_FILE_NAME_})

# add thirdparty
add_subdirectory(thirdparty)

# copy before build
add_custom_command(TARGET ${_EXE_NAME_} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${_EXE_NAME_}>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${_EXE_NAME_}>/shaders
)