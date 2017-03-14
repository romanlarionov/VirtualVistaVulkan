
cmake_minimum_required(VERSION 3.7.0)
project(VirtualVista)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(deps/glfw)

add_subdirectory(deps/SPIRV-Cross)

message(STATUS "Using module to find Vulkan")
find_package(Vulkan)

if(WIN32)
    if(NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan HINTS "$ENV{VULKAN_SDK}/Lib" REQUIRED)
        if(Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
        	message("Using bundled Vulkan library version")
        endif()
    endif()
endif(WIN32)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    
    # for some reason, the MSVC compiler's optimizations executes vital Vulkan commands out of order
    string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/build/")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/build/)
endif()

include_directories("include/"
                    "deps/glfw/include/"
                    "deps/glm/"
                    "deps/SPRIV-Cross/include/"
                    "$ENV{VULKAN_SDK}/Include")

file(GLOB PROJECT_HEADERS include/*.h)
file(GLOB PROJECT_SOURCES src/*.cpp
                          src/Vulkan/*.cpp)
file(GLOB PROJECT_SHADERS assets/shaders/*.comp
                          assets/shaders/*.frag
                          assets/shaders/*.geom
                          assets/shaders/*.vert)

file(GLOB PROJECT_CONFIGS "CMakeLists.txt"
                          "README.md"
                          ".gitignore"
                          ".gitmodules")

source_group("include" FILES ${PROJECT_HEADERS})
source_group("shaders" FILES ${PROJECT_SHADERS})
source_group("src" FILES ${PROJECT_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES}
                               ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS}
                               ${PROJECT_CONFIGS})

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARY} spirv-cross-core spirv-cross-glsl spirv-cross-cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
