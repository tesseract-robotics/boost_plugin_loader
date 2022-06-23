# Example plugin definition and explict template instantiation of plugin loader for test plugin class
add_library(${PROJECT_NAME}_example_plugin plugin.cpp)
target_link_libraries(${PROJECT_NAME}_example_plugin PUBLIC ${PROJECT_NAME})
target_cxx_version(${PROJECT_NAME}_example_plugin PUBLIC VERSION 17)
target_clang_tidy(${PROJECT_NAME}_example_plugin ENABLE ${ENABLE_CLANG_TIDY})

# Example Implementation Library
add_library(${PROJECT_NAME}_example_plugin_impl plugin_impl.cpp)
target_link_libraries(${PROJECT_NAME}_example_plugin_impl PUBLIC ${PROJECT_NAME}_example_plugin)
target_cxx_version(${PROJECT_NAME}_example_plugin_impl PUBLIC VERSION 17)
target_clang_tidy(${PROJECT_NAME}_example_plugin_impl ENABLE ${ENABLE_CLANG_TIDY})

# Example
add_executable(${PROJECT_NAME}_example example.cpp)
target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME}_example_plugin)
target_compile_definitions(${PROJECT_NAME}_example PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                                           PLUGINS="${PROJECT_NAME}_example_plugin_impl")
target_clang_tidy(${PROJECT_NAME}_example ENABLE ${ENABLE_CLANG_TIDY})

# Install the plugin libraries
install(
  TARGETS ${PROJECT_NAME}_example_plugin ${PROJECT_NAME}_example_plugin_impl
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# Install the example executable
install(TARGETS ${PROJECT_NAME}_example RUNTIME DESTINATION lib/${PROJECT_NAME})
