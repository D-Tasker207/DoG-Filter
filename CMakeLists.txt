cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(DoG-Interactive VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CXX_FLAGS "-Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -fsanitize=address -g")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS} -fsanitize=address")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

file(GLOB SOURCES
 "src/*.cpp"
 "src/**/*.cpp"
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_library(glad STATIC ${PROJECT_SOURCE_DIR}/src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PRIVATE ${PROJECT_SOURCE_DIR}/include)

find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(${PROJECT_NAME} glad ${OPENGL_LIBRARIES} glfw glm::glm-header-only)