cmake_minimum_required(VERSION 3.9.2)
project (vulkan-hello-triangle VERSION 0.0.1 DESCRIPTION "vulkan hello triangle")

if (MSVC)
  add_compile_options(/W4 /permissive-)
endif()

add_subdirectory(third_party/fmt-6.0.0 EXCLUDE_FROM_ALL)

add_executable(vulkan-hello-triangle source/main.cpp)
target_link_libraries(vulkan-hello-triangle PRIVATE fmt-header-only)
target_compile_features(vulkan-hello-triangle PUBLIC cxx_std_17)

set_target_properties(vulkan-hello-triangle PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
