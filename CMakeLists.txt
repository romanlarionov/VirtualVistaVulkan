
cmake_minimum_required(VERSION 3.7.0)
project(VirtualVista)

set(SHADER_DIR "assets/shaders")
set(SRC_DIR "src")
set(INCLUDE_DIR "include")
set(DEPS_DIR "deps")

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(${DEPS_DIR}/glfw)

add_subdirectory(${DEPS_DIR}/SPIRV-Cross)

message(STATUS "Using module to find Vulkan")
find_package(Vulkan)

if(NOT Vulkan_FOUND)
    message(STATUS "Vulkan not found through module. Attempting to manually find.")
    if(WIN32)
        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan HINTS "$ENV{VULKAN_SDK}/Lib/" REQUIRED)
        if(Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            message(STATUS "Using bundled Vulkan library version.")
        endif()
    endif(WIN32)
endif()

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library! Maybe you forgot to reboot after changing environment variables or Vulkan update.")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    add_definitions( -DROOTPROJECTDIR="${CMAKE_SOURCE_DIR}" )
    # for some reason, the MSVC compiler's optimizations executes vital Vulkan commands out of order
    string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif()

include_directories("include/"
                    "deps/glfw/include/"
                    "deps/glm/"
                    "deps/SPRIV-Cross/include/"
                    "deps/gli/"
                    "deps/include/"
                    "$ENV{VULKAN_SDK}/Include")

file(GLOB PROJECT_HEADERS ${INCLUDE_DIR}/*.h)
file(GLOB PROJECT_VULKAN_SOURCES ${SRC_DIR}/Vulkan/*.cpp)
file(GLOB PROJECT_SOURCES ${SRC_DIR}/*.cpp)
file(GLOB PROJECT_SHADERS ${SHADER_DIR}/*.comp
                          ${SHADER_DIR}/*.frag
                          ${SHADER_DIR}/*.geom
                          ${SHADER_DIR}/*.vert
                          ${SHADER_DIR}/*.tess
                          ${SHADER_DIR}/*.glsl)

file(GLOB PROJECT_CONFIGS "CMakeLists.txt"
                          "README.md"
                          ".gitignore"
                          ".gitmodules"
                          "${SHADER_DIR}/shader_info.txt")

# Vulkan shaders need compiling prior to running
if(MSVC)
    # todo: need check if any of them have changed
    # todo: only recompile if any of them have changed
    foreach(SHADER_FILE ${PROJECT_SHADERS})
        string(FIND ${SHADER_FILE} "." DOT_POSITION)
        string(SUBSTRING ${SHADER_FILE} 0 ${DOT_POSITION} SHADER_NAME)
        math(EXPR DOT_POSITION "${DOT_POSITION}+1")
        string(SUBSTRING ${SHADER_FILE} ${DOT_POSITION} 4 SHADER_EXTENSION)

        message(STATUS "Compiling shader: ${SHADER_NAME}.${SHADER_EXTENSION}")
        execute_process(COMMAND CMD /c $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V -o ${SHADER_NAME}_${SHADER_EXTENSION}.spv ${SHADER_FILE})
    endforeach(SHADER_FILE)
endif()

source_group("include" FILES ${PROJECT_HEADERS})
source_group("shaders" FILES ${PROJECT_SHADERS})
source_group("src" FILES ${PROJECT_SOURCES})
source_group("src\\vulkan" FILES ${PROJECT_VULKAN_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES}
                               ${PROJECT_VULKAN_SOURCES}
                               ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS}
                               ${PROJECT_CONFIGS})

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARY} spirv-cross-core spirv-cross-glsl spirv-cross-cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
