cmake_minimum_required(VERSION 3.9.2)
project (vulkan-hello-triangle VERSION 0.0.1 DESCRIPTION "vulkan hello triangle")

if (MSVC)
  add_compile_options(/W4 /permissive-)
endif()

# Dependencies
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_program(GLSLANG_VALIDATOR NAMES glslangValidator)

add_subdirectory(third_party/fmt-6.0.0 EXCLUDE_FROM_ALL)

# Our program
add_executable(vulkan-hello-triangle source/main.cpp source/Application.cpp)
target_link_libraries(vulkan-hello-triangle PRIVATE fmt-header-only glfw glm Vulkan::Vulkan)
target_compile_features(vulkan-hello-triangle PUBLIC cxx_std_17)

set_target_properties(vulkan-hello-triangle PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Compile shaders (on every build)
set(shaders_src_dir ${PROJECT_SOURCE_DIR}/source/shaders)
set(shaders_dst_dir shaders)
add_custom_command(
  TARGET vulkan-hello-triangle POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${shaders_dst_dir}
  COMMAND ${GLSLANG_VALIDATOR} -V -o ${shaders_dst_dir}/vert.spv ${shaders_src_dir}/shader.vert
  DEPENDS shader.vert ${GLSLANG_VALIDATOR}
  COMMAND ${GLSLANG_VALIDATOR} -V -o ${shaders_dst_dir}/frag.spv ${shaders_src_dir}/shader.frag
  DEPENDS shader.frag ${GLSLANG_VALIDATOR}
)
