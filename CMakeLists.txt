cmake_minimum_required(VERSION 3.14)

set(APPLICATION_NAME "VulkanApp")
project(${APPLICATION_NAME} VERSION 1.0.0)

set (PROJECT_HEADER_FILES

    ./application/include/renderer.hpp
	./application/include/window.hpp
	./application/include/debugManager.hpp
	./application/include/validationLayersConfig.hpp
	./application/include/extensions.hpp
	./application/include/instance.hpp
	./application/include/device.hpp
	./application/include/queueFamilyIndices.hpp
	./application/include/swapChain.hpp
	./application/include/renderPass.hpp
	./application/include/shaderManager.hpp
	./application/include/pipeline.hpp
	./application/include/vertex.hpp
	./application/include/commandPool.hpp
	./application/include/config.hpp
	./application/include/bufferUtils.hpp
	./application/include/mesh.hpp
	./application/include/descriptorLayout.hpp
	./application/include/uniformBuffers.hpp
	./application/include/texture.hpp
	./application/include/imageUtils.hpp

)
set (PROJECT_SOURCE_FILES
    ./application/src/main.cpp
	./application/src/renderer.cpp
)


# Add excutable target and include directory
add_executable(${APPLICATION_NAME} ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES} ${SCRIPT_SOURCE_FILES} ${SCRIPT_HEADER_FILES})
target_include_directories(${APPLICATION_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/application/include")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APPLICATION_NAME})
set_target_properties(${APPLICATION_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}../application/src" PREFIX "src" FILES ${PROJECT_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}../application/include" PREFIX "include" FILES ${PROJECT_HEADER_FILES})



include(FetchContent)

# ========== Vulkan ==========
find_package(Vulkan REQUIRED)


# ========== GLM ===========
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY ON CACHE BOOL "" FORCE)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 		1.0.1
)

# ========== GLFW =============
FetchContent_Declare(
	glfw 
	GIT_REPOSITORY	https://github.com/glfw/glfw.git 
	GIT_TAG			3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


FetchContent_MakeAvailable(glm glfw)

# Use C++20 everywhere
set_target_properties(${APPLICATION_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(glm PROPERTIES CXX_STANDARD 20)
set_target_properties(glfw PROPERTIES CXX_STANDARD 20)

target_link_libraries(${APPLICATION_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${APPLICATION_NAME} PRIVATE glm)
target_link_libraries(${APPLICATION_NAME} PRIVATE glfw)

set_target_properties(glm PROPERTIES FOLDER "GLM")


# ======= STB Image Setup =======
set(STB_IMAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/stb_image")
set(STB_IMAGE_INCLUDE_DIR "${STB_IMAGE_DIR}/stbImage")
file(MAKE_DIRECTORY "${STB_IMAGE_INCLUDE_DIR}")

# Download stb_image.h
file(DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    "${STB_IMAGE_INCLUDE_DIR}/stb_image.h"
    STATUS _stb_status
    TLS_VERIFY ON
    LOG _stb_log
)

list(GET _stb_status 0 _stb_status_code)
if(NOT _stb_status_code EQUAL 0)
    message(FATAL_ERROR "Failed to download stb_image.h:\n${_stb_log}")
endif()

# Create stb_image.cpp (implementation file)
file(WRITE "${STB_IMAGE_INCLUDE_DIR}/stb_image.cpp" "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"")

# Create a static library
add_library(stb_image_impl STATIC "${STB_IMAGE_INCLUDE_DIR}/stb_image.cpp")
target_include_directories(stb_image_impl PUBLIC "${STB_IMAGE_INCLUDE_DIR}")

# Link it to your app
target_link_libraries(${APPLICATION_NAME} PRIVATE stb_image_impl)

# Organize in Visual Studio
set_target_properties(stb_image_impl PROPERTIES FOLDER "stb_image")
